/* Verifies that a stopped cron watcher can be started again */
#include "check.h"

static void count(uev_t *w, void *arg, int events)
{
	int *fired = arg;

	fail_unless(events == UEV_READ);
	(*fired)++;
}

static void restart_after_overrun(void)
{
	const time_t interval = 3600;
	time_t when = time(NULL) - interval - interval / 2;
	uev_ctx_t ctx;
	uev_t cron;
	int fired = 0;

	fail_unless(uev_init(&ctx) == 0);
	/* Two hourly deadlines have passed before the first read. */
	fail_unless(uev_cron_init(&ctx, &cron, count, &fired, when, interval) == 0);
	alarm(2);
	fail_unless(uev_run(&ctx, UEV_ONCE) == 0);
	alarm(0);
	fail_unless(fired == 1);
	fail_unless(cron.u.c.when == when + 2 * interval);

	fail_unless(uev_cron_stop(&cron) == 0);
	fail_unless(uev_cron_start(&cron) == 0);
	fail_unless(uev_run(&ctx, UEV_ONCE | UEV_NONBLOCK) == 0);
	fail_unless(fired == 1);
	fail_unless(uev_exit(&ctx) == 0);
}

int main(void)
{
	time_t when = time(NULL) + 3600;
	uev_ctx_t ctx;
	uev_t cron;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_cron_init(&ctx, &cron, NULL, NULL, when, 3600) == 0);
	fail_unless(uev_cron_stop(&cron) == 0);
	fail_unless(!uev_cron_active(&cron));

	fail_unless(uev_cron_start(&cron) == 0);
	fail_unless(uev_cron_active(&cron));
	fail_unless(cron.type == UEV_CRON_TYPE);
	fail_unless(cron.u.c.when == when);
	fail_unless(cron.u.c.interval == 3600);

	fail_unless(uev_exit(&ctx) == 0);

	restart_after_overrun();

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
