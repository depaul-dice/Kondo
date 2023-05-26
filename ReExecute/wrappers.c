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

ssize_t __pread_chk(int fd, void *buf, size_t nbytes, off_t offset, size_t buflen)
{
    size_t ret; 
    if(checkTrackingDesc(fd))
    {
        ret = logRead(offset, nbytes, NULL, fd, PREADCHK, buf);
    }
    else
    {
        ret = getSysData()->functions->real___pread_chk(fd, buf, nbytes, offset, buflen);
    }
    return ret;
}
ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    size_t ret; 
    if(checkTrackingDesc(fd))
    {
        ret = logRead(offset, count, NULL, fd, PREAD, buf);
    }
    else
    {
        ret = getSysData()->functions->real_pread(fd, buf, count, offset);
    }
    return ret;
}

ssize_t pread64(int fd, void *buf, size_t count, off_t offset)
{
    size_t ret; 
    if(checkTrackingDesc(fd))
    {
        ret = logRead(offset, count, NULL, fd, PREAD64, buf);
    }
    else
    {
        ret = getSysData()->functions->real_pread64(fd, buf, count, offset);
    }
    return ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
    size_t ret; 
    if(checkTrackingDesc(fd))
    {
        ret = logRead(-1, count, NULL, fd, READ, buf);
    }
    else
    {
        ret = getSysData()->functions->real_read(fd, buf, count);
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

int fclose(FILE *stream)
{

    if(checkTrackingFptr(stream))
    {
        logClose(-1, stream, FCLOSE);
        return 0;
    }
    else
    {
        return getSysData()->functions->real_fclose(stream);
    }
}

int close(int fd)
{
    if(checkTrackingDesc(fd))
    {
        logClose(fd, NULL, CLOSE);
        return 0;
    }
    else
    {
        return getSysData()->functions->real_close(fd);
    }
}