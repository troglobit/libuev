/* libuEv - Micro event loop library
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

#ifndef LIBUEV_UEV_H_
#define LIBUEV_UEV_H_

#include <stdio.h>
#include <sys/epoll.h>
#include <sys/queue.h>

/* Max. number of simulateneous events */
#define UEV_MAX_EVENTS  10

/* I/O events, for signals and timers revents will always be UEV_READ */
#define UEV_NONE        0
#define UEV_READ        EPOLLIN
#define UEV_WRITE       EPOLLOUT

/* Run flags */
#define UEV_ONCE        1
#define UEV_NONBLOCK    2

/* I/O, timer, or signal watcher */
typedef enum {
	UEV_IO_TYPE = 1,
	UEV_TIMER_TYPE,
	UEV_SIGNAL_TYPE,
} uev_type_t;

/* Forward declare due to dependencys, don't try this at home kids. */
struct uev;

/* Main libuEv context type */
typedef struct {
	int             running;
	int             fd;	/* For epoll() */
	LIST_HEAD(,uev) watchers;
} uev_ctx_t;

/* I/O event watcher */
typedef struct uev {
	LIST_ENTRY(uev) link;	/* For queue.h linked list */

	/* Common to all watchers */
	uev_ctx_t      *ctx;
	uev_type_t      type;
	int             active;
	int             fd;
	int             events;

	/* Watcher callback with optional argument */
	void          (*cb)(struct uev *, struct uev *, void *);
	void           *arg;

	/* Timer watchers, time in milliseconds */
	int             timeout;
	int             period;

	/* Signal watchers */
	int             signo;
} uev_t;

/* Generic callback for watchers */
typedef void (uev_cb_t)(uev_ctx_t *ctx, uev_t *w, void *arg, int events);

/* Private methods, do not use directly! */
int uev_watcher_init   (uev_ctx_t *ctx, uev_t *w, uev_type_t type, uev_cb_t *cb, void *arg, int fd, int events);
int uev_watcher_start  (uev_t *w);
int uev_watcher_stop   (uev_t *w);

/* Public interface */
int uev_init           (uev_ctx_t *ctx);
int uev_exit           (uev_ctx_t *ctx);
int uev_run            (uev_ctx_t *ctx, int flags);

int uev_io_init        (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int fd, int events);
int uev_io_set         (uev_t *w, int fd, int events);
int uev_io_stop        (uev_t *w);

int uev_timer_init     (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int timeout, int period);
int uev_timer_set      (uev_t *w, int timeout, int period);
int uev_timer_stop     (uev_t *w);

int uev_signal_init    (uev_ctx_t *ctx, uev_t *w, uev_cb_t *cb, void *arg, int signo);
int uev_signal_set     (uev_t *w, int signo);
int uev_signal_stop    (uev_t *w);

#endif /* LIBUEV_UEV_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
