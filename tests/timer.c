#include "check.h"
#include <time.h>
#include <sys/time.h>

#define TIMEOUT 2		/* 2 sec */

int result = -1;
struct timeval start;

static void cb(uev_t *w, void *arg, int events)
{
	struct timeval now;

	if (UEV_ERROR == events)
		fprintf(stderr, "timer watcher failed, ignoring ...\n");

	gettimeofday(&now, NULL);
	fail_unless(now.tv_sec == start.tv_sec + TIMEOUT);

	result = 0;
	uev_exit(w->ctx);
}

int main(void)
{
	uev_ctx_t ctx;
	uev_t w;
	int rc;

	uev_init(&ctx);
	uev_timer_init(&ctx, &w, cb, NULL, TIMEOUT * 1000, 0);
	gettimeofday(&start, NULL);

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
