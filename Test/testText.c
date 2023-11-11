#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main()
{
    int fd = open("/home/rohan/newDev/Kondo/Test/Data/textFile.txt", O_RDONLY);

    char* c = malloc(120); 
    memset(c, 0, 120);
    // Event 1
    pread(fd, c, 110, 0);
    fprintf(stdout, "%.110s\n",c);
    // Event 2
    pread(fd, c, 30, 70);
    fprintf(stdout, "%.30s\n",c);
    // Event 3
    pread(fd, c, 20, 130);
    fprintf(stdout, "%.20s\n",c);
    // Event 4
    memset(c, 0, 120);
    pwrite(fd, c, 20, 80);

    // Event 5
    pread(fd, c, 30, 90);
    fprintf(stdout, "%.30s\n",c);
    // Event 6
    memset(c, 0, 120);
    pwrite(fd, c, 60, 70);
    
    close(fd);
}
