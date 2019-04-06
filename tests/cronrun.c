/* Test cron API in libuEv event library
 *
 * Copyright (c) 2016-2019  Joachim Nilsson <troglobit()gmail!com>
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

int result = -1;
struct timeval tv;

static void cron_job(uev_t *w, void *arg, int events)
{
	struct timeval now;
	static int laps = 3;

	if (UEV_ERROR == events)
		fprintf(stderr, "cron watcher failed, ignoring ...\n");

	/* Update timer every lap */
	tv.tv_sec += TIMEOUT;

	gettimeofday(&now, NULL);
	printf("Cron job HELO %s", ctime(&now.tv_sec));
	fail_unless(now.tv_sec == tv.tv_sec);

	if (!laps--) {
		result = 0;
		uev_exit(w->ctx);
	}
}

int main(void)
{
	uev_ctx_t ctx;
	time_t when, interval;
	uev_t cron;
	int rc;

	uev_init(&ctx);

	gettimeofday(&tv, NULL);
	when = tv.tv_sec + TIMEOUT;

	printf("Start of test %s", ctime(&tv.tv_sec));
	printf("Expected cron %s", ctime(&when));

	interval = INTERVAL;
	uev_cron_init(&ctx, &cron, cron_job, NULL, when, interval);

	rc = uev_run(&ctx, 0);
	fail_unless(result == 0);

	return rc;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
