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
#include <sys/timerfd.h>
#include <unistd.h>		/* close(), read() */

#include "uev.h"


static struct timespec msec2tspec(int msec)
{
	struct timespec ts;

	ts.tv_sec  =  msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;

	return ts;
}

/**
 * Create a timer watcher
 * @param ctx     A valid libuev context
 * @param handler Timer callback
 * @param data    Optional callback argument
 * @param timeout Timeout in milliseconds before @param handler is called
 * @param period  For periodic timers this is the period time that @param timeout is reset to
 *
 * For one-shot timers you set @param period to zero and only use @param
 * timeout.  For periodic timers you likely set @param timeout to either
 * zero, to call it as soon as the event loop starts, or to the same
 * value as @param period.  When the timer expires, the @param handler
 * is called, with the optional @param data argument.  A non-periodic
 * timer ends its life there, while a periodic task's @param timeout is
 * reset to the @param period and restarted.
 *
 * @return The new timer, or %NULL if invalid pointers or out or memory.
 */
uev_watcher_t *uev_timer_create(uev_t *ctx, uev_cb_t *handler, void *data, int timeout, int period)
{
	int fd;
	uev_watcher_t *w;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (fd < 0)
		return NULL;

	w = uev_watcher_create(ctx, UEV_TIMER_TYPE, fd, UEV_DIR_INBOUND, handler, data);
	if (!w)
		goto exit;

	if (uev_timer_set(ctx, w, timeout, period)) {
		uev_watcher_delete(ctx, w);
	exit:
		close(fd);
		return NULL;
	}

	return w;
}

/**
 * Reset or reschedule a timer
 * @param ctx  A valid libuev context
 * @param w    I/O watcher
 *
 * @return POSIX OK(0) or non-zero with @param errno set.
 */
int uev_timer_set(uev_t *ctx, uev_watcher_t *w, int timeout, int period)
{
	struct itimerspec time;

	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	w->timeout = timeout;
	w->period  = period;

	if (!ctx->running)
		return 0;

	time.it_value    = msec2tspec(timeout);
	time.it_interval = msec2tspec(period);

	return timerfd_settime(w->fd, 0, &time, NULL);
}

/**
 * Delete a timer watcher
 * @param ctx  A valid libuev context
 * @param w    I/O watcher
 *
 * @return POSIX OK(0) or non-zero with @param errno set.
 */
int uev_timer_delete(uev_t *ctx, uev_watcher_t *w)
{
	if (!ctx || !w) {
		errno = EINVAL;
		return -1;
	}

	uev_timer_set(ctx, w, 0, 0);

	return uev_watcher_delete(ctx, w);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
