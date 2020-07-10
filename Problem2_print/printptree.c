/*

This code is the source code of print the whole process tree.

File name: printptree.c

Purpose: Use system call defined before to store information in *buf and *nr. Print the information and it print the blank to show the relationship between parent and child .

*/
#include <stdlib.h>
#include <string.h>
#include "stdio.h"


struct prinfo {
	pid_t parent_pid; /* process id of parent */
	pid_t pid; /* process id */
	pid_t first_child_pid; /* pid of youngest child */
	pid_t next_sibling_pid; /* pid of older sibling */
	long state; /* current state of process */
	long uid; /* user id of process owner */
	char comm[64]; /* name of program executed */
};

void display(struct prinfo* buf, int* nr)
{
	int stack[300];
	int blank[300];//store the depth of the prinfo.
	memset(blank, 0, 300 * sizeof(int));

    int i, j, top = 0;

 
    for (i = 0; i < *nr; ++i)
    {
    	if (i == *nr-1 || (buf[i].next_sibling_pid != buf[i+1].pid && buf[i].first_child_pid != buf[i+1].pid))//with no child or the end 
    		buf[i].next_sibling_pid = 0;
    	if (top == 0 || buf[i].pid == buf[stack[top-1]].first_child_pid)//add to stack
    		stack[top++] = i;
    	else
    	{
    		while (buf[i].pid != buf[stack[top-1]].next_sibling_pid)//with no sibling
    		{
    			--top;
    	        buf[stack[top]].next_sibling_pid = 0;
    	    	}
            stack[top-1] = i;
        }
        blank[i] = top-1;
    }

    // print
    for (i = 1; i < *nr; ++i)//for no need to print swapper, it begins from 1.
    {
    	for (j = 1; j < blank[i]; ++j)//print blank 
		 printf("\t");
    	printf("%s,%d,%ld,%d,%d,%d,%ld\n", buf[i].comm, buf[i].pid, buf[i].state, buf[i].parent_pid, buf[i].first_child_pid, buf[i].next_sibling_pid, buf[i].uid);//print info
    }
}
int main(int argc, char *argv[])
{
    struct prinfo *buf = (struct prinfo*)malloc(sizeof(struct prinfo) * 3000);
    int nr;
    syscall(356, buf, &nr); // call ptree
    display(buf, &nr);
    free(buf);
    return 0;
}
