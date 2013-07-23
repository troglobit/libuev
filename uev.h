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

#ifndef LIBUEV_UEV_H_
#define LIBUEV_UEV_H_

#include <sys/queue.h>

/* Max. number of simulateneous events */
#define UEV_MAX_EVENTS 10

/* Forward declare due to dependencys, don't try this at home. */
struct uev;

/* I/O direction */
typedef enum {
	UEV_DIR_INBOUND = 0,
	UEV_DIR_OUTBOUND = 1
} uev_dir_t;

/* File, Timer, ... */
typedef enum {
	UEV_FILE_TYPE = 1,
	UEV_TIMER_TYPE,
} uev_type_t;

/* I/O event watcher */
typedef struct uev_io {
	LIST_ENTRY(uev_io) link;

	uev_type_t     type;

	int            fd;              ///< File descriptor
	uev_dir_t      dir;             ///< Direction: in or out

	int            timeout;	        ///< Timeout in milliseconds
	int            period;          ///< Period time, in milliseconds

	void         (*handler)(struct uev *, struct uev_io *, void *);
	void          *data;
} uev_io_t;

/* Main libuev context type */
typedef struct {
	int                   running;
	int                   efd;           /* For epoll() */
	LIST_HEAD(, uev_io)  watchers;
} uev_t;

/* Generic callback for watchers */
typedef void (uev_cb_t)        (uev_t *ctx, uev_io_t *w, void *data);

/* Private methods, do not use directly! */
uev_io_t    *uev_watcher_create(uev_t *ctx, uev_type_t type, int fd, uev_dir_t dir, uev_cb_t *handler, void *data);
int          uev_watcher_delete(uev_t *ctx, uev_io_t *w);

/* Public interface */
uev_io_t    *uev_io_create     (uev_t *ctx, uev_cb_t *cb, void *data, int fd, uev_dir_t dir);
int          uev_io_delete     (uev_t *ctx, uev_io_t *w);

int          uev_timer_set     (uev_t *ctx, uev_io_t *w, int timeout, int period);
uev_io_t    *uev_timer_create  (uev_t *ctx, uev_cb_t *cb, void *data, int timeout, int period);
int          uev_timer_delete  (uev_t *ctx, uev_io_t *w);

uev_t       *uev_ctx_create    (void);
void         uev_ctx_delete    (uev_t *uev);

int          uev_run           (uev_t *ctx);
void         uev_exit          (uev_t *ctx);

#endif /* LIBUEV_UEV_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
