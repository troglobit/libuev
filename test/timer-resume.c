#include <poll.h>
#include <sys/timerfd.h>
#include "check.h"

struct result {
	int calls;
	int reset;
	int exit;
};

static void cb(uev_t *w, void *arg, int events)
{
	struct result *result = arg;

	fail_unless(events & UEV_READ);
	fail_unless(!(events & UEV_ERROR));
	result->calls++;
	if (result->reset && result->calls == 1)
		fail_unless(uev_timer_set(w, 20, 0) == 0);
	if (result->exit)
		fail_unless(uev_exit(w->ctx) == 0);
}

static void run_once(uev_ctx_t *ctx)
{
	fail_unless(uev_run(ctx, UEV_ONCE | UEV_NONBLOCK) == 0);
}

static void expired(uev_t *timer)
{
	struct pollfd fd = { .fd = timer->fd, .events = POLLIN };

	fail_unless(poll(&fd, 1, 2000) == 1 && (fd.revents & POLLIN));
}

static void resume_timer(int period, int reset)
{
	struct result result = { .reset = reset };
	struct itimerspec remaining;
	uev_ctx_t ctx;
	uev_t timer;
	int i, count = period ? 3 : reset ? 2 : 1;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_timer_init(&ctx, &timer, cb, &result, 20, period) == 0);
	/* Dormant timers stay disarmed until the first run. */
	fail_unless(timerfd_gettime(timer.fd, &remaining) == 0);
	fail_unless(!remaining.it_value.tv_sec && !remaining.it_value.tv_nsec);
	run_once(&ctx);
	for (i = 0; i < count; i++) {
		expired(&timer);
		run_once(&ctx);
		fail_unless(result.calls == i + 1);
	}
	if (!period)
		fail_unless(!uev_timer_active(&timer) && timer.fd == -1);
	fail_unless(uev_exit(&ctx) == 0);
}

static void configured_while_running(void)
{
	struct result first = { 0 }, next = { 0 }, cron_result = { 0 };
	struct itimerspec remaining;
	uev_ctx_t ctx;
	uev_t timer, later, cron;
	time_t when = time(NULL) + 3600;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_timer_init(&ctx, &timer, cb, &first, 0, 0) == 0);
	fail_unless(uev_cron_init(&ctx, &cron, cb, &cron_result, when, 1) == 0);
	run_once(&ctx);
	fail_unless(!first.calls && !cron_result.calls);
	/* Existing disabled and newly registered timers arm immediately. */
	fail_unless(uev_timer_set(&timer, 20, 0) == 0);
	fail_unless(uev_timer_init(&ctx, &later, cb, &next, 20, 0) == 0);
	expired(&timer);
	expired(&later);
	run_once(&ctx);
	fail_unless(first.calls == 1 && next.calls == 1);
	fail_unless(cron_result.calls == 0);
	fail_unless(timerfd_gettime(cron.fd, &remaining) == 0);
	fail_unless(remaining.it_value.tv_sec > 0 && remaining.it_interval.tv_sec == 1);
	fail_unless(uev_exit(&ctx) == 0);
}

static void blocking_timer(void)
{
	struct result result = { .exit = 1 };
	uev_ctx_t ctx;
	uev_t timer;

	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_timer_init(&ctx, &timer, cb, &result, 20, 0) == 0);
	fail_unless(uev_run(&ctx, 0) == 0);
	fail_unless(result.calls == 1 && ctx.fd == -1);
	/* Reinitializing the same context starts a fresh timer lifetime. */
	result.calls = 0;
	fail_unless(uev_init(&ctx) == 0);
	fail_unless(uev_timer_init(&ctx, &timer, cb, &result, 20, 0) == 0);
	fail_unless(uev_run(&ctx, 0) == 0);
	fail_unless(result.calls == 1 && ctx.fd == -1);
}

int main(void)
{
	alarm(10);
	resume_timer(0, 0);
	resume_timer(20, 0);
	resume_timer(0, 1);
	configured_while_running();
	blocking_timer();
	alarm(0);
	return 0;
}
