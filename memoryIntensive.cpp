#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

int main(int argc, char *argv[])
{
	char *p;

    int i=1;
	while ((p = (char *)malloc(10*MB)))
	{
        memset(p, 0, 10*MB);
        printf("memory used %d MB\n",i*10);
        i++;

        if(i>50)
            break;
    }	

	// while ((p = (char *)malloc(KB)))
	// 	memset(p, 0,
	// 			KB);

	sleep(1);

	return 0;
}