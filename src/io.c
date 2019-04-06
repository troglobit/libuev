/* libuEv - Micro event loop library
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2019  Joachim Nilsson <troglobit()gmail!com>
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
#include "uev.h"


/**
 * Create an I/O watcher
 * @param ctx     A valid libuEv context
 * @param w       Pointer to an uev_t watcher
 * @param cb      I/O callback
 * @param arg     Optional callback argument
 * @param fd      File descriptor to watch, or -1 to register an empty watcher
 * @param events  Events to watch for: %UEV_READ, %UEV_WRITE, %UEV_EDGE, %UEV_ONESHOW
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_io_init(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int fd, int events)
{
	if (fd < 0) {
		errno = EINVAL;
		return -1;
	}

	if (_uev_watcher_init(ctx, w, UEV_IO_TYPE, cb, arg, fd, events))
		return -1;

	return _uev_watcher_start(w);
}

/**
 * Reset an I/O watcher
 * @param w       Pointer to an uev_t watcher
 * @param fd      New file descriptor to monitor
 * @param events  Requested events to watch for, a mask of %UEV_READ and %UEV_WRITE
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_io_set(uev_t *w, int fd, int events)
{
	if ((events & UEV_ONESHOT) && _uev_watcher_active(w))
		return _uev_watcher_rearm(w);

	/* Ignore any errors, only to clean up anything lingering ... */
	uev_io_stop(w);

	return uev_io_init(w->ctx, w, (uev_cb_t *)w->cb, w->arg, fd, events);
}

/**
 * Start an I/O watcher
 * @param w  Watcher to start (again)
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_io_start(uev_t *w)
{
	return uev_io_set(w, w->fd, w->events);
}

/**
 * Stop an I/O watcher
 * @param w  Watcher to stop
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_io_stop(uev_t *w)
{
	return _uev_watcher_stop(w);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
