#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

int main()
{
    for (int i = 0; i < 5; i++)
    {
        pid_t p = fork();
        if (p != 0)
        {
            wait(NULL);
            printf("iter %d, %d from %d\n", i, getpid(), getppid());
            fflush(stdout);
            break;
        }
    }
    exit(0);
}
