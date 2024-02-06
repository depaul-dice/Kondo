#ifndef LOG_STRUCTURES_H
#define LOG_STRUCTURES_H
#include <sys/stat.h>
#define HASH_LEN 32
enum CallType
{
    FOPEN,
    FOPEN64,
    OPEN,
    OPEN64,
    OPENAT,

    FCLOSE,
    CLOSE,

    FREAD,
    PREAD,
    PREAD64,
    READ,
    PREADCHK,

    FSEEK,
    LSEEK,
    LSEEK64,

    FWRITE,
    WRITE,
    PWRITE,

    FSTAT,
    FSTAT64,
    LSTAT
};

typedef struct CallList
{
    /* data */
    // type of the call
    enum CallType type;

    // offset for the call
    off_t offset;

    // size of the call
    off_t size;

    // other depending on the call
    int other;

    // timestamp of the call
    off_t timeStamp;

    // hash for read and writes
    unsigned char* hash;
    // LL pointers
    struct CallList *pNext;
    struct CallList *pPrev;

} CallList;

// Struct for Black Lsit and White List entries
typedef struct pathList
{
    char path[1024];
    int fd;
    UT_hash_handle hh;
} pathList;

// Struct to hold the black List and white list
typedef struct specializationList
{
    pathList *whiteList;
    pathList *blackList;
} specializationList;

// Struct to hold real versions of all the functions
typedef struct realFunctions
{
    // Flavours of open
    FILE *(*real_fopen)(const char *filename, const char *mode);
    FILE *(*real_fopen64)(const char *filename, const char *mode);
    int (*real_open)(const char *, int);
    int (*real_open64)(const char *, int);

    // Flavours of close
    int (*real_fclose)(FILE *fptr);
    int (*real_close)(int fd);

    // flavours of read
    ssize_t (*real_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
    ssize_t (*real_pread)(int fd, void *buf, size_t count, off_t offset);
    ssize_t (*real_pread64)(int fd, void *buf, size_t count, off_t offset);
    ssize_t (*real_read)(int fd, void *buf, size_t count);
    ssize_t (*real___pread_chk)(int fd, void *buf, size_t nbytes, off_t offset, size_t buflen);
    // flavours of write
    size_t (*real_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);
    ssize_t (*real_write)(int fildes, const void *buf, size_t nbytes);
    ssize_t (*real_pwrite)(int fd, const void *buf, size_t count, off_t offset);

    // Flavours of seek
    int (*real_fseek)(FILE *stream, long int offset, int whence);
    off_t (*real_lseek)(int fildes, off_t offset, int whence);
    off_t (*real_lseek64)(int fildes, off_t offset, int whence);

    // Flavours of stat
    int (*real_fstat)(int fd, struct stat *buf);
    int (*real_fstat64)(int fd, struct stat *buf);
    int (*real_lstat)(const char *path, struct stat *buf);

} realFunctions;

// Struct to hold emtadata of all the open files
typedef struct openFile
{
    int fd;
    FILE *fptr;
    char path[PATH_MAX];
    UT_hash_handle fdHandle;
    UT_hash_handle fptrHandle;
    UT_hash_handle pathHandle;

} openFile;

// Struct  to hold different hash tables of the open file metadata
typedef struct openFileList
{
    openFile *filePtrs;
    openFile *fileDescs;
    openFile *filePaths;

} openFileList;

typedef struct fileMetadata
{
    // File Descriptor
    int fd;

    // File pointer
    FILE *fptr;

    // Absolute path
    char path[PATH_MAX];

    // Read and write Interval Trees
    Node *readTree;
    Node *writeTree;

    // File pointer of the file
    size_t filePointer;

    // status whether it is open or not
    int openStatus;

    // List of calls made to this file
    CallList *listOfCalls;

    // List of backups
    NodeList *backups;

    // Size of the original file
    size_t fileSize;

    // Handle to hash using absolute path
    UT_hash_handle pathHandle;

} fileMetadata;

typedef struct SysData
{
    openFileList *openFiles;
    specializationList *IOList;
    realFunctions *functions;
    fileMetadata *metaadata;

} SysData;

#endif
