#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/timekeeping.h>
#include <linux/moduleparam.h>
#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/delay.h>

// #define THREAD_NUM 8

static int buff_size = 0;
static int p = 0;
static int c = 0;
static int uid = 0;

module_param(buff_size, int, 0);
module_param(p, int, 0);
module_param(c, int, 0);
module_param(uid, int, 0);

struct task_list_node
{
	struct task_struct *task;
	struct task_list_node *next;
	int process_num;
	int consumer_counter;
};

static struct task_list_node *consumer_list = NULL;
static struct task_list_node *consumer_list_tail = NULL;
static struct task_struct *producer_thread;
int process_thread_id;

int process_counter = 0;
int producer_counter = 0;
// int consumer_counter = 0;
int consumed_counter = 0;

int total_hours = 0;
int total_minutes = 0;
int total_seconds = 0;

int pfinished = 0;
int cfinished = 0;

int buffer_size = 0;
static struct task_list_node *buffer_head = NULL;

struct semaphore empty;
struct semaphore mutex;
struct semaphore full;

unsigned long time_elapsed = 0;
int i;
int j;

int kproducer_thread(void *arg)
{
	struct task_struct *pr;
	struct task_list_node *new_node;

	for_each_process(pr)
	{

		if (pr->cred->uid.val == uid)
		{
			process_counter++;
			if (down_interruptible(&empty))
			{
				break;
			}
			if (down_interruptible(&mutex))
			{
				break;
			}
			new_node = kmalloc(sizeof(struct task_list_node), GFP_KERNEL);
			new_node->task = pr;
			new_node->next = buffer_head;
			new_node->process_num = process_counter;
			buffer_head = new_node;
			// buffer_size++;
			producer_counter++;
			printk(KERN_INFO "[Producer - 1] Produced Item#-%d on buffer index:%d for PID:%d\n", process_counter, buffer_size, pr->pid);
			buffer_size++;
			up(&mutex);
			up(&full);
		}

		if (kthread_should_stop())
			return 0;
	}

	pfinished = 1;
	return 0;
}

int kconsumer_thread(void *arg)
{
	struct task_list_node *current_task;
	unsigned long consumer_time_elapsed;
	int seconds;
	int hours;
	int minutes;
	while (!kthread_should_stop())
	{
		if (down_interruptible(&full))
			break;
		if (down_interruptible(&mutex))
			break;
		if (buffer_size == 0)
			continue;

		current_task = buffer_head;
		buffer_head = buffer_head->next;
		buffer_size--;

		consumer_time_elapsed = ktime_get_ns() - current_task->task->start_time;
		time_elapsed += consumer_time_elapsed;
		seconds = consumer_time_elapsed / 1000000000;
		hours = (seconds / 3600) % 24;
		minutes = (seconds / 60) % 60;
		seconds = seconds % 60;
		consumed_counter++;
		printk(KERN_INFO "[Consumer - %d] Consumed Item#-%d on buffer index:%d PID:%d Elapsed Time- %d:%d:%d\n", consumer_list->consumer_counter, current_task->process_num, buffer_size, current_task->task->pid, hours, minutes, seconds);

		up(&mutex);
		up(&empty);
	}

	cfinished = 1;
	return 0;
}

int name_init(void)
{
	char name[50];
	printk("Kernel Module recieved the following inputs: UID:%d BUFF_SIZE:%d PROD:%d CONS:%d\n", uid, buff_size, p, c);
	sema_init(&empty, buff_size - 1);
	sema_init(&mutex, 1);
	sema_init(&full, 0);
	if (p == 1)
	{
		sprintf(name, "Producer_thread-1");
		producer_thread = kthread_create(kproducer_thread, NULL, name);
		if (producer_thread)
		{
			printk("[Producer - 1] created\n");
			wake_up_process(producer_thread);
		}
	}

	for (i = 0; i < c; i++)
	{
		struct task_list_node *current_thread = kmalloc(sizeof(struct task_list_node), GFP_KERNEL);
		sprintf(name, "Consumer_Thread-%d", i + 1);
		current_thread->task = kthread_run(kconsumer_thread, NULL, name);

		current_thread->next = consumer_list;
		consumer_list = current_thread;
		consumer_list->consumer_counter = i + 1;
	}
	return 0;
}

void thread_exit(void)
{
	time_elapsed = time_elapsed / 1000000000;
	total_hours = (time_elapsed / 3600) % 24;
	total_minutes = (time_elapsed / 60) % 60;
	total_seconds = time_elapsed % 60;

	if (c == 1)
	{
		for (i = 0; i < c; i++)
		{
			for (j = c; i > 0; i--)
			{
				up(&empty);
				up(&mutex);
				up(&full);
			}
			if (cfinished == 0)
			{
				kthread_stop(consumer_list->task);
			}
			printk("[Consumer - %d] stopped\n", consumer_list->consumer_counter);
			consumer_list_tail = consumer_list;
			consumer_list = consumer_list->next;
			kfree(consumer_list_tail);
			cfinished = 0;
		}
	}

	if (p == 1)
	{
		up(&mutex);
		up(&empty);
		up(&full);
		if (pfinished == 0)
		{
			kthread_stop(producer_thread);
		}
		printk("[Producer - 1] stopped\n");
		if (buffer_head != NULL)
		{
			while (buffer_head->next != NULL)
			{
				consumer_list = buffer_head;
				buffer_head = buffer_head->next;
				kfree(consumer_list);
			}
			kfree(buffer_head);
		}
	}
	printk("Total number of items Produced: %d\n", producer_counter);
	printk("Total number of items Consumed: %d\n", consumed_counter);
	printk(KERN_INFO "The total elapsed time of all processes for UID %d is %d:%d:%d\n", uid, total_hours, total_minutes, total_seconds);
}

module_init(name_init);
module_exit(thread_exit);
MODULE_LICENSE("GPL");