/* Verifies that a stopped cron watcher can be started again */
#include "check.h"

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

	return uev_exit(&ctx);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
