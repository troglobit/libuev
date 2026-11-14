/* Verifies that timers keep their deadline between calls to uev_run() */
#include "check.h"

static int fired;

static void cb(uev_t *w, void *arg, int events)
{
	fired++;
}

int main(void)
{
	uev_ctx_t ctx;
	uev_t w;
	int i;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_timer_init(&ctx, &w, cb, NULL, 50, 0) == 0);

	for (i = 0; i < 15; i++) {
		fail_unless(uev_run(&ctx, UEV_ONCE | UEV_NONBLOCK) == 0);
		usleep(10000);
	}
	fail_unless(fired == 1);

	return uev_exit(&ctx);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
