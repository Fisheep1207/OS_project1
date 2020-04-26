#include <linux/linkage.h>
#include <linux/kernel.h>
// system call number = 334
asmlinkage void mySysPrintk(unsigned long start_sec,
                           unsigned long start_float,
                           unsigned long end_sec,
                           unsigned long end_float, int pid){
    printk( "project1: %d %lu.%09lu %lu.%09lu\n",
        pid, start_sec, start_float, end_sec, end_float);
    return ;
}