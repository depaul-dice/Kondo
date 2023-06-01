#ifndef LOG_UTILITIES_H
#define LOG_UTILITIES_H

#include <limits.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <openssl/ssl.h>

#include "../IntervalTree.h"
#include "uthash.h"
#include "logStructures.h"

#define BACKUP_DEST "../AuditLog/Backups/"
#define AUDIT_LOG "../AuditLog/"

extern int setup;
extern SysData* systemData;

/// @brief gets the structure which holds all metadata for the system
/// @return metadata strucutre for the system
SysData *getSysData();

/// @brief create all the function pointers to the real versions of the
/// i/o functions
void initRealFunctions();

/// @brief Check whether the file at the given path is supposed to be interposed or not
/// @param path path to the file we are checking interposition for
/// @return 1 if the file is supposed to be interposed 0 if not
int checkInterposition(char *path);

/// @brief Get the metadata structure for ther file whos' pointer is given
/// @param fptr file pointer to the file we want metadata of
/// @return the file metadata structure
fileMetadata *getMetadata(FILE *fptr,int fd);

/// @brief Add the given call to the given metadata structure
/// @param metadata metadata structure of file to which we want to add the call
/// @param call Call to add to the structure
void addCall(fileMetadata *metadata, CallList *call);

/// @brief Given a write tree and the intervals to a read call return
/// a list of intervals to append to the write tree
/// @param writeTree Write tree for the given file
/// @param readInterval  Interval of the current read to process
/// @return NULL if nothing to add a LL of intervals otherwise
NodeList *getNonIntersectingRead(Node *writeTree, Interval *readInterval);

/// @brief Print all the metadata for the given file
/// @param metadata Metadata structure for which we have to print the metadata for
/// @param stream Stream to print it to
void printMetadata(fileMetadata *metadata, FILE* steam);

/// @brief Print the calls for the given file
/// @param metadata Metadata structure fo the file to pritn calls for
/// @param stream stream to print to
void printCalls(fileMetadata *metadata, FILE *stream);

/// @brief Given a file metadata structure and a write interval perform the backup
/// and remove neccessary parts from the write tree
/// @param metadata File metadata structure of the file in question
/// @param writeInterval Interval for the write call
void performBackup(fileMetadata *metadata, Interval *writeInterval);

/// @brief Flush the char buf to the apt backup file for the file we are given the path of
/// @param fName abs path of file we want to flush our backup for
/// @param buf buf we want to flush
void flushBackup(char *fName, char *buf);

/// @brief Read size amount of data at readOffset and then reset file pointer to to oldOffset
/// @param fptr File pointer of the file to read from
/// @param fd File desc of the file to read from
/// @param readOffset offset to read from
/// @param size size to read
/// @param oldOffset offset to set to after read
/// @return char buffer of data read
char *readFile(FILE *fptr, int fd, off_t readOffset, off_t size, off_t oldOffset);

/// @brief Add the backup structure to the file metadata strucutre
/// @param metadata File emtadata for th efile to add backup to
/// @param interval Interval of file for which we are adding the backup
void addBackup(fileMetadata *metadata, Interval *interval);

/// @brief Given a file metadata struct print all the backups info into stream provided
/// @param metadata File metadata struct to print for
/// @param stream Stream to print to
void printBackup(fileMetadata *metadata, FILE *stream);

/// @brief Give the char version of the call type enum
/// @param  enumeration of the type of call
/// @return a char ptr to the call type string representation
char *getCharOfCall(enum CallType type);

/// @brief Check if we are tracking the file of given path
/// @param path Path to check
/// @return 1 if tracking else 0
int inList(char *path);

unsigned char* getSHA256(void* buf, int len);
#endif