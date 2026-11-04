/* Verifies that UEV_NONBLOCK returns when no event is pending */
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

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_event_init(&ctx, &w, cb, NULL) == 0);

	alarm(2);
	fail_unless(uev_run(&ctx, UEV_NONBLOCK) == 0);
	fail_unless(fired == 0);

	fail_unless(uev_event_post(&w) == 0);
	fail_unless(uev_run(&ctx, UEV_NONBLOCK) == 0);
	fail_unless(fired == 1);

	return uev_exit(&ctx);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
