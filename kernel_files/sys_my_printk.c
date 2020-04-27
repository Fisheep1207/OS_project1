#include <linux/linkage.h>
#include <linux/kernel.h>
// system call number = 334
asmlinkage void mySysPrintk(unsigned long __user start_sec,
                           unsigned long __user start_float,
                           unsigned long __user end_sec,
                           unsigned long __user end_float, int __user pid){
    printk( "[Project1] %d %lu.%09lu %lu.%09lu\n",
        pid, start_sec, start_float, end_sec, end_float);
    return ;
}