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

#include "check.h"
#include <time.h>
#include <sys/time.h>

#define TIMEOUT  2
#define INTERVAL 2

struct timeval tv;

static void cron_job(uev_t *w, void *UNUSED(arg), int UNUSED(events))
{
	static int laps = 3;
	struct timeval now;

	/* Update timer every lap */
	tv.tv_sec += TIMEOUT;

	gettimeofday(&now, NULL);
	printf("Cron job HELO %s", ctime(&now.tv_sec));
	fail_unless(now.tv_sec == tv.tv_sec);

	if (!laps--)
		uev_exit(w->ctx);
}

int main(void)
{
	uev_t cron_watcher;
	uev_ctx_t ctx;
	time_t when, interval;

	uev_init(&ctx);

	gettimeofday(&tv, NULL);
	printf("Start of test %s", ctime(&tv.tv_sec));
	printf("Expected cron %s", ctime(&when));
	
	when     = tv.tv_sec + TIMEOUT;
	interval = INTERVAL;
	uev_cron_init(&ctx, &cron_watcher, cron_job, NULL, when, interval);

	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
