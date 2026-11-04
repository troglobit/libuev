/* Verifies cleanup on failure to set up a context or watcher */
#include "check.h"

/* A failed uev_event_init() must not leak its eventfd */
static void event_init(void)
{
	uev_ctx_t ctx;
	uev_t w;
	int fd;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(close(ctx.fd) == 0);

	fd = dup(STDIN_FILENO);
	fail_unless(fd >= 0);
	close(fd);

	fail_unless(uev_event_init(&ctx, &w, NULL, NULL) != 0);
	fail_unless(w.fd == -1);
	fail_unless(dup(STDIN_FILENO) == fd);
	close(fd);
}

int main(void)
{
	event_init();

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
