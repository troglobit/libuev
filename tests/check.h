#ifndef UEV_TESTS_CHECK_H_
#define UEV_TESTS_CHECK_H_

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../uev.h"

#define UNUSED(arg) arg __attribute__ ((unused))

#define fail_unless(test)						\
  do {									\
    if (!(test)) {							\
      fprintf(stderr,							\
	      "----------------------------------------------\n"	\
	      "%s:%d: test FAILED:\nFailed test: %s\n"			\
	      "----------------------------------------------\n",	\
	      __FILE__, __LINE__, #test);				\
      exit(1);								\
    }									\
  } while (0)

static inline int test(int result, const char *fmt, ...)
{
	char buf[80];
	size_t len;
	va_list ap;
	const char success[] = " \033[1m[ OK ]\033[0m\n";
	const char failure[] = " \033[7m[FAIL]\033[0m\n";
	const char dots[] = " .....................................................................";

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	write(STDERR_FILENO, buf, len);
	write(STDERR_FILENO, dots, 60 - len); /* pad with dots. */

	if (!result)
		write(STDERR_FILENO, success, strlen(success));
	else
		write(STDERR_FILENO, failure, strlen(failure));

	return result;
}

static inline int timespec_newer(const struct timespec *a, const struct timespec *b)
{
	if (a->tv_sec != b->tv_sec)
		return a->tv_sec > b->tv_sec;

	return a->tv_nsec > b->tv_nsec;
}

static inline char *timespec2str(struct timespec *ts, char *buf, size_t len)
{
	size_t ret, pos;
	struct tm t;

	memset(buf, 0, len);

	tzset();
	if (localtime_r(&(ts->tv_sec), &t) == NULL)
		return buf;

	ret = strftime(buf, len, "%F %T", &t);
	if (ret == 0)
		return buf;
	len -= ret - 1;

	pos  = strlen(buf);
	len -= pos;
	snprintf(&buf[pos], len, ".%09ld", ts->tv_nsec);

	return buf;
}

#endif /* UEV_TESTS_CHECK_H_ */
