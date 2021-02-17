/* Example with fork and signals, in this case Ctrl-C, from terminal
 * 
 * A common problem for users of libuEV is how to combine it with the
 * traditional aspects of UNIX.  Since libuEV is just a wrapper for
 * signalfd() and epoll(), in this case, it can be worth a dedicated
 * example.  After all, command line use with Ctrl-C is one of the most
 * common use-cases.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "src/uev.h"


void timeout(uev_t *w, void *arg, int events)
{
	char *id = (char *)arg;

	if (UEV_ERROR == events)
		puts("Ignoring timer watcher error ...");

	printf("I am the %s robot\n", id);
}

void teaser(uev_t *w, void *arg, int events)
{
	(void)arg;

	if (UEV_ERROR == events)
		puts("Ignoring SIGINT watcher error ...");

	puts("\nWe are here to protect you ...");
	puts("Sorry, you actually need to press Ctrl-\\ to exit.");
}

void cleanup(uev_t *w, void *arg, int events)
{
	(void)arg;

	if (UEV_ERROR == events)
		puts("Ignoring signal watcher error ...");

	/* Graceful exit, with optional cleanup ... */
	puts("\nAgainst the terrible secret of space.");

	uev_exit(w->ctx);
}

int main(int argc, char **argv)
{
	uev_ctx_t ctx;
	uev_t timerw;
	uev_t sig1w;
	uev_t sig2w;
	char *robot;
	pid_t pid;

	pid = fork();
	if (pid == -1)
		return 1;
	if (pid > 0)
		robot = "Shover";
	else
		robot = "Pusher";

	setsid();			/* puts child in new process group */
	uev_init(&ctx);

	/* example worker, a timer callback */
	uev_timer_init(&ctx, &timerw, timeout, robot, 100, 2000);

	/* the owls are not what they seem ... */
	uev_signal_init(&ctx, &sig1w, teaser,  NULL, SIGINT);
	uev_signal_init(&ctx, &sig2w, cleanup, NULL, SIGQUIT);

	puts("Starting, press Ctrl-C to exit.");

	return uev_run(&ctx, 0);
}

/**
 * Local Variables:
 *  compile-command: "make forky; ./forky"
 *  c-file-style: "linux"
 * End:
 */
