/*
This program is the source file of the moudle.

Filename: hello.c

Purpose: Add a new system call to the module. And it will keep the basic information in *buf of all process with calling the system call by dfs sequence.

	It consists of struct prinfo (contains the information);
		       void task_to_prinfo (gain information from task_struct to prinfo);
		       void dfs (store all informasion of process in the array buf[]);
		       int ptree (system callfunction)
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

MODULE_LICENSE("Dual BSD/GPL");
#define __NR_ptree 356

static int (*oldcall)(void);

struct prinfo//informasion of prcess
{
    pid_t parent_pid; /* process id of parent */
    pid_t pid; /* process id */
    pid_t first_child_pid; /* pid of youngest child */
    pid_t next_sibling_pid; /* pid of older sibling */
    long state; /* current state of process */
    long uid; /* user id of process owner */
    char comm[64]; /* name of program executed */
};
void task_to_prinfo(struct task_struct* task, struct prinfo* prin)
{
    if(task->parent == NULL)//parent_pid
	prin->parent_pid = 0;
    else prin->parent_pid = task->parent->pid;

    prin->pid = task->pid;//pid

    if(list_empty(&(task->children)) == true)//first_child_pid
        prin->first_child_pid = 0;
    else
        prin->first_child_pid = list_entry(task->children.next, struct task_struct, sibling)->pid;

    if(list_empty(&(task->sibling)) == true)//next_sibling_pid
        prin->next_sibling_pid = 0;
    else
        prin->next_sibling_pid = list_entry(task->sibling.next, struct task_struct, sibling)->pid;

    prin->state = task->state;//state

    prin->uid = task->cred->uid;//uid

    strcpy(prin->comm, task->comm);//name
}

void dfs(struct task_struct* task, struct prinfo* prin, int* nr)
{
    struct list_head *p;
    struct task_struct *q;
    task_to_prinfo(task, &prin[*nr]);
    ++(*nr);
    list_for_each(p, &(task->children))//traverse all children, and it can end up the recursive as well
    {
        q = list_entry(p, struct task_struct, sibling);
        dfs(q, prin, nr);//recursive
    }
}

int ptree(struct prinfo *buf,int *nr)
{
    struct prinfo * _buf = (struct prinfo *)kmalloc(sizeof(struct prinfo) * 3000, GFP_KERNEL);
    int _nr = 0;

    read_lock(&tasklist_lock);

    dfs(&init_task, _buf, &_nr);

    read_unlock(&tasklist_lock);

    copy_to_user(buf, _buf, sizeof(struct prinfo) * 3000);//read data from kernel to users. it make the data stored in *buf and *nr can be read.
    
    copy_to_user(nr, &_nr, sizeof(int));
    
    kfree(_buf);
    return 0;


}
static int addsyscall_init(void)
{
    long *syscall = (long*) 0xc000d8c4;
    oldcall=(int(*)(void))(syscall[__NR_ptree]);
    syscall[__NR_ptree] = (unsigned long ) ptree;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall= (long*)0xc000d8c4;
    syscall[__NR_ptree] = (unsigned long )oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
