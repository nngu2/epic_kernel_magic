#include <stdio.h>
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

int process_counter = 0;
int consumer_counter = 0;

int buffer_size;
struct task_list_node* buffer_head;

struct semaphore empty;
struct semaphore mutex;
struct semaphore full;

static int producer_thread(void* arg)
{
    while (!kthread_should_stop())
    {
        buffer_head = kmalloc(sizeof(struct task_list_node), GFP_KERNEL);
        struct task_list_node* buffer_tail = buffer_head;
        struct task_struct* p;
        for_each_process(p)
        {
            struct task_list_node* new_node = kmalloc(sizeof(struct task_list_node), GFP_KERNEL);
            new_node->task = p;
            buffer_tail->next = new_node;
            buffer_tail = buffer_tail->next;
            buffer_size++;
        }
    }
    buffer_head = buffer_head->next;
    return 0;
}

static int consumer_thread(void* arg)
{
    while (!kthread_should_stop())
    {
        if (down_interruptible(&full)) break;
        if(down_interruptible(&mutex)) break;

        // FIXME : WTF DO I DO NEXT?!?!?!?
        if (buffer_head != NULL && buffer_size > 0)
        {
            struct task_list_node* current_task = buffer_head;
            buffer_head = buffer_head->next;

            unsigned long consumer_time_elapsed = ktime_get_ns() - current_task->start_time;
            int seconds = consumer_time_elapsed / 1000000000;
            int hours = (consumer_time_elapsed / 3600) % 24;
            int minutes = (consumer_time_elapsed / 60) % 6;
            seconds = seconds % 60;

            process_counter++;
            buffer_size--;

            printk(KERN_INFO "[Consumer - %d] Consumed Item#-%d on buffer index:%d PID:%d Elapsed Time- %d:%d:%d\n", consumer_counter, process_counter, buffer_size, current_task->task->pid, hours, minutes, seconds);
        }

        up(&mutex);
        up(&empty);
    }

    return 0;
}

static int __init module_init_function(void)
{
    sema_init(&empty, buffSize);
    sema_init(&mutex, 1);
    sema_init(&full, 0);

    if (prod == 1) 
    {
        producer_thread = kthread_run(producer_thread, NULL, "Producer_Thread-1");
        process_thread_id = producer_thread->pid;
    }

    for (int i = 0; i < cons; i++)
    {
        struct task_list_node* current_thread = kmalloc(sizeof(struct task_list_node), GFP_KERNEL); 
        char name[50];
        sprintf(name, "Consumer_Thread-%d", i);
        current_thread->task = kthread_run(consumer_thread, NULL, name);

        current_thread->next = consumer_list;
        consumer_list = current_thread;
        consumer_counter++;
    }
} 

static void __exit module_exit_function(void) 
{
    struct task_struct_list* list_tail = consumer_thread;
    while (list_tail != NULL)
    {
        for (int i = 0; i < cons; i++)
        {
            up(&empty);
            up(&mutex);
            up(&full);
        }

        kthread_stop(list_tail->task);
        list_tail = list_tail->next;
    }

    int current_time_elapsed = time_elapsed / 1000000000;
    int hours = (current_time_elapsed / 3600) % 24;
    int minutes = (current_time_elapsed / 60) % 6;
    int seconds = current_time_elapsed % 60;
    printk("The total elapsed time of all processes for UID %d is %02d:%02d:%02d", uuid, hours, minutes, seconds);
}

module_init(module_init_function);
module_exit(module_exit_function);
MODULE_LICENSE("GPL");