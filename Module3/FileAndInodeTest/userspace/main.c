#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

char buf[256];
int main(int argc, char *argv[])
{
    int fd;
    // "/dev/solution_node"
    fd = open(argv[1], O_RDWR);
    for(int i = 0; i < 20; ++i){
        ssize_t res = read(fd, buf, 64);
        printf ("READ FROM %s = %s\n", argv[1], buf);
        sleep(1);
    }
}
