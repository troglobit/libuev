/*
 * Copyright 2003 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Mon 03/10/2003 - Modified by Davide Libenzi <davidel@xmailserver.org>
 *
 *     Added chain event propagation to improve the sensitivity of
 *     the measure respect to the event loop efficency.
 *
 * Sun 04/08/2013 - Modified by Joachim Nilsson <troglobit@gmail.com>
 *
 *     Adaptations for libuEv, no libev/libevent API wrappers available.
 *     Reindent to Linux coding style
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define USE_PIPES
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "uev.h"

#define UNUSED(arg) arg __attribute__ ((unused))

typedef struct {
	int index;
} myarg_t;

static int num_pipes, num_active, num_writes;
static int timers, count, writes, fired;
static myarg_t *args;
static int *pipes;
static uev_t *evio;
static uev_t *evto;

static void read_cb(uev_t *w, void *arg, int UNUSED(events))
{
	int idx, widx;
	u_char ch;
	myarg_t *m = arg;

	idx  = m->index;
	widx = idx + 1;
	if (timers)
		uev_timer_set(&evto[idx], 10000 + drand48() * 1000, 0);

	count += read(w->fd, &ch, sizeof(ch));
	if (writes) {
		if (widx >= num_pipes)
			widx -= num_pipes;

		if (write(pipes[2 * widx + 1], "e", 1) < 0) {
			perror("write()");
			abort();
		}

		writes--;
		fired++;
	}
}

static void timer_cb(uev_t UNUSED(*w), void UNUSED(*arg), int UNUSED(events))
{
	/* nop */
}

static struct timeval *run_once(uev_ctx_t *ctx)
{
	int *cp, i, space;
	static struct timeval ta, ts, te;

	gettimeofday(&ta, NULL);
	for (cp = pipes, i = 0; i < num_pipes; i++, cp += 2) {
		uev_io_set(&evio[i], cp[0], UEV_READ);

		if (timers)
			uev_timer_set(&evto[i], 10000 + drand48() * 1000, 0);
	}

	uev_run(ctx, UEV_ONCE | UEV_NONBLOCK);

	fired = 0;
	space = num_pipes / num_active;
	space = space * 2;
	for (i = 0; i < num_active; i++, fired++) {
		if (write(pipes[i * space + 1], "e", 1) < 0) {
			perror("write()");
			abort();
		}
	}

	count = 0;
	writes = num_writes;
	{
		int xcount = 0;

		gettimeofday(&ts, NULL);

		do {
			uev_run(ctx, UEV_ONCE | UEV_NONBLOCK);
			xcount++;
		} while (count != fired);

		gettimeofday(&te, NULL);

//		if (xcount != count) fprintf(stderr, "Xcount: %d, Rcount: %d\n", xcount, count);
	}

	timersub(&te, &ta, &ta);
	timersub(&te, &ts, &ts);
	fprintf(stdout, "%8ld %8ld\n",
		ta.tv_sec * 1000000L + ta.tv_usec,
		ts.tv_sec * 1000000L + ts.tv_usec);

	return &te;
}

int main(int argc, char **argv)
{
	struct rlimit rl;
	int i, c;
	int *cp;
	uev_ctx_t ctx;
	extern char *optarg;

	num_pipes = 100;
	num_active = 1;
	num_writes = num_pipes;
	while ((c = getopt(argc, argv, "a:n:tw:")) != -1) {
		switch (c) {
		case 'a':
			num_active = atoi(optarg);
			break;

		case 'n':
			num_pipes = atoi(optarg);
			break;

		case 't':
			timers = 1;
			break;

		case 'w':
			num_writes = atoi(optarg);
			break;

		default:
			fprintf(stderr, "Illegal argument \"%c\"\n", c);
			return 1;
		}
	}

	rl.rlim_cur = rl.rlim_max = num_pipes * 3 + 50;
	if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
		perror("setrlimit");
		return 1;
	}

	args   = calloc(num_pipes, sizeof(myarg_t));
	evio   = calloc(num_pipes, sizeof(uev_t));
	evto   = calloc(num_pipes, sizeof(uev_t));
	pipes  = calloc(num_pipes * 2, sizeof(int));
	if (!args || !evio || !evto || !pipes) {
		perror("calloc");
		return 1;
	}

	uev_init(&ctx);

	for (cp = pipes, i = 0; i < num_pipes; i++, cp += 2) {
		if (timers)
			uev_timer_init(&ctx, &evto[i], timer_cb, NULL, 0, 0);
		args[i].index = i;

#ifdef USE_PIPES
		if (pipe2(cp, O_NONBLOCK) == -1) {
#else
		if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, cp) == -1) {
#endif
			perror("pipe");
			exit(1);
		}

		uev_io_init(&ctx, &evio[i], read_cb, &args[i], cp[0], UEV_READ);
	}

	for (i = 0; i < 2; i++)
		run_once(&ctx);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
