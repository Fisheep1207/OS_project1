#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/linkage.h>
// system call number = 333
asmlinkage int mySysTime(unsigned long *s, unsigned long *f) {
    struct timespec t;
    getnstimeofday(&t);
    *s = t.tv_sec;
    *f = t.tv_nsec;
    return 0;
}