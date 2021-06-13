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

/**
 * Micro event loop library
 * @file uev.h
 * @author Flemming Madsen (2012)
 * @author Joachim Wiberg (2013-2021)
 * @copyright MIT License
 *
 * The latest version of this manual and the libuEv software are
 * available at https://github.com/troglobit/libuEv/
 */

#ifndef LIBUEV_UEV_H_
#define LIBUEV_UEV_H_

#include "private.h"

#define UEV_MAX_EVENTS  10		/**< Max. number of simulateneous events */

/* I/O events, signal and timer revents are always UEV_READ */
#define UEV_NONE        0		/**< normal loop      */
#define UEV_ERROR       EPOLLERR	/**< error flag       */
#define UEV_READ        EPOLLIN		/**< poll for reading */
#define UEV_WRITE       EPOLLOUT	/**< poll for writing */
#define UEV_PRI         EPOLLPRI	/**< priority message */
#define UEV_HUP         EPOLLHUP	/**< hangup event     */
#define UEV_RDHUP       EPOLLRDHUP	/**< peer shutdown    */
#define UEV_EDGE        EPOLLET		/**< edge triggered   */
#define UEV_ONESHOT     EPOLLONESHOT	/**< one-shot event   */

/* Run flags */
#define UEV_ONCE        1		/**< run loop once    */
#define UEV_NONBLOCK    2		/**< exit if no event */

/** Check if I/O watcher is active or stopped */
#define uev_io_active(w)     _uev_watcher_active(w)
/** Check if signal watcher is active or stopped */
#define uev_signal_active(w) _uev_watcher_active(w)
/** Check if timer is active or stopped */
#define uev_timer_active(w)  _uev_watcher_active(w)
/** Check if cron timer watcher is active or stopped */
#define uev_cron_active(w)   _uev_watcher_active(w)
/** Check if event watcher is active or stopped */
#define uev_event_active(w)  _uev_watcher_active(w)

/** Event loop context, need one per process and thread */
typedef struct uev_ctx uev_ctx_t;

/** Event watcher */
typedef struct uev {
	/* Private data for libuEv internal engine */
	uev_private_t   type;

	/* Public data for users to reference  */
	int             signo;		/**< configured signal */
	int             fd;		/**< active descriptor */
	uev_ctx_t      *ctx;		/**< watcher context   */

	/* Extra data for certain watcher types */
	struct signalfd_siginfo siginfo; /**< received signal  */
} uev_t;

/**
 * Generic callback for watchers, @p events holds ::UEV_READ and/or
 * ::UEV_WRITE with optional ::UEV_PRI (priority data available to read)
 * and any of the ::UEV_HUP and/or ::UEV_RDHUP, which may be used to
 * signal hang-up and peer shutdown events.
 *
 * Note: ::UEV_ERROR conditions must be handled by all callbacks!  I/O
 *       watchers may also need to check ::UEV_HUP.  Appropriate action,
 *       e.g. restart the watcher, is up to the application and is thus
 *       delegated to the callback.
 */
typedef void (uev_cb_t)(uev_t *w, void *arg, int events);

/* Public interface */

/** Create an event loop context */
int uev_init           (uev_ctx_t *ctx);
int uev_init1          (uev_ctx_t *ctx, int maxevents);
int uev_exit           (uev_ctx_t *ctx);
int uev_run            (uev_ctx_t *ctx, int flags);

int uev_io_init        (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int fd, int events);
int uev_io_set         (uev_t *w, int fd, int events);
int uev_io_start       (uev_t *w);
int uev_io_stop        (uev_t *w);

int uev_timer_init     (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period);
int uev_timer_set      (uev_t *w, int timeout, int period);
int uev_timer_start    (uev_t *w);
int uev_timer_stop     (uev_t *w);

int uev_cron_init      (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, time_t when, time_t interval);
int uev_cron_set       (uev_t *w, time_t when, time_t interval);
int uev_cron_start     (uev_t *w);
int uev_cron_stop      (uev_t *w);

int uev_signal_init    (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int signo);
int uev_signal_set     (uev_t *w, int signo);
int uev_signal_start   (uev_t *w);
int uev_signal_stop    (uev_t *w);

int uev_event_init     (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg);
int uev_event_post     (uev_t *w);
int uev_event_stop     (uev_t *w);

#endif /* LIBUEV_UEV_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
