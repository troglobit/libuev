/* Verify context state when event loop initialization fails.
 *
 * Copyright (c) 2026  libuev contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "check.h"
#include <errno.h>
#include <sys/resource.h>

int main(void)
{
	struct rlimit limit, exhausted;
	uev_ctx_t ctx;
	int saved_errno;

	fail_unless(getrlimit(RLIMIT_NOFILE, &limit) == 0);
	exhausted = limit;
	exhausted.rlim_cur = 0;
	fail_unless(setrlimit(RLIMIT_NOFILE, &exhausted) == 0);

	fail_unless(uev_init(&ctx) == -1);
	saved_errno = errno;

	/* Restore the limit before checking the failed context. */
	fail_unless(setrlimit(RLIMIT_NOFILE, &limit) == 0);
	fail_unless(saved_errno == EMFILE);
	fail_unless(ctx.fd == -1);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
