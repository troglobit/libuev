/* libuEv - Micro event loop library
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2015  Joachim Nilsson <troglobit()gmail!com>
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

#ifndef LIBUEV_UEV_H_
#define LIBUEV_UEV_H_

#include "private.h"

/* Max. number of simulateneous events */
#define UEV_MAX_EVENTS  10

/* I/O events, for signals and timers revents will always be UEV_READ */
#define UEV_NONE        0
#define UEV_READ        EPOLLIN
#define UEV_WRITE       EPOLLOUT

/* Run flags */
#define UEV_ONCE        1
#define UEV_NONBLOCK    2

/* Event watcher */
typedef struct uev {
	/* Private data for libuEv internal engine */
	uev_private_t   type;

	/* Public data for users to reference  */
	int             fd;
	uev_ctx_t      *ctx;
} uev_t;

/* Generic callback for watchers */
typedef void (uev_cb_t)(uev_t *w, void *arg, int events);

/* Public interface */
int uev_init           (uev_ctx_t *ctx);
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

int uev_signal_init    (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int signo);
int uev_signal_set     (uev_t *w, int signo);
int uev_signal_start   (uev_t *w);
int uev_signal_stop    (uev_t *w);

#endif /* LIBUEV_UEV_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
