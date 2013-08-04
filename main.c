/* libuev - Micro event loop library
 *
 * Copyright (c) 2012  Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013  Joachim Nilsson <troglobit()gmail!com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <string.h>		/* memset() */
#include <sys/epoll.h>
#include <sys/signalfd.h>	/* struct signalfd_siginfo */
#include <unistd.h>		/* close(), read() */

#include "uev.h"

/* Private to libuev, do not use directly! */
int uev_watcher_init(uev_ctx_t *ctx, uev_t *w, uev_type_t type, uev_cb_t *cb, void *arg, int fd, int events)
{
	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	memset(w, 0, sizeof(*w));
	w->ctx    = ctx;
	w->fd     = fd;
	w->type   = type;
	w->cb     = (void *)cb;
	w->arg    = arg;
	w->events = events;

	LIST_INSERT_HEAD(&w->ctx->watchers, w, link);

	return 0;
}

/* Private to libuev, do not use directly! */
int uev_watcher_start(uev_t *w)
{
	struct epoll_event ev;

	if (!w || w->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	if (w->active)
		return 0;

	ev.events   = w->events;
	ev.data.ptr = w;
	if (epoll_ctl(w->ctx->fd, EPOLL_CTL_ADD, w->fd, &ev) < 0)
		return -1;

	w->active = 1;

	return 0;
}

/* Private to libuev, do not use directly! */
int uev_watcher_stop(uev_t *w)
{
	if (!w) {
		errno = EINVAL;
		return -1;
	}

	if (!w->active)
		return 0;

	/* Remove from kernel */
	epoll_ctl(w->ctx->fd, EPOLL_CTL_DEL, w->fd, NULL);
	w->active = 0;

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
	int fd;

	fd = epoll_create(1);
	if (fd < 0)
		return -1;

	memset(ctx, 0, sizeof(*ctx));
	ctx->fd = fd;
	LIST_INIT(&ctx->watchers);

	return 0;
}

/**
 * Terminate the event loop
 * @param ctx  A valid libuev context
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
 * @param ctx  A valid libuev context
 *
 * @return POSIX OK(0) upon successful termination of the event loop, or non-zero on error.
 */
int uev_run(uev_ctx_t *ctx)
{
	int result = 0;
	uev_t *w;

        if (!ctx || ctx->fd < 0) {
		errno = EINVAL;
                return -1;
	}

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

		while ((nfds = epoll_wait(ctx->fd, events, UEV_MAX_EVENTS, -1)) < 0) {
			if (!ctx->running)
				break;

			if (EINTR == errno)
				continue; /* Signalled, try again */
		exit:
			result = -1;
			ctx->running = 0;
			break;
		}

		for (i = 0; ctx->running && i < nfds; i++) {
			w = (uev_t *)events[i].data.ptr;

			if (UEV_TIMER_TYPE == w->type) {
				uint64_t exp;

				if (read(w->fd, &exp, sizeof(exp)) != sizeof(exp))
					goto exit;

				if (!w->period)
					w->timeout = 0;
			}

			if (UEV_SIGNAL_TYPE == w->type) {
				struct signalfd_siginfo fdsi;
				ssize_t sz = sizeof(fdsi);

				if (read(w->fd, &fdsi, sz) != sz)
					goto exit;

				w->signo = fdsi.ssi_signo;
			}

			if (w->cb)
				w->cb((struct uev *)ctx, w, w->arg);

			if (UEV_TIMER_TYPE == w->type) {
				if (!w->timeout)
					uev_timer_stop(w);
			}
		}
	}

	return result;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
