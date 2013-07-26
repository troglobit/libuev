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
#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>		/* close(), read() */

#include "uev.h"


/**
 * Create a signal watcher
 * @param ctx     A valid libuev context
 * @param handler Timer callback
 * @param data    Optional callback argument
 * @param signo   Signal to watch for
 *
 * @return The new watcher, or %NULL with @param errno set on error.
 */
uev_t *uev_signal_create(uev_ctx_t *ctx, uev_cb_t *handler, void *data, int signo)
{
	int fd;
	sigset_t mask;
	uev_t *w;

	sigemptyset(&mask);
	fd = signalfd(-1, &mask, SFD_NONBLOCK);
	if (fd < 0)
		return NULL;

	w = uev_watcher_create(ctx, UEV_SIGNAL_TYPE, fd, UEV_DIR_INBOUND, handler, data);
	if (!w)
		goto exit;

	if (uev_signal_set(ctx, w, signo)) {
		uev_watcher_delete(ctx, w);
	exit:
		close(fd);
		return NULL;
	}

	return w;
}

/**
 * Reset a signal watcher
 * @param ctx   A valid libuev context
 * @param w     Watcher to reset
 * @param signo New signal to watch for
 *
 * @return POSIX OK(0) or non-zero with @param errno set.
 */
int uev_signal_set(uev_ctx_t *ctx, uev_t *w, int signo)
{
	sigset_t mask;

	if (!ctx || !w) {
		errno = EINVAL;
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

	return 0;
}

/**
 * Delete a signal watcher
 * @param ctx  A valid libuev context
 * @param w    Watcher to delete
 *
 * @return POSIX OK(0) or non-zero with @param errno set.
 */
int uev_signal_delete(uev_ctx_t *ctx, uev_t *w)
{
	int fd;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	uev_timer_set(ctx, w, 0, 0);
	fd = w->fd;
	uev_watcher_delete(ctx, w);
	close(fd);

	return 0;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
