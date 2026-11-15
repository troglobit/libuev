/* Verifies UEV_ONCE with stdin redirected from a regular file */
#include "check.h"

static int calls;

static void cb(uev_t *w, void *arg, int events)
{
	char ch;

	calls++;
	read(w->fd, &ch, 1);
}

int main(void)
{
	FILE *fp = tmpfile();
	uev_ctx_t ctx;
	uev_t w;

	fail_unless(fp != NULL);
	fail_unless(fputs("abc", fp) >= 0);
	rewind(fp);
	fail_unless(dup2(fileno(fp), STDIN_FILENO) == STDIN_FILENO);

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_io_init(&ctx, &w, cb, NULL, STDIN_FILENO, UEV_READ) == 0);

	alarm(5);
	fail_unless(uev_run(&ctx, UEV_ONCE) == 0);
	fail_unless(calls == 1);

	/* Without UEV_ONCE the rest is drained, including EOF */
	fail_unless(uev_run(&ctx, 0) == 0);
	fail_unless(calls == 4);

	return uev_exit(&ctx);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
