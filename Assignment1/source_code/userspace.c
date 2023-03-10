#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __NR_my_syscall 450

int main() {
    long int shoutout = syscall(__NR_my_syscall);
    printf("The new system call returned: %ld\n", shoutout);
    return 0;
}