/* libuEv - Private methods and data types, do not use directly!
 *
 * Copyright (c) 2012       Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013-2024  Joachim Wiberg <troglobit()gmail!com>
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
#include <sys/signalfd.h>

/*
 * List functions.
 */
#define _UEV_FOREACH(node, list)		\
	for (typeof (node) next, node = list;	\
	     node && (next = node->next, 1);	\
	     node = next)

#define _UEV_INSERT(node, list) do {		\
	typeof (node) next;			\
	next       = list;			\
	list       = node;			\
	if (next)				\
		next->prev = node;		\
	node->next = next;			\
	node->prev = NULL;			\
} while (0)

#define _UEV_REMOVE(node, list) do {		\
	typeof (node) prev, next;		\
	prev = node->prev;			\
	next = node->next;			\
	if (prev)				\
		prev->next = next;		\
	if (next)				\
		next->prev = prev;		\
	node->prev = NULL;			\
	node->next = NULL;			\
	if (list == node)			\
		list = next;			\
} while (0)

/* I/O, timer, or signal watcher */
typedef enum {
	UEV_IO_TYPE = 1,
	UEV_SIGNAL_TYPE,
	UEV_TIMER_TYPE,
	UEV_CRON_TYPE,
	UEV_EVENT_TYPE,
} uev_type_t;

/* Event mask, used internally only. */
#define UEV_EVENT_MASK  (UEV_ERROR | UEV_READ | UEV_WRITE | UEV_PRI |	\
			 UEV_RDHUP | UEV_HUP  | UEV_EDGE  | UEV_ONESHOT)

/* Main libuEv context type, internal use only! */
struct uev_ctx {
	int             running;
	int             fd;	    /* For epoll() */
	int             maxevents;  /* For epoll() */
	struct uev     *watchers;
	uint32_t        workaround; /* For workarounds, e.g. redirected stdin */
};

/* Forward declare due to dependencys, don't try this at home kids. */
struct uev;

/* This is used to hide all private data members in uev_t */
#define uev_private_t                                           \
	struct uev     *next, *prev;				\
								\
	int             active;                                 \
	int             events;                                 \
								\
	/* Watcher callback with optional argument */           \
	void          (*cb)(struct uev *, void *, int);         \
	void           *arg;                                    \
								\
	/* Arguments for different watchers */			\
	union {							\
		/* Cron watchers */				\
		struct {					\
			time_t when;				\
			time_t interval;			\
		} c;						\
								\
		/* Timer watchers, time in milliseconds */	\
		struct {					\
			int timeout;				\
			int period;				\
		} t;						\
	} u;							\
								\
	/* Watcher type */					\
	uev_type_t

/* Internal API for dealing with generic watchers */
int _uev_watcher_init  (struct uev_ctx *ctx, struct uev *w, uev_type_t type,
			void (*cb)(struct uev *, void *, int), void *arg,
			int fd, int events);
int _uev_watcher_start (struct uev *w);
int _uev_watcher_stop  (struct uev *w);
int _uev_watcher_active(struct uev *w);
int _uev_watcher_rearm (struct uev *w);

#endif /* LIBUEV_PRIVATE_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
