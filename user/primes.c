#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int num_size = 36;
    int *num_array = (int *)malloc(num_size * 4);
    //TODO
    // memset(num_array,num_size,10);
    for (int i = 0; i < num_size; i++)
    {
        num_array[i] = i;
    }

    int parent_pipe[2];
    pipe(parent_pipe);
    write(parent_pipe[1], num_array + 2, 4 * (num_size - 2));
rrr:
    close(parent_pipe[1]);
    int num1;
    num1 = 0;
    int rsize = read(parent_pipe[0], &num1, 4);
    if (rsize == 0)
    {
        wait(0);
        exit(0);
    }
    printf("prime %d \n", num1);
    int child_pipe[2];
    pipe(child_pipe);
    if (fork() == 0)
    {
        parent_pipe[0] = child_pipe[0];
        parent_pipe[1] = child_pipe[1];
        goto rrr;
    }
    else
    { //left parent process
        int num2;
        for (;;)
        {
            int rsize = read(parent_pipe[0], &num2, 4);
            if (rsize == 0)
            {
                break;
            }
            if (num2 % num1 != 0)
            {
                write(child_pipe[1], &num2, 4);
            }
        }
        close(parent_pipe[0]);
        close(child_pipe[1]);
        // printf("wait father  %d\n", getpid());
        wait(0);
        // printf("exit father %d\n", getpid());
        // printf("pid =%d \n",getpid());
        exit(0);
    }
    if (num_array != 0)
    {
        free(num_array);
        num_array = 0;
    }
    // printf("ppppid =%d \n", getpid());
    exit(0);
}