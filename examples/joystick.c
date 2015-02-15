/* Simple example of how to use libuEv with Linux joystick API
 *
 * Copyright (c) 2014-2015  Joachim Nilsson <troglobit()gmail!com>
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

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "uev.h"

#define UNUSED(arg) arg __attribute__ ((unused))

struct js_event {
	uint32_t time;		/* event timestamp in milliseconds */
	int16_t  value;		/* value */
	uint8_t  type;		/* event type */
	uint8_t  number;	/* axis/button number */
} e;

static void joystick_cb(uev_ctx_t *UNUSED(ctx), uev_t *w, void *UNUSED(arg), int UNUSED(events))
{
	if (read (w->fd, &e, sizeof(e)) < 0)
		errx(errno, "Failed reading joystick event");

	switch (e.type) {
	case 1:
		if (e.value) printf("Button %d pressed\n", e.number);
		else 	     printf("Button %d released\n", e.number);
		break;

	case 2:
		printf("Joystick axis %d moved, value %d!\n", e.number, e.value);
		break;
	}
}

int main(void)
{
	int       fd = open("/dev/input/js1", O_RDONLY, O_NONBLOCK);
	uev_t     js1_watcher;
	uev_ctx_t ctx;

	if (fd < 0)
		errx(errno, "Cannot find a joystick attached.");

	uev_init(&ctx);
	uev_io_init(&ctx, &js1_watcher, joystick_cb, NULL, fd, UEV_READ);

	puts("Starting, press Ctrl-C to exit.");

	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  compile-command: "gcc -o joystick joystick.c io.o timer.o signal.o main.o"
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
