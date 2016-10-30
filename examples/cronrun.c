/* Test cron API in libuEv event library
 *
 * Copyright (c) 2016  Joachim Nilsson <troglobit()gmail!com>
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

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "uev.h"

#define UNUSED(arg) arg __attribute__ ((unused))


static void cron_job(uev_t *w, void *arg, int events)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	printf("Cron job HELO %s", ctime(&tv.tv_sec));
}

int main(void)
{
	uev_t cron_watcher;
	uev_ctx_t ctx;
	time_t when, interval;
	struct timeval tv;

	/* Initialize libuEv */
	uev_init(&ctx);

	/* Cron job */
	gettimeofday(&tv, NULL);
	when     = tv.tv_sec + 30;
	interval = 30;
	uev_cron_init(&ctx, &cron_watcher, cron_job, NULL, when, interval);

	/* Start event loop */
	printf("Start of test %s", ctime(&tv.tv_sec));
	printf("Expected cron %s", ctime(&when));

	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
