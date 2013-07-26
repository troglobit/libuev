/* libuev - Micro event loop library
 *
 * Copyright (c) 2012  Flemming Madsen <flemming!madsen()madsensoft!dk>
 * Copyright (c) 2013  Joachim Nilsson <troglobit()gmail!com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "uev.h"


/**
 * Create an I/O watcher
 * @param ctx     A valid libuev context
 * @param handler I/O callback
 * @param data    Optional callback argument
 * @param fd      File descriptor to watch
 * @param dir     Direction of I/O to watch for: %UEV_DIR_INBOUND, or %UEV_DIR_OUTBOUND
 *
 * @return The new I/O watcher, or %NULL if invalid pointers or out or memory.
 */
uev_t *uev_io_create(uev_ctx_t *ctx, uev_cb_t *handler, void *data, int fd, uev_dir_t dir)
{
	return uev_watcher_create(ctx, UEV_FILE_TYPE, fd, dir, handler, data);
}

/**
 * Delete an I/O watcher
 * @param ctx  A valid libuev context
 * @param w    I/O watcher
 *
 * @return POSIX OK(0) or non-zero with @param errno set.
 */
int uev_io_delete(uev_ctx_t *ctx, uev_t *w)
{
	return uev_watcher_delete(ctx, w);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
