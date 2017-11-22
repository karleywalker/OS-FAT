/* myFat32.cc
* Program implements FAT32 filesystem to support read-only operations on FS.
* Program reads a raw FAT32 file and depending on function call, performs 
* operations on the FS.
*
* Author: Vanamala Venkataswamy
* Date: 11-03-2017
* Last Modified Date: 11-22-2017
*/

#include <cstring>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h> 

using namespace std;

typedef struct __attribute__ ((packed)) {

        uint8_t bs_jmpBoot[3];          // jmp instr to boot code
        uint8_t bs_oemName[8];          // indicates what system formatted this field, default=MSWIN4.1
        uint16_t bpb_bytesPerSec;       // Count of bytes per sector
        uint8_t bpb_secPerClus;         // no.of sectors per allocation unit
        uint16_t bpb_rsvdSecCnt;        // no.of reserved sectors in the resercved region of the volume starting at 1st sector
        uint8_t bpb_numFATs;            // The count of FAT datastructures on the volume
        uint16_t bpb_rootEntCnt;        // Count of 32-byte entries in root dir, for FAT32 set to 0
        uint16_t bpb_totSec16;          // total sectors on the volume
        uint8_t bpb_media;              // value of fixed media
        uint16_t bpb_FATSz16;           // count of sectors occupied by one FAT
        uint16_t bpb_secPerTrk;         // sectors per track for interrupt 0x13, only for special devices
        uint16_t bpb_numHeads;          // no.of heads for intettupr 0x13
        uint32_t bpb_hiddSec;           // count of hidden sectors
        uint32_t bpb_totSec32;          // count of sectors on volume
        uint32_t bpb_FATSz32;           // define for FAT32 only
        uint16_t bpb_extFlags;          // Reserved for FAT32
        uint16_t bpb_FSVer;             // Major/Minor version num
        uint32_t bpb_RootClus;          // Clus num of 1st clus of root dir
        uint16_t bpb_FSInfo;            // sec num of FSINFO struct
        uint16_t bpb_bkBootSec;         // copy of boot record
        uint8_t bpb_reserved[12];       // reserved for future expansion
        uint8_t bs_drvNum;              // drive num
        uint8_t bs_reserved1;           // for ue by NT
        uint8_t bs_bootSig;             // extended boot signature
        uint32_t bs_volID;              // volume serial number
        uint8_t bs_volLab[11];          // volume label
        uint8_t bs_fileSysTye[8];       // FAT12, FAT16 etc
} bpbFat32 ;

typedef struct __attribute__ ((packed)) {
        uint8_t dir_name[11];           // short name
        uint8_t dir_attr;               // File sttribute
        uint8_t dir_NTRes;              // Set value to 0, never chnage this
        uint8_t dir_crtTimeTenth;       // millisecond timestamp for file creation time
        uint16_t dir_crtTime;           // time file was created
        uint16_t dir_crtDate;           // date file was created
        uint16_t dir_lstAccDate;        // last access date
        uint16_t dir_fstClusHI;         // high word fo this entry's first cluster number
        uint16_t dir_wrtTime;           // time of last write
        uint16_t dir_wrtDate;           // dat eof last write
        uint16_t dir_fstClusLO;         // low word of this entry's first cluster number
        uint32_t dir_fileSize;          // 32-bit DWORD hoding this file's size in bytes
} dirEnt;


//Helper functions to print
int printFat32(bpbFat32 *bpbfat32);
int printDirEnt(dirEnt *dirInfo);

//Helper functions
void safe_read(int descriptor, uint8_t *buffer, size_t size, long long offset);
char* tokenize(char *string, char *path[], int *depth);

//FAT related helper functions
int initFAT();
int initializeMBR(bpbFat32 *bpbcomm, int inFile);
dirEnt initializeRootDir(bpbFat32 *bpbcomm, int inFile);
int getFirstDataSec(bpbFat32 *bpbfat32, int N);
int getFileDesc(dirEnt *fileEnt);


//Library Functions to support FAT read-only
int OS_cd(const char *path);
int OS_open(const char *path);
int OS_close(int fd);
int OS_read(int fildes, void *buf, int nbyte, int offset);
dirEnt * OS_readDir(const char *dirname);

//Library Functions to support FAT read-write
int OS_mkdir(const char *path);
int OS_rmdir(const char *path);
int OS_creat(const char *path);
int OS_write(int fildes, const void *buf,int nbytes);
int OS_writeDir(const char *path, dirEnt *entries, int entryCount);


#define SUCCESS 1       // Flag to indicate success
#define FAILURE -1      // Flag to indicate failure

#define READDIR 1       // Flag to indicate readDir operation type
#define CD 2            // Flag to indicate cd operation type
#define OPENFILE 3      // Flag to indicate file open operation type
#define READFILE 4      // Flag to indicate file read operation type
#define MAX_OPEN 129

bpbFat32 bpbcomm;       // global structure to store MBR
dirEnt rootDir;         // globar structure to store rootDir entry
dirEnt cwd;             // current working directory
int inFile;             // file desriptor to point to raw disk
char *cwdPath;          // current working dir name
int fdCount;            // no.of open file descriptor count
dirEnt fdTable[MAX_OPEN];       // array to store directory entries of open files

int init = 0;           // global variable to indicate if FAT initialization was done or not, 1 indicates initialized


// function to tokenize path based on / as delimiter
char * tokenize(char *string, char *path[], int *depth) {
        char *token;

        char *fullpath;
        char *relpath;
        int relative = 0;

        //If relative path
        if(string[0] != '/') {
                //add cwd to begining of path
                int newlen = strlen(string) + strlen(cwdPath)+1;
                fullpath = (char *)malloc(sizeof(char) * newlen);
                relpath = (char *)malloc(sizeof(char) * newlen);
                relpath[0] = '\0';
                fullpath[0] = '\0';
                strcat(fullpath,cwdPath);
                strcat(fullpath,"/");
                strcat(fullpath,string);
                fullpath[newlen] = '\0';
                relative = 1;
        } else {        // not replative path

                int newlen = strlen(string)+1;
                fullpath = (char *)malloc(sizeof(char) * newlen);
                relpath = (char *)malloc(sizeof(char) * newlen);
                relpath[0] = '\0';
                fullpath[0] = '\0';
                strcat(fullpath,string);
        }
        //printf("REL PATH = %s\n", fullpath);

        token = strtok(fullpath, "/");
        path[0] = token;
        int count = 1;
        while( token != NULL ) {
                token = strtok( NULL, "/");
                if(token == NULL)
                        break;
                if( strncmp(token,"..", 2) == 0) {
                        count--;
                        continue;
                } else if( strncmp(token,".", 1) == 0) {
                        path[count] = token;
                        continue;
                } else {
                        path[count++] = token;
                }
        }
        (*depth) = count;
        path[count] = NULL;

        if(relative == 1) {
                strcat(relpath,"/");
                for(int i=0; i<count; i++) {
                        strcat(relpath, path[i]);
                        strcat(relpath,"/");
                }
                //printf("REL FULL PATH = %s\n", relpath);
        } else {
                relpath = string;
                //printf("FULL PATH = %s\n", relpath);
        }
        return relpath;



}

// general read function to seek to a offset and read data into buffer
void safe_read(int descriptor, uint8_t *buffer, size_t size, long long offset){
        lseek(descriptor, offset, SEEK_SET);
        int remaining = size;
        int read_size;
        uint8_t *pos = buffer;
        do {
                read_size = read(descriptor, pos, remaining);
                pos += read_size;
                remaining -= read_size;
        } while (remaining > 0);
}


// function to initialize root directory struture
dirEnt initializeRootDir(bpbFat32 *bpbcomm, int inFile) {
        dirEnt dirInfo;
        memset(&dirInfo, 0, sizeof(dirEnt));
        int first_data_sec = getFirstDataSec(bpbcomm, bpbcomm->bpb_RootClus);
        safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), first_data_sec*bpbcomm->bpb_bytesPerSec);
        return dirInfo;
}

// function to initilaize FAT, read MBR and Root Directory entries
int initFAT() {

        //get env for FAT dir
        char *rawDisk = getenv("FAT_FS_PATH");

        if(rawDisk == NULL) {
                printf("Please export FAT_FS_PATH env\n");
                init = 0; 
                return 0;
        } 

        //open FAT disk file
        inFile = open(rawDisk, O_RDWR);

        //initialize cwd
        cwdPath = (char *)"/";
        fdCount = 0;

        //get MBR
        memset(&bpbcomm, 0, sizeof(bpbFat32));
        safe_read(inFile, (uint8_t *)&bpbcomm, sizeof(bpbFat32), 0x00);

        //get root directory info
        memset(&rootDir, 0, sizeof(dirEnt));
        rootDir = initializeRootDir(&bpbcomm, inFile);

        //initialize fdTable
        fdTable[MAX_OPEN] = {{0}};

        init = 1;

        return SUCCESS;

} 

// function to get fist data sector given cluster number
int getFirstDataSec(bpbFat32 *bpbcomm, int N) {
        int root_sec = (( bpbcomm->bpb_rootEntCnt * 32 ) + (bpbcomm->bpb_bytesPerSec -1 )) / bpbcomm->bpb_bytesPerSec;
        int first_data_sec = bpbcomm->bpb_rsvdSecCnt + ( bpbcomm->bpb_numFATs * bpbcomm->bpb_FATSz32 ) + root_sec;
        int first_sec_of_cluster = (( N - 2) * bpbcomm->bpb_secPerClus ) + first_data_sec ;
        return first_sec_of_cluster;
}

// function to read directory entries of given path
dirEnt * getDirEs(dirEnt dirInfo, bpbFat32 *bpbcomm, int inFile, int cluster, int *count) {

        dirEnt * dirs = NULL;
        int i=0;
        int first_data_sec = getFirstDataSec(bpbcomm, cluster);
        int root_offset = first_data_sec * bpbcomm->bpb_bytesPerSec;
        int next = sizeof(dirEnt);
        while(dirInfo.dir_name[0] != 0 ){
                next += sizeof(dirEnt);
                safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), root_offset+next);
                if((dirInfo.dir_fileSize != -1) && (dirInfo.dir_fstClusLO != 0)) {
                        //printDirEnt(&dirInfo);
                        dirs = (dirEnt *)realloc(dirs, sizeof *dirs * next);
                        dirs[i] = dirInfo;
                        i++;
                }
        }
        //*count = i-1;
        *count = i;
        return dirs;
}

// get file discrptor when file, does not handle if entry already exists
int getFileDesc(dirEnt *fileEnt){

        //check if the file is already open
        for(int i=0; i<MAX_OPEN; i++) {
                if(strcmp((char *)fdTable[i].dir_name, (char *)fileEnt->dir_name) == 0)
                        return i;
        }
        //printf("Incrementing fd from %d to %d\n", fdCount, fdCount+1);
        for(int i=0; i<MAX_OPEN; i++) {
                if( fdTable[i].dir_attr == 0 && fdCount != MAX_OPEN) {
                        fdCount = fdCount + 1;
                        fdTable[i] = *fileEnt;
                        return i;
                }
        }
        //something went wrong
        return -1;
}



// change directory from current to new directory
int OS_cd(const char *dirpath){

        char *path_tokens[1000];
        int depth = 0;
        int cluster = 0;
        char *path;
        bool found = false;
        dirEnt lookupDir = rootDir;
        dirEnt *dirs = NULL;
        dirEnt prevCwd;
        char *prevPath;

        if(init == 0) {
                int ret = initFAT();
                if(ret != SUCCESS) {
                        printf("FAT initialization Failed.. exiting\n");
                        return FAILURE;
                }
        }

        prevCwd = cwd;
        prevPath = cwdPath;


        path = (char *) malloc (strlen(dirpath+1)*sizeof(char *));
        cluster = bpbcomm.bpb_RootClus;

        strcpy(path, dirpath);
        char *retpath = tokenize(path, path_tokens, &depth);
        if(strcmp(retpath, "/") == 0) {
                cwdPath = retpath;
                return SUCCESS;
        }

        for(int i=0; i<depth; i++) {
                //printf("Lookup Dir: %s\n", path_tokens[i]);
                int count = 0;
                found = false;

                dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);

                if(depth == 1)
                        count = count+1;

                for(int j=0; j<count; j++) {
                        //printf("Comparing %s and %s i=%d j=%d\n", path_tokens[i], (char *)dirs[j].dir_name, i, j);
                        if(strncmp(path_tokens[i], (char *)dirs[j].dir_name, strlen(path_tokens[i])) == 0){

                                //printf("Looking for %s in %s \n", path_tokens[i], dirs[j].dir_name);
                                lookupDir = dirs[j];
                                cluster = dirs[j].dir_fstClusLO;
                                if((i==depth-1) && dirs[j].dir_attr != 0x10) { //reached end, no directory found
                                        free(dirs);
                                        return FAILURE;
                                } else {
                                        //printf("CD %s\n", path_tokens[i]);
                                        cwd = lookupDir;
                                        found = true;
                                        break;
                                }
                        } else { // directory name compare didnt match
                                found = false;
                        } //end of else
                } //end of inner for
                free(dirs);
                dirs = NULL;
        } //end of outer for

        if(found) {
                cwdPath = (char *)retpath;
                return SUCCESS;
        } else {
                cwd = prevCwd;
                cwdPath = prevPath;
                printf("ERROR: Failed to change directory %s\n", dirpath);
                return FAILURE;
        } 
}

// read directory entries for given path
dirEnt * OS_readDir(const char *dirpath) {

        if(init == 0) {
                int ret = initFAT();
                if(ret != SUCCESS) {
                        printf("FAT initialization Failed.. exiting\n");
                        return NULL;
                }
        }

        char *path_tokens[1000];
        int depth = 0;
        int cluster = 0;
        int fd = -1;
        char *path;
        bool found = false;
        dirEnt lookupDir = rootDir;
        dirEnt *dirs = NULL;
        dirEnt prevCwd;
        char *prevPath;

        //save old paths
        prevCwd = cwd;
        prevPath = cwdPath;

        path = (char *) malloc (strlen(dirpath+1)*sizeof(char *));
        cluster = bpbcomm.bpb_RootClus;

        strcpy(path, dirpath);
        //printf("OLD PATH = %s\n", path);
        char *retpath = tokenize(path, path_tokens, &depth);
        //printf("NEW PATH = %s\n", path);

        int k;
        if(strcmp(retpath, "/") == 0) {
                int count=0;
                dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);

                //resize dirEnts
                for(k = 0; k < 64; k++) {
                        if(dirs[k].dir_fstClusHI != 0) break;
                }
                if (k != 64) {
                        dirs = (dirEnt *)realloc(dirs, k * sizeof(dirEnt));
                }
                return dirs;
        }

        for(int i=0; i<depth; i++) {
                //printf("Lookup Dir: %s\n", path_tokens[i]);
                int count = 0;
                found = false;

                dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);

                if(depth ==1)
                        count = count+1;

                for(int j=0; j<count; j++) {
                        //printf("Looking for %s %d in %s \n", path_tokens[i], (int)strlen(path_tokens[i]), dirs[j].dir_name);
                        if(strncmp(path_tokens[i], (char *)dirs[j].dir_name, strlen(path_tokens[i])) == 0){

                                //printf("Looking for %s in %s \n", path_tokens[i], dirs[j].dir_name);
                                lookupDir = dirs[j];
                                cluster = dirs[j].dir_fstClusLO;

                                //printf("READDIR\n");
                                if((i==depth-1) && dirs[j].dir_attr != 0x10) { //reached end, no directory found
                                        free(dirs);
                                        return NULL;
                                } else {
                                        found = true;
                                        dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);
                                        break;
                                }
                        } else { // directory name compare didnt match
                                found = false;
                        } //end of else
                } //end of inner for
        } //end of outer for

        //resize dirEnts
        for(k = 0; k < 64; k++) {
                if(dirs[k].dir_fstClusHI != 0) break;
        }
        if (k != 64) {
                dirs = (dirEnt *)realloc(dirs, k * sizeof(dirEnt));
        }

        if(found) {
                return dirs;
        } else {
                free(dirs);
                printf("ERROR: Failed to read directory %s\n", dirpath);
                return NULL;
        }

}

// open a file
int OS_open(const char *dirpath) {

        if(init == 0) {
                int ret = initFAT();
                if(ret != SUCCESS) {
                        printf("FAT initialization Failed.. exiting\n");
                        return FAILURE;
                }
        }

        char *path_tokens[1000];
        int depth = 0;
        int cluster = 0;
        int fd = -1;
        bool found = false;
        dirEnt lookupDir = rootDir;
        dirEnt *dirs = NULL;
        dirEnt prevCwd;
        char *prevPath;

        prevCwd = cwd;
        prevPath = cwdPath;

        char *path = (char *) malloc (strlen(dirpath+1)*sizeof(char *));
        cluster = bpbcomm.bpb_RootClus;

        strcpy(path, dirpath);
        //printf("OLD PATH = %s\n", path);
        char *retpath = tokenize(path, path_tokens, &depth);
        //printf("NEW PATH = %s\n", retpath);

        if(strcmp(retpath, "/") == 0) {
                return FAILURE;
        }


        for(int i=0; i<depth; i++) {
                //printf("Lookup Dir: %s\n", path_tokens[i]);
                int count = 0;
                found = false;

                dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);

                //printf("After Dir\n");
                //printDirEnt(&dirs[2]);

                if(count == 0 && dirs!= NULL)
                        count = count+1;

                //printf("DEPTH = %d COUNT = %d\n", depth, count);
                for(int j=0; j<count; j++) {
                        //printf("Looking for %s %d in %s count %d i=%d j=%d\n", path_tokens[i], (int)strlen(path_tokens[i]), dirs[j].dir_name, count, i, j);
                        if(strncmp(path_tokens[i], (char *)dirs[j].dir_name, strlen(path_tokens[i])) == 0){

                                //printf("Looking for %s in %s \n", path_tokens[i], dirs[j].dir_name);
                                lookupDir = dirs[j];
                                cluster = dirs[j].dir_fstClusLO;

                                //printf("OPEN\n");
                                //printf("File found: %s %d\n", lookupDir.dir_name, fdCount);
                                if((i==depth-1) && dirs[j].dir_attr != 0x20) { //reached end, no file found
                                        //printf("Reached here\n");
                                        free(dirs);
                                        dirs = NULL;
                                        return FAILURE;
                                } else if ((i==depth-1) && dirs[j].dir_attr == 0x20){
                                        //printf("File found: %s %d\n", lookupDir.dir_name, fdCount);
                                        fd = getFileDesc(&lookupDir);
                                        printf("GOT FD %d\n", fd);
                                        found = true;
                                        break;
                                } else {
                                        found = false;
                                        continue;
                                }
                        } else { // directory name compare didnt match
                                found = false;
                        } //end of else
                } //end of inner for
                free(dirs);
                dirs = NULL;
        } //end of outer for

        if(found) {
               return fd;
        } else {
                return FAILURE;
        }
}

// close a file
int OS_close(int fd) {

        if(init == 0) {
                int ret = initFAT();
                if(ret != SUCCESS) {
                        printf("FAT initialization Failed.. exiting\n");
                        return FAILURE;
                }
        }

        if( fd>-1 && fd<=MAX_OPEN) {
                if(fdTable[fd].dir_attr != 0) {
                        fdTable[fd] = {{ 0 }};
                        fdCount = fdCount - 1;
                        return SUCCESS;
                } else {
                        printf("ERROR: File not open\n");
                        return FAILURE;
                }
        } else {
                printf("ERROR: Invalid file descriptor\n");
                return FAILURE;
        }

}


/*      Reading contents of the file
*       look up dirEnt in fdTable
*       get cluster number if dirEnt exists
*       seek to right offset
*       read data
*       return bytes read
*/
int OS_read(int fd, void *readbuf, int nbytes, int offset){

        if(init == 0) {
                int ret = initFAT();
                if(ret != SUCCESS) {
                        printf("FAT initialization Failed.. exiting\n");
                        return FAILURE;
                }
        }

        if(fd > -1 && fd <= MAX_OPEN) {
                dirEnt fileInfo = fdTable[fd];
                if(fileInfo.dir_attr != 0x00) {
                        //printDirEnt(&fileInfo);
                        int first_sec_of_cluster = getFirstDataSec(&bpbcomm, fileInfo.dir_fstClusLO);

                        uint8_t buf[nbytes];
                        safe_read(inFile, (uint8_t *)&buf, nbytes, first_sec_of_cluster*bpbcomm.bpb_bytesPerSec+offset);
                        //printf("\n%s\n", buf);
                        strcpy((char *)readbuf, (char *)buf);
                        return nbytes;
                } else {
                        printf("ERROR: File not open\n");
                        return FAILURE;
                }
        } else {
                printf("ERROR: Invalid file descriptor\n");
                return FAILURE;
        }

}

// function to print MBR
int printFat32(bpbFat32 *bpbInfo) {
        cout<< "\n--------BPB--------" << endl;
        printf("bs_jmpBoot:\t\t%04x\n",bpbInfo->bs_jmpBoot[0]);
        printf("bs_oemName:\t\t%s\n",bpbInfo->bs_oemName);
        printf("bpb_bytesPerSec:\t%d\n",bpbInfo->bpb_bytesPerSec);
        printf("bpb_secPerClus:\t\t%d\n", bpbInfo->bpb_secPerClus);
        printf("bpb_rsvdSecCnt:\t\t%d\n",bpbInfo->bpb_rsvdSecCnt);
        printf("bpb_numFATs:\t\t%d\n",bpbInfo->bpb_numFATs);
        printf("bpb_rootEntCnt:\t\t%d\n",bpbInfo->bpb_rootEntCnt);
        printf("bpb_totSec16:\t\t%d\n",bpbInfo->bpb_totSec16);
        printf("bpb_media:\t\t%04x\n",bpbInfo->bpb_media);
        printf("bpb_FATSz16:\t\t%d\n",bpbInfo->bpb_FATSz16);
        printf("bpb_secPerTrk:\t\t%d\n",bpbInfo->bpb_secPerTrk);
        printf("bpb_numHeads:\t\t%d\n",bpbInfo->bpb_numHeads);
        printf("bpb_hiddSecs:\t\t%d\n",bpbInfo->bpb_hiddSec);
        printf("bpb_totSec32:\t\t%d\n",bpbInfo->bpb_totSec32);
        printf("--------FAT32--------\n");
        printf("bpb_FATSz32:\t\t%d\n",bpbInfo->bpb_FATSz32);
        printf("bpb_extFlags:\t\t%d\n",bpbInfo->bpb_extFlags); 
        printf("bpb_FSVer:\t\t%d\n",bpbInfo->bpb_FSVer);
        printf("bpb_RootClus:\t\t%d\n",bpbInfo->bpb_RootClus);
        printf("bpb_FSInfo:\t\t%d\n",bpbInfo->bpb_FSInfo);
        printf("bpb_bkBootSec:\t\t%d\n",bpbInfo->bpb_bkBootSec); 
        printf("bpb_reserved:\t\t%d\n",bpbInfo->bpb_reserved[12]);
        printf("bs_drvNum:\t\t%04x\n",bpbInfo->bs_drvNum);
        printf("bs_reserved1:\t\t%d\n",bpbInfo->bs_reserved1);
        printf("bs_bootSig:\t\t%d\n",bpbInfo->bs_bootSig);
        printf("bs_bs_volID:\t\t%d\n",bpbInfo->bs_volID);
        printf("bs_volLab:\t\t%s\n",bpbInfo->bs_volLab);
        printf("bs_fileSysTye:\t\t%s\n",bpbInfo->bs_fileSysTye);
        cout<< "--------END-FAT32--------" << endl;
}

// function to print dirEnt
int printDirEnt(dirEnt *dirInfo) {
        cout<< "\n--------DIR_ENT--------" << endl;
        printf("dir_name:\t\t%s \n",dirInfo->dir_name);
        printf("dir_attr:\t\t%04x\n",dirInfo->dir_attr);
        printf("dir_NTRes:\t\t%d\n",dirInfo->dir_NTRes);
        printf("dir_crtTimeTenth:\t\t%04x\n",dirInfo->dir_crtTimeTenth);
        printf("dir_crtTime:\t\t%d\n",dirInfo->dir_crtTime);
        printf("dir_crtDate:\t\t%d\n",dirInfo->dir_crtDate);
        printf("dir_lstAccDate:\t\t%d\n",dirInfo->dir_lstAccDate);
        printf("dir_fstClusHI:\t\t%d\n",dirInfo->dir_fstClusHI);
        printf("dir_wrtTime:\t\t%d\n",dirInfo->dir_wrtTime);
        printf("dir_wrtDate:\t\t%d\n",dirInfo->dir_wrtDate);
        printf("dir_fstClusLO:\t\t%d\n",dirInfo->dir_fstClusLO);
        printf("dir_fileSize:\t\t%d\n",dirInfo->dir_fileSize); 
        cout<< "--------DIR_ENT--------" << endl;
}


