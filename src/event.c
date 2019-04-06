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
#include <sys/eventfd.h>
#include <unistd.h>		/* close(), read() */

#include "uev.h"


/**
 * Create a generic event watcher
 * @param ctx    A valid libuEv context
 * @param w      Pointer to an uev_t watcher
 * @param cb     Callback when an event is posted
 * @param arg    Optional callback argument
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_event_init(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg)
{
	int fd;

	if (!w || !ctx) {
		errno = EINVAL;
		return -1;
	}
	w->fd = -1;

	fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (fd < 0)
		return -1;

	return _uev_watcher_init(ctx, w, UEV_EVENT_TYPE, cb, arg, fd, UEV_READ)
		|| _uev_watcher_start(w);
}

/**
 * Post a generic event
 * @param w  Watcher to post to
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_event_post(uev_t *w)
{
	uint64_t val = 1;

	if (!w || -1 == w->fd) {
		errno = EINVAL;
		return -1;
	}


	if (write(w->fd, &val, sizeof(val)) != sizeof(val))
		return -1;

	return 0;
}

/**
 * Stop a generic event watcher
 * @param w  Watcher to stop
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_event_stop(uev_t *w)
{
	if (!_uev_watcher_active(w))
		return 0;

	if (_uev_watcher_stop(w))
		return -1;

	close(w->fd);
	w->fd = -1;

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
