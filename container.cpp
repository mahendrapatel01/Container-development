#include<iostream>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define STACK_SIZE (1024 * 1024) 

int argcnt = 0;
char filename[30]="xyz",ip[30]="1234",hostname[30]="mahi";
int verbose=0;

static int childProcess(void* argv)
{
	char **c = (char**)argv;

	int fd;
	char cmd[50]="/run/netns/netns";
	strcat(cmd,filename);
	// cout<<"netns cmd : "<<cmd<<endl;
	fd = open(cmd, O_RDONLY| O_CLOEXEC);   /* Get descriptor for namespace */
	// cout<<"fd : "<<fd<<endl;
	if(fd!=-1)
	{
	 	setns(fd, CLONE_NEWNET);
		//  cout<<"setns !!"<<endl;
	}     
	if(verbose)
	printf("child PID: %ld\n", (long)getpid());
	if(verbose)
 	printf("Parent PID: %ld\n", (long)getppid());
	strcpy(cmd,"./images/");
	strcat(cmd,filename);
	strcat(cmd,"/rootfs");
	
	char s[100];
	chroot(cmd);

	// unshare(CLONE_NEWUTS);
	sethostname(hostname,strlen(hostname));
	if(verbose)
	printf("\ncwd : %s\n", getcwd(s, 100));
	mount("proc","/proc","proc",0,"");
	chdir("/");
	// unshare(CLONE_NEWNET);

	system("/bin/bash");
	return 0;
}

int main(int argc,char** argv)
{
	cout<<"argc : "<<argc<<endl;
	argcnt=argc;
	// int flags=0,options;
	char **c = (char**)argv;
	for(int i=0;i<argc;i++)
		cout<<"argv "<<i<<" : "<<argv[i]<<endl;
	// char filename[30];
	int options=0;
	int firsttime=0;
	char contianerID[3]="1";
	// char ip_addr[30]="127.0.0.1";
	while((options = getopt(argc,c,":f:nc:u:v"))!=-1)
	{
		switch(options)
		{
			case 'f':{
				if(verbose)
					cout<<"filesystem : "<<optarg<<endl;
				strcpy(filename,optarg);
				break;
			}
			case 'n':{
				if(verbose)
					cout<<"firsttime : "<<endl;
				firsttime=1;
				break;
			}

			case 'c':{
				if(verbose)
					cout<<"containerID : "<<optarg<<endl;
				strcpy(contianerID ,optarg);
				break;
			}
	
			case 'u':{
				if(verbose)
					cout<<"hostname : "<<optarg<<endl;
				strcpy(hostname,optarg);
				break;
			}

			case 'v':{
				cout<<"verbose !"<<endl;
				verbose=1;
				break;
			}
		}
	}
	
	if(verbose)
		cout<<"main process pid : "<<getpid()<<endl;
	
	// return 0;
	char *stack;                    /* Start of stack buffer */
  	char *stackTop; 
	stack = (char*)mmap(NULL , STACK_SIZE, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  	// if (stack == MAP_FAILED)
    // 	errExit("mmap");
	if(stack==NULL){
		cout<<"error in allocating stack !!!"<<endl;
		exit(0);
	}
  	stackTop = stack + STACK_SIZE; 
	int fd;
	// if it is first time then create and extract FS
	char cmd[100] = "./initdir.sh " ;

	if(firsttime)
	{
		strcat(cmd,filename);
		if(verbose)
		cout<<"cmd : "<<cmd<<endl;
		//create dir /images/filename/rootfs
		system(cmd);
	}
	//create new network namespace
	strcpy(cmd, "/run/netns/netns");
	strcat(cmd,filename);
	//read /run/netns/netns-filename
	if(verbose)
	cout<<"cmd : "<<cmd<<endl;
	fd = open(cmd, O_RDONLY| O_CLOEXEC);

	if(verbose)
	{
		cout<<"filename : "<<filename<<endl;
		cout<<"container : "<<contianerID<<endl;
		cout<<"hostname : "<<hostname<<endl;
	}
	if(fd==-1){
		char createns[100]="./createVeth.sh ";
		strcat(createns,filename);
		// strcat(createns," ");
		// strcat(createns,ip_addr);
		strcat(createns," ");
		strcat(createns,contianerID);
		if(verbose)
			cout<<"creating netns : "<<createns<<endl;
		system(createns);
		if(verbose)
			cout<<"ntw namespace created"<<endl;
	}
	else{
		cout<<"namespace is already created"<<endl;
	}
	close(fd);
	// system("mount -t cgroup -o memory memory /sys/fs/cgroup/memory");
	// system("mkdir /sys/fs/cgroup/memory/red");
	// system("cat /sys/fs/cgroup/memory/red/memory.limit_in_bytes");
	// system("echo 104857600 > /sys/fs/cgroup/memory/red/memory.limit_in_bytes");
	// system("cat /sys/fs/cgroup/memory/red/memory.oom_control");
	
  	pid_t child_pid = clone(childProcess, stackTop, CLONE_NEWNS  | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, argv);
	if(verbose)
	cout<<"entering cgroup!!!"<<endl;
	char cpc[10];
	sprintf(cpc," %ld ",(long)child_pid);
	if(verbose)
	printf("cpc : %s\n",cpc);
	char cmd2[100];
	strcpy(cmd2,"echo ");
	strcat(cmd2,cpc);
	strcat(cmd2,"| sudo tee /sys/fs/cgroup/memory/demo1/tasks");
	if(verbose)
	cout<<"cmd2 : "<<cmd2<<endl;
	system(cmd2);
	if(verbose)
  	printf("clone() = %ld\n", (long)child_pid);
	
  	waitpid(child_pid, NULL, 0);
	return 0;
}
