#include <linux/module.h>
#include <linux/kernel.h>

int myname_init(void)
{
    printk("[Group-33][Abhinav Koyyalamudi, Jesse Jing, Noel Ngu, Thomas Tung] Hello, I am Noel Ngu, a student of CSE330 Spring 2022\n");
    return 0;
}

void myname_exit(void)
{
    printk("Goodbye-Noel Ngu\n");
}

module_init(myname_init);
module_exit(myname_exit);
MODULE_LICENSE("GPL");