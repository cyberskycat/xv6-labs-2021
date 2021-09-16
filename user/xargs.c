#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{

    char buf[512];
    char read_char;


    char *process_name = argv[1];
    char **ext_argv = malloc((argc + 1) * sizeof *argv);

    for (int i = 1; i < argc; i++)
    {
        ext_argv[i - 1] = argv[i];
        // printf("argv %s  \n",ext_argv[i-1]);
    }
    ext_argv[argc - 1] = buf;
    ext_argv[argc] = 0;
    int status;

    for (;;)
    {
        int read_size = read(0, &read_char, 1);
        if (read_char == '\n')
        { //new line
            if (fork() == 0)
            {
                exec(process_name, ext_argv);
            }
            else
            {
                wait(&status);
                if (status != 0)
                {
                    printf("child exit wiht error \n");
                    exit(1);
                }
            }
            //clear buf prepare for new line
            memset(buf, 0, strlen(buf));
        }
        else
        {
            buf[strlen(buf)] = read_char;
        }
        if (read_size == 0)
        {
            exit(0);
        }
    }
    free(ext_argv);
    exit(0);
}
