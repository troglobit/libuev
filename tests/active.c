#include "check.h"
#include <time.h>
#include <sys/time.h>

#define TIMEOUT1 2		/* 2 sec */
#define TIMEOUT2 4		/* 4 sec */
#define DAYLY    (24 * 60 * 60)	/* Seconds in one day */

uev_t timer1;
uev_t timer2;
uev_t cron1;
uev_t cron2;

static void cb(uev_t *w, void *UNUSED(arg), int UNUSED(events))
{
	fail_unless(w == &timer1);

	fail_unless(uev_timer_active(&timer2));
	fail_unless(uev_cron_active(&cron1));
	fail_unless(uev_cron_active(&cron2));

	uev_timer_stop(&timer2);
	uev_timer_stop(&cron2);

	fail_unless(!uev_timer_active(&timer2));
	fail_unless(!uev_cron_active(&cron2));

	uev_exit(w->ctx);
}

int main(void)
{
	uev_ctx_t ctx;
	struct timeval tomorrow;

	gettimeofday(&tomorrow, NULL);
	tomorrow.tv_sec += 24 * 60 * 60;

	uev_init(&ctx);
	uev_timer_init(&ctx, &timer1, cb, NULL, TIMEOUT1 * 1000, 0);
	uev_timer_init(&ctx, &timer2, cb, NULL, TIMEOUT2 * 1000, 0);

	uev_cron_init(&ctx, &cron1, cb, NULL, tomorrow.tv_sec, 24 * 60 * 60);
	uev_cron_init(&ctx, &cron2, cb, NULL, tomorrow.tv_sec, 24 * 60 * 60);

	return uev_run(&ctx, 0);
}
