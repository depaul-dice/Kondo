#define _GNU_SOURCE

#include "repeatLibrary.h"


SysData *systemData;
int setup = 0;

FILE *fopen(const char *filename, const char *mode)
{
    if(setup == 0)
    {
        initSystem();
    }
    FILE* ret;
    if(checkTrackingPath(filename))
    {
        logOpen(filename, NULL, &ret, FOPEN);
        return ret;
    }
    return getSysData()->functions->real_fopen(filename, mode);
    return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret ;

    if(checkTrackingFptr(stream))
    {
        ret = logRead(-1, size*nmemb, stream, -1, FREAD, ptr);
    }
    else
    {
        ret = getSysData()->functions->real_fread(ptr, size, nmemb, stream);
    }
    return ret;
}