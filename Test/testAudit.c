#include <stdio.h>
#include <string.h>

int main()
{
    FILE* fptr = fopen("trial.txt", "r+");
    char bufWrite[200]= {'A'};
    char bufRead[200] = {0};
    size_t ret =  0;
    ret = fread(bufRead, 110,1,fptr);
    fseek(fptr, 80, SEEK_SET);
    ret = fwrite(bufWrite, 1, 200, fptr);
    fclose(fptr);
}
