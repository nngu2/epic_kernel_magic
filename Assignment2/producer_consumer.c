#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/timekeeping.h>
#include <linux/moduleparam.h>
#include <linux/sched/signal.h>

int buffSize = 0;
int prod = 0;
int cons = 0;
int uuid = 0;

module_param(buffSize, int, 0);
module_param(prod, int, 0);
module_param(cons, int, 0);
module_param(uuid, int, 0);



struct task_list_node 
{
    struct task_struct* task;
    struct task_list_node* next;
}

struct task_list_node* consumer_list;
struct task_struct *producer_thread;
int process_thread_id;

int consumer_counter = 0;

struct semaphore empty;
struct semaphore mutex;
struct semaphore full;

static int producer_thread(void* arg)
{
    while (!kthread_should_stop())
    {

    }
}

static int consumer_thread(void* arg)
{
    while (!kthread_should_stop())
    {}
}


static int __init module_init_function(void)
{
    sema_init(&empty, buffSize);
    sema_init(&mutex, 1);
    sema_init(&full, 0);

    if (prod == 1) producer_thread = kthread_run(producer_thread, NULL, )

    for (int i = 0; i < cons; i++)
    {
        struct task_list_node* cthread = 
    }
} 

static void __exit module_exit_function(void) 
{
    struct task_struct_list* list_head = consumer_thread;
    while (list_head != NULL)
    {
        for (int i = 0; i < cons)
        struct task_struct* head = list_head->task;
        for_each_process(head) { kthread_stop(head); }
        list_head = list_head->next;
    }

    int current_time_elapsed = time_
    int uuid = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    printk("The total elapsed time of all processes for UID %d is %02d:%02d:%02d", uuid, hours, minutes, seconds);

    list_head = list_head->next;
}

module_init(module_init_function);
module_exit(module_exit_function);
MODULE_LICENSE("GPL");