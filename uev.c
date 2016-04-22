/* libuEv - Micro event loop library
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2016  Joachim Nilsson <troglobit()gmail!com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>		/* O_CLOEXEC */
#include <string.h>		/* memset() */
#include <sys/epoll.h>
#include <sys/signalfd.h>	/* struct signalfd_siginfo */
#include <unistd.h>		/* close(), read() */

#include "uev.h"

#define UNUSED(arg) arg __attribute__ ((unused))


static int _init(uev_ctx_t *ctx, int close_old)
{
	int fd;

	fd = epoll_create1(EPOLL_CLOEXEC);
	if (fd < 0)
		return -1;

	if (close_old)
		close(ctx->fd);

	ctx->fd = fd;

	return 0;
}

/* Simple check if a descriptor is still valid in the kernel */
static int is_valid_fd(int fd)
{
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

/* Private to libuEv, do not use directly! */
int uev_watcher_init(uev_ctx_t *ctx, uev_t *w, uev_type_t type, uev_cb_t *cb, void *arg, int fd, int events)
{
	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	w->ctx    = ctx;
	w->type   = type;
	w->active = 0;
	w->fd     = fd;
	w->cb     = (void *)cb;
	w->arg    = arg;
	w->events = events;

	return 0;
}

/* Private to libuEv, do not use directly! */
int uev_watcher_start(uev_t *w)
{
	struct epoll_event ev;

	if (!w || w->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	if (w->active)
		return 0;

	ev.events   = w->events | EPOLLRDHUP;
	ev.data.ptr = w;
	if (epoll_ctl(w->ctx->fd, EPOLL_CTL_ADD, w->fd, &ev) < 0)
		return -1;

	w->active = 1;

	/* Add to internal list for bookkeeping */
	LIST_INSERT_HEAD(&w->ctx->watchers, w, link);

	return 0;
}

/* Private to libuEv, do not use directly! */
int uev_watcher_stop(uev_t *w)
{
	if (!w) {
		errno = EINVAL;
		return -1;
	}

	if (!w->active)
		return 0;

	w->active = 0;

	/* Remove from internal list */
	LIST_REMOVE(w, link);

	/* Remove from kernel */
	if (epoll_ctl(w->ctx->fd, EPOLL_CTL_DEL, w->fd, NULL) < 0)
		return -1;

	return 0;
}

/**
 * Create an event loop context
 * @param ctx  Pointer to an uev_ctx_t context to be initialized
 *
 * @return POSIX OK(0) on success, or non-zero on error.
 */
int uev_init(uev_ctx_t *ctx)
{
	if (!ctx) {
		errno = EINVAL;
		return -1;
	}

	memset(ctx, 0, sizeof(*ctx));
	LIST_INIT(&ctx->watchers);

	return _init(ctx, 0);
}

/**
 * Terminate the event loop
 * @param ctx  A valid libuEv context
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_exit(uev_ctx_t *ctx)
{
	if (!ctx) {
		errno = EINVAL;
		return -1;
	}

	while (!LIST_EMPTY(&ctx->watchers)) {
		uev_t *w = LIST_FIRST(&ctx->watchers);

		/* Remove from internal list */
		LIST_REMOVE(w, link);

		if (!w->active)
			continue;

		switch (w->type) {
		case UEV_TIMER_TYPE:
			uev_timer_stop(w);
			break;

		case UEV_SIGNAL_TYPE:
			uev_signal_stop(w);
			break;

		case UEV_IO_TYPE:
			uev_io_stop(w);
			break;
		}
	}

	ctx->running = 0;
	close(ctx->fd);
	ctx->fd = -1;

	return 0;
}

/**
 * Start the event loop
 * @param ctx    A valid libuEv context
 * @param flags  A mask of %UEV_ONCE and %UEV_NONBLOCK, or zero
 *
 * With @flags set to %UEV_ONCE the event loop returns after the first
 * event has been served, useful for instance to set a timeout on a file
 * descriptor.  If @flags also has the %UEV_NONBLOCK flag set the event
 * loop will return immediately if no event is pending, useful when run
 * inside another event loop.
 *
 * @return POSIX OK(0) upon successful termination of the event loop, or
 * non-zero on error.
 */
int uev_run(uev_ctx_t *ctx, int flags)
{
	int result = 0, timeout = -1;
	uev_t *w;

        if (!ctx || ctx->fd < 0) {
		errno = EINVAL;
                return -1;
	}

	if (flags & UEV_NONBLOCK)
		timeout = 0;

	/* Start the event loop */
	ctx->running = 1;

	/* Start all dormant timers */
	LIST_FOREACH(w, &ctx->watchers, link) {
		if (UEV_TIMER_TYPE == w->type)
			uev_timer_set(w, w->timeout, w->period);
	}

	while (ctx->running) {
		int i, nfds;
		struct epoll_event events[UEV_MAX_EVENTS];

		while ((nfds = epoll_wait(ctx->fd, events, UEV_MAX_EVENTS, timeout)) < 0) {
			if (!ctx->running)
				break;

			if (EINTR == errno)
				continue; /* Signalled, try again */

			/* Unrecoverable error, cleanup and exit with error. */
			uev_exit(ctx);
			return -1;
		}

		for (i = 0; ctx->running && i < nfds; i++) {
			w = (uev_t *)events[i].data.ptr;

			if (events[i].events & (EPOLLHUP | EPOLLERR)) {
				ctx->errors++;

				if (ctx->errors >= 42) {
					uev_t *tmp, *retry = w;;

					/* If not valid anymore, try to remove, ignore any errors. */
					if (!is_valid_fd(w->fd))
						uev_watcher_stop(w);

					/* Must recreate epoll fd now ... */
					if (_init(ctx, 1)) {
						uev_exit(ctx);
						return -2;
					}

					/* Restart watchers in new efd */
					LIST_FOREACH_SAFE(w, &ctx->watchers, link, tmp) {
						if (w->active) {
							w->active = 0;
							LIST_REMOVE(w, link);
							uev_watcher_start(w);
						}
					}
					uev_watcher_start(retry);

					/* New efd, restart everything! */
					ctx->errors = 0;
					continue;
				}
			}

			if (UEV_TIMER_TYPE == w->type) {
				uint64_t exp;

				if (read(w->fd, &exp, sizeof(exp)) != sizeof(exp)) {
					uev_exit(ctx);
					return -3;
				}

				if (!w->period)
					w->timeout = 0;
			}

			if (UEV_SIGNAL_TYPE == w->type) {
				struct signalfd_siginfo fdsi;
				ssize_t sz = sizeof(fdsi);

				if (read(w->fd, &fdsi, sz) != sz) {
					uev_exit(ctx);
					return -4;
				}
			}

			if (w->cb)
				w->cb(w, w->arg, events[i].events & UEV_EVENT_MASK);

			if (UEV_TIMER_TYPE == w->type) {
				if (!w->timeout)
					uev_timer_stop(w);
			}
		}

		if ((flags & UEV_ONCE) || LIST_EMPTY(&ctx->watchers))
			break;
	}

	return result;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
