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
#include <sys/timerfd.h>
#include <unistd.h>		/* close(), read() */

#include "uev.h"


static void msec2tspec(int msec, struct timespec *ts)
{
	if (msec) {
		ts->tv_sec  =  msec / 1000;
		ts->tv_nsec = (msec % 1000) * 1000000;
	} else {
		ts->tv_sec  = 0;
		ts->tv_nsec = 0;
	}
}

/**
 * Create and start a timer watcher
 * @param ctx      A valid libuEv context
 * @param w        Pointer to an uev_t watcher
 * @param cb       Callback function
 * @param arg      Optional callback argument
 * @param timeout  Timeout in milliseconds before @param cb is called
 * @param period   For periodic timers this is the period time that @param timeout is reset to
 *
 * For one-shot timers you set @param period to zero and only use @param
 * timeout.  For periodic timers you likely set @param timeout to either
 * zero, to call it as soon as the event loop starts, or to the same
 * value as @param period.  When the timer expires, the @param cb
 * is called, with the optional @param arg argument.  A non-periodic
 * timer ends its life there, while a periodic task's @param timeout is
 * reset to the @param period and restarted.
 *
 * A timer is automatically started if the event loop is already
 * running, otherwise it is kept on hold until triggered by calling
 * uev_run().
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_timer_init(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period)
{
	int fd;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (fd < 0)
		return -1;

	if (uev_watcher_init(ctx, w, UEV_TIMER_TYPE, cb, arg, fd, UEV_READ))
		goto exit;

	if (uev_timer_set(w, timeout, period)) {
		uev_watcher_stop(w);
	exit:
		close(fd);
		return -1;
	}

	return 0;
}

/**
 * Reset a timer
 * @param w        Watcher to reset
 * @param timeout  Timeout in milliseconds before @param cb is called
 * @param period   For periodic timers this is the period time that @param timeout is reset to
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_timer_set(uev_t *w, int timeout, int period)
{
	/* Every watcher must be registered to a context */
	if (!w || !w->ctx) {
		errno = EINVAL;
		return -1;
	}

	/* Handle stopped timers */
	if (w->fd < 0) {
		/* Timer already stopped */
		if (!timeout && !period)
			return 0;

		/* Remove from internal list */
		LIST_REMOVE(w, link);

		if (uev_timer_init(w->ctx, w, (uev_cb_t *)w->cb, w->arg, timeout, period))
			return -1;
	}

	w->timeout = timeout;
	w->period  = period;

	if (w->ctx->running) {
		struct itimerspec time;

		msec2tspec(timeout, &time.it_value);
		msec2tspec(period, &time.it_interval);
		timerfd_settime(w->fd, 0, &time, NULL);

		return uev_watcher_start(w);
	}

	return 0;
}

/**
 * Stop and unregister a timer watcher
 * @param w  Watcher to stop
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_timer_stop(uev_t *w)
{
	if (uev_watcher_stop(w))
		return -1;

	/* Stop kernel timer */
	uev_timer_set(w, 0, 0);

	/* Close timerfd, will have to be reopened again on reset */
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
