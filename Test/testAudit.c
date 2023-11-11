#include <stdio.h>
#include <string.h>

int main()
{
    FILE* fptr = fopen("trial.txt", "r+");
    
    char bufRead[200] = {0};
    size_t ret =  0;
    ret = fread(bufRead, 110,1,fptr);
    fprintf(stdout, "The read data is %s\n",bufRead);
    fseek(fptr, 100, SEEK_SET);
    char write[11];
    strcpy(write, "aaaaaaaaaa");
    ret = fwrite(write, 1, sizeof(write), fptr);
    fseek(fptr, 100, SEEK_SET);
    char bufRead2[10];
    ret = fread(bufRead2, 10, 1, fptr);
    fprintf(stdout, "The read data is %s\n",bufRead2);
    fclose(fptr);
}
