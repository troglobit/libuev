/* Verifies cleanup on failure to set up a context or watcher */
#include "check.h"
#include <sys/resource.h>

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

/* A failed uev_init() must not leave stdin as the context descriptor */
static void ctx_init(void)
{
	struct rlimit old, lim;
	uev_ctx_t ctx;
	int rc;

	fail_unless(getrlimit(RLIMIT_NOFILE, &old) == 0);
	lim = old;
	lim.rlim_cur = 0;
	fail_unless(setrlimit(RLIMIT_NOFILE, &lim) == 0);

	rc = uev_init(&ctx);

	fail_unless(setrlimit(RLIMIT_NOFILE, &old) == 0);
	fail_unless(rc == -1);
	fail_unless(ctx.fd == -1);
}

int main(void)
{
	event_init();
	ctx_init();

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
