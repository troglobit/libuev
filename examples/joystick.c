/* Simple example of how to use libuEv with Linux joystick API
 *
 * Copyright (c) 2014-2017  Joachim Nilsson <troglobit()gmail!com>
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

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "uev.h"

struct js_event {
	uint32_t time;		/* event timestamp in milliseconds */
	int16_t  value;		/* value */
	uint8_t  type;		/* event type */
	uint8_t  number;	/* axis/button number */
} e;

/*
 * Always check for UEV_ERROR in watcher callback!
 * Possibly joystick was unplugged
 */
static void joystick_cb(uev_t *w, void *arg, int events)
{
	if (UEV_ERROR == events)
		errx(EBADF, "Unrecoverable error, exiting");

	if (read(w->fd, &e, sizeof(e)) < 0)
		errx(errno, "Failed reading joystick event");

	switch (e.type) {
	case 1:
		printf("Button %d %s\n", e.number, e.value ? "pressed" : "released");
		break;

	case 2:
		printf("Joystick axis %d moved, value %d!\n", e.number, e.value);
		break;
	}
}

int main(void)
{
	int fd;
	uev_t js1_watcher;
	uev_ctx_t ctx;

	fd = open("/dev/input/js1", O_RDONLY, O_NONBLOCK);
	if (fd < 0)
		errx(errno, "Cannot find a joystick attached.");

	uev_init(&ctx);
	uev_io_init(&ctx, &js1_watcher, joystick_cb, NULL, fd, UEV_READ);

	puts("Starting, press Ctrl-C to exit.");

	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  compile-command: "make joystick"
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
