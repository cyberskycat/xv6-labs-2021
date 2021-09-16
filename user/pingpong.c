#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int pipe_parent[2];
    int pipe_child[2];
    pipe(pipe_parent);
    pipe(pipe_child);
    if (fork() == 0)
    {
        char buf[1];
        close(pipe_parent[0]);
        close(pipe_child[1]);
        int size = read(pipe_child[0], buf, 1);
        if (size > 0)
        {
            printf("%d: received ping\n", getpid());
        }
        close(pipe_child[0]);
        write(pipe_parent[1], "n", 1);
        close(pipe_parent[1]);
        exit(0);
    }
    else
    {
        char buf[1]; 
        close(pipe_child[0]);
        close(pipe_parent[1]);
        write(pipe_child[1], "n", 1);
        close(pipe_child[1]);
        int size = read(pipe_parent[0], buf, 1);
        if (size > 0)
            printf("%d: received pong\n", getpid());
        close(pipe_parent[0]);
        wait(0);
    }

    exit(0);
}