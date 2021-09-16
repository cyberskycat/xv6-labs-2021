#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void find(char *path, char *find_name)
{
    int fd;
    struct stat st;
    char buf[512], *p;
    struct dirent de;
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if (st.type == T_FILE)
    {
        fprintf(2, "find: is a file %s\n", path);
        close(fd);
        return;
    }
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
        printf("find: path too long\n");
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0)
        {
            printf("ls: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_DIR)
        {
            // printf("rescurse path =% s ", buf);
            find(buf, find_name);
        }
        // printf("%s:   %s  %d %d %d\n", de.name,name,strcmp(fmtname(buf), name),strlen(fmtname(buf)),strlen(name));
        if (strcmp(de.name, find_name) == 0)
            printf("%s\n", buf);
    }
    close(fd);
    // printf("%s  eixt \n", path);
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        printf("Usage: find path flename\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
