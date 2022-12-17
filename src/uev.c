/* libuEv - Micro event loop library
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2021  Joachim Wiberg <troglobit()gmail!com>
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

/** Micro event loop library
 * @file uev.c
 *
 */

#include <errno.h>
#include <fcntl.h>		/* O_CLOEXEC */
#include <string.h>		/* memset() */
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/select.h>		/* for select() workaround */
#include <sys/signalfd.h>	/* struct signalfd_siginfo */
#include <unistd.h>		/* close(), read() */

#include "uev.h"


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

/* Used by file i/o workaround when epoll => EPERM */
static int has_data(int fd)
{
	struct timeval timeout = { 0, 0 };
	fd_set fds;
	int n = 0;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (select(1, &fds, NULL, NULL, &timeout) > 0)
		return ioctl(0, FIONREAD, &n) == 0 && n > 0;

	return 0;
}

/* Private to libuEv, do not use directly! */
int _uev_watcher_init(uev_ctx_t *ctx, uev_t *w, uev_type_t type, uev_cb_t *cb, void *arg, int fd, int events)
{
	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	w->ctx    = ctx;
	w->type   = type;
	w->active = 0;
	w->fd     = fd;
	w->cb     = cb;
	w->arg    = arg;
	w->events = events;

	return 0;
}

/* Private to libuEv, do not use directly! */
int _uev_watcher_start(uev_t *w)
{
	struct epoll_event ev;

	if (!w || w->fd < 0 || !w->ctx) {
		errno = EINVAL;
		return -1;
	}

	if (_uev_watcher_active(w))
		return 0;

	ev.events   = w->events | EPOLLRDHUP;
	ev.data.ptr = w;
	if (epoll_ctl(w->ctx->fd, EPOLL_CTL_ADD, w->fd, &ev) < 0) {
		if (errno != EPERM)
			return -1;

		/* Handle special case: `application < file.txt` */
		if (w->type != UEV_IO_TYPE || w->events != UEV_READ)
			return -1;

		/* Only allow this special handling for stdin */
		if (w->fd != STDIN_FILENO)
			return -1;

		w->ctx->workaround = 1;
		w->active = -1;
	} else {
		w->active = 1;
	}

	/* Add to internal list for bookkeeping */
	_UEV_INSERT(w, w->ctx->watchers);

	return 0;
}

/* Private to libuEv, do not use directly! */
int _uev_watcher_stop(uev_t *w)
{
	if (!w) {
		errno = EINVAL;
		return -1;
	}

	if (!_uev_watcher_active(w))
		return 0;

	w->active = 0;

	/* Remove from internal list */
	_UEV_REMOVE(w, w->ctx->watchers);

	/* Remove from kernel */
	if (epoll_ctl(w->ctx->fd, EPOLL_CTL_DEL, w->fd, NULL) < 0)
		return -1;

	return 0;
}

/* Private to libuEv, do not use directly! */
int _uev_watcher_active(uev_t *w)
{
	if (!w)
		return 0;

	return w->active > 0;
}

/* Private to libuEv, do not use directly! */
int _uev_watcher_rearm(uev_t *w)
{
	struct epoll_event ev;

	if (!w || w->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	ev.events   = w->events | EPOLLRDHUP;
	ev.data.ptr = w;
	if (epoll_ctl(w->ctx->fd, EPOLL_CTL_MOD, w->fd, &ev) < 0)
		return -1;

	return 0;
}

/**
 * Create an event loop context
 * @param ctx  Pointer to an uev_ctx_t context to be initialized
 *
 * This function calls uev_init1() with @p maxevents set to
 * ::UEV_MAX_EVENTS
 *
 * @return POSIX OK(0) on success, or non-zero on error.
 */
int uev_init(uev_ctx_t *ctx)
{
	return uev_init1(ctx, UEV_MAX_EVENTS);
}

/**
 * Create an event loop context
 * @param ctx       Pointer to an uev_ctx_t context to be initialized
 * @param maxevents Maximum number of events in event cache [1, 10]
 *
 * This function is the same as uev_init() except for the @p maxevents
 * argument, max ::UEV_MAX_EVENTS, which controls the number of events
 * in the event cache returned to the main loop.
 *
 * In cases where you have multiple events pending in the cache and some
 * event may cause later ones, already sent by the kernel to userspace,
 * to be deleted the pointer returned to the event loop for this later
 * event may be deleted.
 *
 * There are two ways around this (accessing deleted memory):
 *   -# use this function to initialize your event loop and set
 *      @p maxevents to 1
 *   -# use a free list in you application that you garbage collect
 *      at intervals relevant to your application
 *
 * @return POSIX OK(0) on success, or non-zero on error.
 */
int uev_init1(uev_ctx_t *ctx, int maxevents)
{
	if (!ctx || maxevents < 1) {
		errno = EINVAL;
		return -1;
	}

	if (maxevents > UEV_MAX_EVENTS)
		maxevents = UEV_MAX_EVENTS;

	memset(ctx, 0, sizeof(*ctx));
	ctx->maxevents = maxevents;

	return _init(ctx, 0);
}

/**
 * Terminate the event loop
 * @param ctx  A valid libuEv context
 *
 * @return POSIX OK(0) or non-zero with @p errno set on error.
 */
int uev_exit(uev_ctx_t *ctx)
{
	uev_t *w;

	if (!ctx) {
		errno = EINVAL;
		return -1;
	}

	_UEV_FOREACH(w, ctx->watchers) {
		/* Remove from internal list */
		_UEV_REMOVE(w, ctx->watchers);

		if (!_uev_watcher_active(w))
			continue;

		switch (w->type) {
		case UEV_IO_TYPE:
			uev_io_stop(w);
			break;

		case UEV_SIGNAL_TYPE:
			uev_signal_stop(w);
			break;

		case UEV_TIMER_TYPE:
		case UEV_CRON_TYPE:
			uev_timer_stop(w);
			break;

		case UEV_EVENT_TYPE:
			uev_event_stop(w);
			break;
		}
	}

	ctx->watchers = NULL;
	ctx->running = 0;
	if (ctx->fd > -1)
		close(ctx->fd);
	ctx->fd = -1;

	return 0;
}

/**
 * Start the event loop
 * @param ctx    A valid libuEv context
 * @param flags  A mask of ::UEV_ONCE and ::UEV_NONBLOCK, or zero
 *
 * With @p flags set to ::UEV_ONCE the event loop returns after the first
 * event has been served, useful for instance to set a timeout on a file
 * descriptor.  If @p flags also has the ::UEV_NONBLOCK flag set the event
 * loop will return immediately if no event is pending, useful when run
 * inside another event loop.
 *
 * @return POSIX OK(0) upon successful termination of the event loop, or
 * non-zero on error.
 */
int uev_run(uev_ctx_t *ctx, int flags)
{
	int timeout = -1;
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
	_UEV_FOREACH(w, ctx->watchers) {
		if (UEV_CRON_TYPE == w->type)
			uev_cron_set(w, w->u.c.when, w->u.c.interval);
		if (UEV_TIMER_TYPE == w->type)
			uev_timer_set(w, w->u.t.timeout, w->u.t.period);
	}

	while (ctx->running && ctx->watchers) {
		struct epoll_event ee[UEV_MAX_EVENTS];
		int maxevents = ctx->maxevents;
		int i, nfds, rerun = 0;

		if (maxevents > UEV_MAX_EVENTS)
			maxevents = UEV_MAX_EVENTS;

		/* Handle special case: `application < file.txt` */
		if (ctx->workaround) {
			_UEV_FOREACH(w, ctx->watchers) {
				if (w->active != -1 || !w->cb)
					continue;

				if (!has_data(w->fd)) {
					w->active = 0;
					_UEV_REMOVE(w, ctx->watchers);
				}

				rerun++;
				w->cb(w, w->arg, UEV_READ);
			}
		}

		if (rerun)
			continue;
		ctx->workaround = 0;

		while ((nfds = epoll_wait(ctx->fd, ee, maxevents, timeout)) < 0) {
			if (!ctx->running)
				break;

			if (EINTR == errno)
				continue; /* Signalled, try again */

			/* Unrecoverable error, cleanup and exit with error. */
			uev_exit(ctx);

			return -2;
		}

		for (i = 0; ctx->running && i < nfds; i++) {
			struct signalfd_siginfo fdsi;
			ssize_t sz = sizeof(fdsi);
			uint32_t events;
			uint64_t exp;

			w = (uev_t *)ee[i].data.ptr;
			events = ee[i].events;

			switch (w->type) {
			case UEV_IO_TYPE:
				if (events & (EPOLLHUP | EPOLLERR))
					uev_io_stop(w);
				break;

			case UEV_SIGNAL_TYPE:
				if (read(w->fd, &fdsi, sz) != sz) {
					if (uev_signal_start(w)) {
						uev_signal_stop(w);
						events = UEV_ERROR;
					}
					memset(&w->siginfo, 0, sizeof(w->siginfo));
				} else
					w->siginfo = fdsi;
				break;

			case UEV_TIMER_TYPE:
				if (read(w->fd, &exp, sizeof(exp)) != sizeof(exp)) {
					uev_timer_stop(w);
					events = UEV_ERROR;
				}

				if (!w->u.t.period)
					w->u.t.timeout = 0;
				if (!w->u.t.timeout)
					uev_timer_stop(w);
				break;

			case UEV_CRON_TYPE:
				if (read(w->fd, &exp, sizeof(exp)) != sizeof(exp)) {
					events = UEV_HUP;
					if (errno != ECANCELED) {
						uev_cron_stop(w);
						events = UEV_ERROR;
					}
				}

				if (!w->u.c.interval)
					w->u.c.when = 0;
				else
					w->u.c.when += w->u.c.interval;
				if (!w->u.c.when)
					uev_timer_stop(w);
				break;

			case UEV_EVENT_TYPE:
				if (read(w->fd, &exp, sizeof(exp)) != sizeof(exp))
					events = UEV_HUP;
				break;
			}

			/*
			 * NOTE: Must be last action for watcher, the
			 *       callback may delete itself.
			 */
			if (w->cb)
				w->cb(w, w->arg, events & UEV_EVENT_MASK);
		}

		if (flags & UEV_ONCE)
			break;
	}

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
