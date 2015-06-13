/* libuEv - Micro event loop library
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2015  Joachim Nilsson <troglobit()gmail!com>
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
#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>		/* close(), read() */

#include "uev.h"


/**
 * Create a signal watcher
 * @param ctx    A valid libuEv context
 * @param w      Pointer to an uev_t watcher
 * @param cb     Timer callback
 * @param arg    Optional callback argument
 * @param signo  Signal to watch for
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_signal_init(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int signo)
{
	int fd;
	sigset_t mask;

	if (!w || !ctx) {
		errno = EINVAL;
		return -1;
	}
	w->fd = -1;

	sigemptyset(&mask);
	fd = signalfd(-1, &mask, SFD_NONBLOCK);
	if (fd < 0)
		return -1;

	if (uev_watcher_init(ctx, (uev_private_t*)w, UEV_SIGNAL_TYPE, cb, arg, fd, UEV_READ))
		goto exit;

	if (uev_signal_set(w, signo)) {
		uev_watcher_stop((uev_private_t*)w);
	exit:
		close(fd);
		return -1;
	}

	return 0;
}

/**
 * Reset a signal watcher
 * @param w      Watcher to reset
 * @param signo  New signal to watch for
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_signal_set(uev_t *w, int signo)
{
	sigset_t mask;
        uev_private_t *_w = (uev_private_t*)w;

	/* Every watcher must be registered to a context */
	if (!_w || !_w->ctx) {
		errno = EINVAL;
		return -1;
	}

	/* Remember for callbacks and start/stop */
	_w->signo = signo;

	/* Handle stopped signal watchers */
	if (_w->fd < 0) {
		/* Remove from internal list */
		LIST_REMOVE(_w, link);

		if (uev_signal_init(_w->ctx, w, _w->cb, _w->arg, signo))
			return -1;
	}

	sigemptyset(&mask);
	sigaddset(&mask, signo);

	/* Block signals so that they aren't handled
	   according to their default dispositions */
	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
		return -1;

	if (signalfd(w->fd, &mask, SFD_NONBLOCK) < 0)
		return -1;

	return uev_watcher_start(_w);
}


/**
 * Start a stopped signal watcher
 * @param w  Watcher to start (again)
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_signal_start(uev_t *w)
{
	if (!w) {
		errno = EINVAL;
		return -1;
	}

	if (-1 != w->fd)
		uev_signal_stop(w);

	return uev_signal_set(w, ((uev_private_t*)w)->signo);
}

/**
 * Stop a signal watcher
 * @param w  Watcher to stop
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_signal_stop(uev_t *w)
{
	if (uev_watcher_stop((uev_private_t*)w))
		return -1;

	close(w->fd);
	w->fd = -1;

	return 0;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
