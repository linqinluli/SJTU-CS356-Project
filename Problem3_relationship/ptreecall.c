/*
This is the source code of problem3.

File name: ptreecall.c

Purpose: use fork() to form the child process, and it use execl() to execuate ptree in child process. show the relationship between above two process
*/
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
int main()
{
	pid_t parentpid = getpid();
	pid_t childpid = fork();
	if (childpid > 0)//parent
	{
	    printf("518030910022 Parent PID = %d\n", parentpid); // print parent process ID
	    wait(NULL); 
	}
	else if (childpid == 0) //child 
	{
		childpid=getpid();
		printf("518030910022 Child PID = %d\n", childpid);// print child process ID
		printf("In child process, ptree:\n");
		execl("./ptree", "ptree", NULL); // execuate ptree file
	}
	else//error
	{	
		printf("Fork Failed");
		return 1;
	}
	return 0;
}
