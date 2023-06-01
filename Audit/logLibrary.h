#ifndef LOG_LIBRARY_H
#define LOG_LIBRARY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../Utilities/logUtil/logUtilities.h"
extern int setup;
extern SysData* systemData;

/// @brief Initialise the system to start tracking 
void initSystem();

/// @brief Given the name of a file return if we are tracking the file or not
/// @param filename name of file to check tracking for
/// @return 1 if tracking 0 if not
int checkTrackingPath(const char *path);

/// @brief Given the pointer of a file return if we are tracking the file or not
/// @param fptr pointer of file to check tracking for
/// @return 1 if tracking 0 if not
int checkTrackingFptr(FILE *fptr);

/// @brief Given the desc of a file return if we are tracking the file or not
/// @param fd desc of file to check tracking for
/// @return 1 if tracking 0 if not
int checkTrackingDesc(int fd);


/// @brief Add allm thje apt structures for the given file
/// @param buf Path to the file
/// @param fd FD of the file or -1
/// @param fptr File pointer fo the file or NULL
/// @param type an Enumeration of what type of open call was made
void logOpen(const char *buf, int fd, FILE *fptr, enum CallType tpye);

/// @brief Close the file structures aptly and flush all metadata, provide 1 of fd or fptr
/// @param fd fd if the file was opened using Fd else -1
/// @param fptr fptr if it ws using pointer else NULL
/// @param type an Enumeration of what type of open call was made
void logClose(int fd, FILE *fptr, enum CallType tpye);

/// @brief Log a read for the file pointed to by fptr. If offset is -1 then file pointer is not changed, else it is
/// @param offset -1 if we want to use the file pointer or a given offset if call is like pread where file pointer is not imapcted
/// @param readSize Size to read
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param type an Enumeration of what type of open call was made
/// @param ptr Pointer to buffer with data that has been read
void logRead(off_t offset, size_t readSize, FILE *fptr, int fd, enum CallType tpye, void* ptr);

/// @brief given the offset at which write is happening, or -1 if the write is from the current file pointer
/// log the write call and perform the needed backups
/// @param offset offset for the write or -1 if using the current file pointer
/// @param wrtteSize size fo the write
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param type an Enumeration of what type of open call was made
/// @param ptr Pointer of buffer holding data to write
void logWrite(off_t offset, size_t wrtteSize, FILE *fptr, int fd, enum CallType tpye, const void* ptr);


/// @brief Log the seek call and change metadata accordingly
/// @param off offset for the seek call
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param whence mode for the seek SET/CUR/END
/// @param type an Enumeration of what type of open call was made
void logSeek(long off, FILE *fptr, int fd, int whence, enum CallType tpye);

/// @brief Log a stat call for given fd
/// @param path Path of file we are
/// @param fptr File Pointer of file to log call for
/// @param fd FD of file to log call for
/// @param type Type of call we are logging
void logStat(const char* path, FILE* fptr, int fd, enum CallType type);

#endif
