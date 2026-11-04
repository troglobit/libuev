/* Verifies that a cron watcher with start time zero runs now */
#include "check.h"

static int fired;

static void cb(uev_t *w, void *arg, int events)
{
	fired++;
	uev_exit(w->ctx);
}

int main(void)
{
	uev_ctx_t ctx;
	uev_t w;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_cron_init(&ctx, &w, cb, NULL, 0, 0) == 0);

	alarm(2);
	fail_unless(uev_run(&ctx, 0) == 0);
	fail_unless(fired == 1);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
