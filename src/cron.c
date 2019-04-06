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
#include <string.h>		/* memset() */
#include <sys/timerfd.h>
#include <unistd.h>		/* close(), read() */

#include "uev.h"

/* Missing defines in GLIBC <= 2.24 */
#ifndef TFD_TIMER_CANCEL_ON_SET
#define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#endif
#ifndef TFD_SETTIME_FLAGS
#define TFD_SETTIME_FLAGS (TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET)
#endif


/**
 * Create and start an at/cron job watcher
 * @param ctx      A valid libuEv context
 * @param w        Pointer to an uev_t watcher
 * @param cb       Callback function for cron job
 * @param arg      Optional callback argument
 * @param when     First point in time to call @param cb
 * @param interval For an at job this is zero, for cron the offset interval
 *
 * For at jobs set @param interval to zero and only use @param when.  For
 * cron jobs, set @param interval to the offset.  E.g., if the job should
 * run every five minutes set the @param tm_min of struct tm to five.
 *
 * Use mktime() to create the time_t arguments.  The special value zero
 * may be used for @param when to denote 'now', where 'now' is when the
 * event loop is started.  You can also treat time_t simply as a signed
 * integer.  E.g., set @param interval to 3600 to create a cron job that
 * runs every hour.
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_cron_init(uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, time_t when, time_t interval)
{
	int fd;

	if (when < 0 || interval < 0) {
		errno = ERANGE;
		return -1;
	}

	fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
	if (fd < 0)
		return -1;

	if (_uev_watcher_init(ctx, w, UEV_CRON_TYPE, cb, arg, fd, UEV_READ))
		goto exit;

	if (uev_cron_set(w, when, interval)) {
		_uev_watcher_stop(w);
	exit:
		close(fd);
		return -1;
	}

	return 0;
}

/**
 * Reset an at/cron job watcher
 * @param w        Watcher to reset
 * @param when     First point in time to call @param cb
 * @param interval For an at job this is zero, for cron the offset interval
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_cron_set(uev_t *w, time_t when, time_t interval)
{
	/* Every watcher must be registered to a context */
	if (!w || !w->ctx) {
		errno = EINVAL;
		return -1;
	}

	if (when < 0 || interval < 0) {
		errno = ERANGE;
		return -1;
	}

	/* Handle stopped timers */
	if (w->fd < 0) {
		/* Timer already stopped */
		if (!when && !interval)
			return 0;

		if (uev_cron_init(w->ctx, w, (uev_cb_t *)w->cb, w->arg, when, interval))
			return -1;
	}

	w->u.c.when     = when;
	w->u.c.interval = interval;

	if (w->ctx->running) {
		struct itimerspec time;

		memset(&time, 0, sizeof(time));
		time.it_value.tv_sec    = when;
		time.it_interval.tv_sec = interval;
		if (timerfd_settime(w->fd, TFD_SETTIME_FLAGS, &time, NULL) < 0)
			return 1;
	}

	return _uev_watcher_start(w);
}

/**
 * Start a stopped at/cron job watcher
 * @param w  Watcher to start (again)
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_cron_start(uev_t *w)
{
	return uev_timer_start(w);
}

/**
 * Stop and unregister an at/cron job watcher
 * @param w  Watcher to stop
 *
 * @return POSIX OK(0) or non-zero with @param errno set on error.
 */
int uev_cron_stop(uev_t *w)
{
	return uev_timer_stop(w);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
