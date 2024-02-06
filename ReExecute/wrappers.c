#define _GNU_SOURCE

#include "repeatLibrary.h"


SysData *systemData;
int setup = 0;
// int callNum = 0;
FILE *fopen(const char *filename, const char *mode)
{
    if(setup == 0)
    {
        initSystem();
    }
    FILE* ret;
    if(checkTrackingPath(filename))
    {
        logOpen(filename, -1, &ret, FOPEN);
        return ret;
    }
    return getSysData()->functions->real_fopen(filename, mode);
    
}

FILE *fopen64(const char *filename, const char *mode)
{
    if(setup == 0)
    {
        initSystem();
    }
    
    FILE* ret = getSysData()->functions->real_fopen64(filename, mode);
    if(checkTrackingPath(filename))
    {
        logOpen(filename, -1, &ret, FOPEN64);
    }
    return ret; 
}

int openat(int dirfd, const char *filename, int flags)
{
    if(setup == 0)
    {
        initSystem();
    }
    int fd;
    if(checkTrackingPath(filename))
    {
        return logOpen(filename, fd, NULL, OPENAT);
    }
    return getSysData()->functions->real_open(filename, flags);
    
}
int open(const char *filename, int flags)
{
     if(setup == 0)
    {
        initSystem();
    }
    int fd;
    if(checkTrackingPath(filename))
    {
        return logOpen(filename, fd, NULL, OPEN);
        
    }
    return getSysData()->functions->real_open(filename, flags);
    
}

int open64(const char *filename, int flags)
{
     if(setup == 0)
    {
        initSystem();
    }
    int fd;
    if(checkTrackingPath(filename))
    {
        return logOpen(filename, fd, NULL, OPEN64);
    }
    return getSysData()->functions->real_open64(filename, flags);
    
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret ;

    if(checkTrackingFptr(stream))
    {
        ret = logRead(-1, size*nmemb, stream, -1, FREAD, ptr);
        return nmemb;
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
        // char p[PATH_MAX] = {0};
        // strcpy(p, "call");
        // char num = '0'+callNum++;
        // strcat(p,&num);
        // strcat(p,".log");
        // FILE* fptr = getSysData()->functions->real_fopen(p, "w+");
        // getSysData()->functions->real_fwrite(buf, nbytes,1, fptr);
        // getSysData()->functions->real_fclose(fptr);
        return nbytes;
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
        return count;
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
        return count;
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
        return count;
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
        return logSeek(offset, NULL, fildes, whence, LSEEK64);    
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


size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if(checkTrackingFptr(stream))
    {
        logWrite(-1, size*nmemb, stream, -1, FWRITE, ptr);    
        return nmemb;
    }
    else
    {
        return getSysData()->functions->real_fwrite(ptr, size, nmemb, stream);   
    }
}

int fstat(int fd, struct stat *buf)
{
    if(checkTrackingDesc(fd))
    {
        logStat(NULL,fd, FSTAT, (void*) buf);    
        return 0;
    }
    return getSysData()->functions->real_fstat(fd, buf);
}

int fstat64(int fd, struct stat64 *buf)
{
    if(checkTrackingDesc(fd))
    {
        logStat(NULL, fd, FSTAT64, (void*) buf);    
        return 0;
    }
    return getSysData()->functions->real_fstat64(fd, (struct stat*)buf);
}

int lstat(const char *path, struct stat *buf)
{
    
    if(checkTrackingPath(path))
    {
        logLstat(path, buf);
        return 0;
    }
    return getSysData()->functions->real_lstat(path, buf);
}