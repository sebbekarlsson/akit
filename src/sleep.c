#include <akit/sleep.h>
#include <date/date.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

int akit_msleep(long msec) {
  struct timespec ts;
  int res;

  if (msec < 0) {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}

void __attribute__((optimize("O0"))) akit_delay(double d) {
  Date start = date_now();

  while (1) {
    Date now = date_now();

    Date diff = date_diff(&now, &start);

    // printf("%12.6f\n", diff.milliseconds_static);
    if (diff.milliseconds_static >= d)
      return;
  }
}
