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

int fseek(FILE *stream, long int offset, int whence)
{
    if(checkTrackingFptr(stream))
    {
        logSeek(offset, stream, -1, whence, FSEEK);
        return 0;    
    }
    else
    {
        return getSysData()->functions->real_fseek(stream, offset, whence);
    }

}

off_t lseek(int fildes, off_t offset, int whence)
{
    if(checkTrackingDesc(fildes))
    {
        logSeek(offset, NULL, fildes, whence, LSEEK);    
        return 0;
    }
    else
    {
        return getSysData()->functions->real_lseek(fildes, offset, whence);
    }
}

off_t lseek64(int fildes, off_t offset, int whence)
{
    if(checkTrackingDesc(fildes))
    {
        logSeek(offset, NULL, fildes, whence, LSEEK64);    
        return 0;
    }
    else
    {
        return getSysData()->functions->real_lseek64(fildes, offset, whence);
    }
}