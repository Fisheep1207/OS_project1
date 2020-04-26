#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/linkage.h>
// system call number = 333
asmlinkage int mySysTime(unsigned long __user *s, unsigned long __user *f) {
    struct timespec t;
    getnstimeofday(&t);
    copy_to_user(s, &t.tv_sec, sizeof(long));
    copy_to_user(f, &t.tv_nsec, sizeof(long));
    return 0;
}