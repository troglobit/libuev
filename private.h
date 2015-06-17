/* libuEv - Private methods and data types, do not use directly!
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

#ifndef LIBUEV_PRIVATE_H_
#define LIBUEV_PRIVATE_H_

#include <stdio.h>
#include <sys/epoll.h>
#include "queue.h"	       /* OpenBSD queue.h > old GLIBC version */

/* I/O, timer, or signal watcher */
typedef enum {
	UEV_IO_TYPE = 1,
	UEV_TIMER_TYPE,
	UEV_SIGNAL_TYPE,
} uev_type_t;

/* Event mask, used internally only. */
#define UEV_EVENT_MASK  (UEV_READ | UEV_WRITE)

/* Main libuEv context type */
typedef struct {
	int             running;
	int             fd;     /* For epoll() */
	uint32_t        errors;
	LIST_HEAD(,uev) watchers;
} uev_ctx_t;

/* Forward declare due to dependencys, don't try this at home kids. */
struct uev;

/* This is used to hide all private data members in uev_t */
#define uev_private_t                                           \
	LIST_ENTRY(uev) link;   /* For queue.h linked list */   \
								\
	int             active;                                 \
	int             events;                                 \
								\
	/* Watcher callback with optional argument */           \
	void          (*cb)(struct uev *, void *, int);         \
	void           *arg;                                    \
								\
	/* Timer watchers, time in milliseconds */		\
	int             timeout;                                \
	int             period;                                 \
								\
	/* Signal watchers */                                   \
	int             signo;                                  \
								\
	/* Watcher type */					\
	uev_type_t

/* Internal API for dealing with generic watchers */
int uev_watcher_init (uev_ctx_t *ctx, struct uev *w, uev_type_t type,
		      void (*cb)(struct uev *, void *, int), void *arg,
		      int fd, int events);
int uev_watcher_start(struct uev *w);
int uev_watcher_stop (struct uev *w);

#endif /* LIBUEV_PRIVATE_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
