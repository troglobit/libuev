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
 * This function creates, and optionally starts, a timer watcher.  There
 * are two types of timers: one-shot and periodic.
 *
 * One-shot timers only use @param timeout, @param period is zero.
 *
 * Periodic timers can either start their life disabled, with @param
 * timeout set to zero, or with the same value as @param period.
 *
 * When the timeout expires, for either of the two types, @param cb is
 * called, with the optional @param arg argument.  A one-shot timer ends
 * its life there, while a periodic task's @param timeout is reset to
 * the @param period and restarted.
 *
 * A timer is automatically started if the event loop is already
 * running, otherwise it is kept on hold until triggered by calling
 * uev_run().
 *
 * @see uev_timer_set
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_timer_init(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period)
{
	int fd;

	if (timeout < 0 || period < 0) {
		errno = ERANGE;
		return -1;
	}

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (fd < 0)
		return -1;

	if (_uev_watcher_init(ctx, w, UEV_TIMER_TYPE, cb, arg, fd, UEV_READ))
		goto exit;

	if (uev_timer_set(w, timeout, period)) {
		_uev_watcher_stop(w);
	exit:
		close(fd);
		w->fd = -1;
		return -1;
	}

	return 0;
}

/**
 * Reset a timer
 * @param w        Watcher to reset
 * @param timeout  Timeout in milliseconds before @param cb is called, zero disarms timer
 * @param period   For periodic timers this is the period time that @param timeout is reset to
 *
 * Note, the @param timeout value must be non-zero.  Setting it to zero
 * will disarm the timer.  This is the underlying Linux function @func
 * timerfd_settimer() which has this behavior.
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

	if (timeout < 0 || period < 0) {
		errno = ERANGE;
		return -1;
	}

	/* Handle stopped timers */
	if (w->fd < 0) {
		/* Timer already stopped */
		if (!timeout && !period)
			return 0;

		if (uev_timer_init(w->ctx, w, (uev_cb_t *)w->cb, w->arg, timeout, period))
			return -1;
	}

	w->u.t.timeout = timeout;
	w->u.t.period  = period;

	if (w->ctx->running) {
		struct itimerspec time;

		msec2tspec(timeout, &time.it_value);
		msec2tspec(period, &time.it_interval);
		if (timerfd_settime(w->fd, 0, &time, NULL) < 0)
			return 1;
	}

	return _uev_watcher_start(w);
}

/**
 * Start a stopped timer watcher
 * @param w  Watcher to start (again)
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_timer_start(uev_t *w)
{
	if (!w) {
		errno = EINVAL;
		return -1;
	}

	if (-1 != w->fd)
		_uev_watcher_stop(w);

	return uev_timer_set(w, w->u.t.timeout, w->u.t.period);
}

/**
 * Stop and unregister a timer watcher
 * @param w  Watcher to stop
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_timer_stop(uev_t *w)
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
