#include "check.h"
#include <time.h>
#include <sys/time.h>

#define TIMEOUT 2		/* 2 sec */
struct timeval start;

static void cb(uev_t *w, void *UNUSED(arg), int UNUSED(events))
{
	struct timeval now;

	gettimeofday(&now, NULL);
	fail_unless(now.tv_sec == start.tv_sec + TIMEOUT);

	uev_exit(w->ctx);
}

int main(void)
{
	uev_t w;
	uev_ctx_t ctx;

	uev_init(&ctx);
	uev_timer_init(&ctx, &w, cb, NULL, TIMEOUT * 1000, 0);
	gettimeofday(&start, NULL);

	return uev_run(&ctx, 0);
}
