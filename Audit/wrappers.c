#define _GNU_SOURCE

#include "logLibrary.h"


SysData *systemData;
int setup = 0;

FILE *fopen(const char *filename, const char *mode)
{
    if(setup == 0)
    {
        initSystem();
    }
    
    FILE* ret = getSysData()->functions->real_fopen(filename, mode);
    if(checkTrackingPath(filename))
    {
        logOpen(filename, -1, ret, FOPEN);
    }
    return ret;
}
int open(const char *filename, int flags)
{
    if(setup == 0)
    {
        initSystem();
    }
    
    int ret = getSysData()->functions->real_open(filename, flags);
    if(checkTrackingPath(filename))
    {
        logOpen(filename, ret, NULL, OPEN);
    }
    return ret; 
}

int open64(const char *filename, int flags)
{
    if(setup == 0)
    {
        initSystem();
    }
    
    int ret = getSysData()->functions->real_open64(filename, flags);
    if(checkTrackingPath(filename))
    {
        logOpen(filename, ret, NULL, OPEN64);
    }
    return ret; 
}

int fclose(FILE *stream)
{

    if(checkTrackingFptr(stream))
    {
        logClose(-1, stream, FCLOSE);
    }
    return getSysData()->functions->real_fclose(stream);
}

int close(int fd)
{
    if(checkTrackingDesc(fd))
    {
        logClose(fd, NULL, CLOSE);
    }
    return getSysData()->functions->real_close(fd);
}


size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret = getSysData()->functions->real_fread(ptr, size, nmemb, stream);

    if(checkTrackingFptr(stream))
    {
        logRead(-1, size*nmemb, stream, -1, FREAD);
    }
    return ret;
}
ssize_t __pread_chk(int fd, void *buf, size_t nbytes, off_t offset, size_t buflen)
{
    size_t ret = getSysData()->functions->real___pread_chk(fd, buf, nbytes, offset, buflen);

    if(checkTrackingDesc(fd))
    {
        logRead(offset, nbytes, NULL, fd, PREADCHK);
    }
    return ret;
}
ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    size_t ret = getSysData()->functions->real_pread(fd, buf, count, offset);

    if(checkTrackingDesc(fd))
    {
        logRead(offset, count, NULL, fd, PREAD);
    }
    return ret;
}

ssize_t pread64(int fd, void *buf, size_t count, off_t offset)
{
    size_t ret = getSysData()->functions->real_pread64(fd, buf, count, offset);

    if(checkTrackingDesc(fd))
    {
        logRead(offset, count, NULL, fd, PREAD64);
    }
    return ret;
}

ssize_t read(int fd, void *buf, size_t count)
{
    size_t ret = getSysData()->functions->real_read(fd, buf, count);

    if(checkTrackingDesc(fd))
    {
        logRead(-1, count, NULL, fd, READ);
    }
    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if(checkTrackingFptr(stream))
    {
        logWrite(-1, size*nmemb, stream, -1, FWRITE);    
    }
    return getSysData()->functions->real_fwrite(ptr, size, nmemb, stream);   
}

ssize_t write(int fildes, const void *buf, size_t nbytes)
{
    if(checkTrackingDesc(fildes))
    {
        logWrite(-1, nbytes, NULL, fildes, WRITE);    
    }
    return getSysData()->functions->real_write(fildes, buf, nbytes);   
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    if(checkTrackingDesc(fd))
    {
        logWrite(offset, count, NULL, fd, PWRITE);    
    }
    return getSysData()->functions->real_pwrite(fd, buf, count, offset);     
}


int fseek(FILE *stream, long int offset, int whence)
{
    if(checkTrackingFptr(stream))
    {
        logSeek(offset, stream, -1, whence, FSEEK);    
    }
    return getSysData()->functions->real_fseek(stream, offset, whence);

}

off_t lseek(int fildes, off_t offset, int whence)
{
    if(checkTrackingDesc(fildes))
    {
        logSeek(offset, NULL, fildes, whence, LSEEK);    
    }
    return getSysData()->functions->real_lseek(fildes, offset, whence);
}

off_t lseek64(int fildes, off_t offset, int whence)
{
    if(checkTrackingDesc(fildes))
    {
        logSeek(offset, NULL, fildes, whence, LSEEK64);    
    }
    return getSysData()->functions->real_lseek64(fildes, offset, whence);
}