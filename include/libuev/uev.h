/* libuev - Asynchronous event loop library
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

#include "queue.h"

/* Elapsed time between two times() tick measurements in msec */
#define TIME_DIFF_MSEC(now, then)				\
	( (now - then) / clock_tick  * 1000 +			\
	 ((now - then) % clock_tick) * 1000 / clock_tick )

/* Forward declare due to dependencys, don't try this at home. */
struct uev;

/* I/O direction */
typedef enum {
	UEV_DIR_INBOUND = 0,
	UEV_DIR_OUTBOUND = 1
} uev_dir_t;

/* I/O event watcher */
typedef struct uev_io {
	TAILQ_ENTRY(uev_io) link;
	TAILQ_ENTRY(uev_io) gc;

	int            fd;              ///< File descriptor
	uev_dir_t      dir;             ///< Direction: in or out
	int            index;           ///< Index in poll array

	void         (*handler)(struct uev *, struct uev_io *, void *);
	void          *data;
} uev_io_t;

/* Timer event watcher */
typedef struct uev_timer {
	TAILQ_ENTRY(uev_timer) link;

	int            timeout;	        ///< Timeout in milliseconds
	int            period;          ///< Period time, in milliseconds

	void         (*handler)(struct uev *, struct uev_timer *, void *);
	void          *data;
} uev_timer_t;

/* Main libuev context type */
typedef struct {
	TAILQ_HEAD(, uev_io)     io_list;      ///< File handlers
	TAILQ_HEAD(, uev_timer)  timer_list;   ///< Timer handlers

	TAILQ_HEAD(, uev_io)     io_delist;    ///< List of file handles to be garbage collected
	TAILQ_HEAD(, uev_timer)  timer_delist; ///< List of timer handles to be garbage collected

	clock_t           base_time;    ///< Time at last timer tick
	clock_t           next_time;    ///< Next timer tick epoch
	int               use_next;     ///< True if next timer epoch is set

	int               exiting;      ///< True if the application is terminating
} uev_t;

/* Callbacks for different watchers */
typedef void (*uev_io_cb_t)   (uev_t *ctx, uev_io_t    *w, int fd, void *data);
typedef void (*uev_timer_cb_t)(uev_t *ctx, uev_timer_t *w,         void *data);

/* Public interface */
uev_io_t    *uev_io_create    (uev_t *ctx, uev_io_cb_t cb, void *data, int fd, uev_dir_t dir);
int          uev_io_delete    (uev_t *ctx, uev_io_t *w);

int          uev_timer_set    (uev_t *ctx, uev_timer_t *w, int timeout, int period);
uev_timer_t *uev_timer_create (uev_t *ctx, uev_timer_cb_t cb, void *data, int timeout, int period);
int          uev_timer_delete (uev_t *ctx, uev_timer_t *w);

void         uev_run          (uev_t *ctx);
void         uev_run_tick     (uev_t *ctx, int msec);
int          uev_run_pending  (uev_t *ctx);
void         uev_exit         (uev_t *ctx);

uev_t       *uev_ctx_create   (void);
void         uev_ctx_delete   (uev_t *uev);

#endif /* LIBUEV_UEV_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
