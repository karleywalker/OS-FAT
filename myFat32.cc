/* fat32.cc
* Program implements FAT16 filesystem to support read/writing to FS.
* 
*
*
* Author: Vanamala Venkataswamy
* Date: 11-03-2017
*/

#include <cstring>
#include "myFat32.h"

#define READDIR 1
#define CD 2
#define OPEN 3

bpbFat32 bpbcomm;
dirEnt rootDir;
dirEnt cwd;
int inFile;
char *cwdPath;

int main() {

	initFAT();
	print_fat32(&bpbcomm);
	print_dirEnt(&rootDir);
	printf("Initialized\n");

	dirEnt *dirs = OS_readDir("/PEOPLE/AG8T");

	if(dirs != NULL) {
		for(int i=0; i<100; i++) {
			if(dirs[i].dir_attr == 0)
				break;
			print_dirEnt(&dirs[i]);
		}
	} else
		printf("NULL\n");

	printf("Before CD: %s\n", cwdPath);
	int status = OS_cd("/PEOPLE");
	print_dirEnt(&cwd);
	printf("Status: %d\n", status);
	printf("After CD: %s\n", cwdPath); 
	
} 


void tokenize(char *string, char *path[], int *depth) {
        char *token;

        /* Establish string and get the first token: */
        token = strtok(string, "/");
        path[0] = token;
        int count = 1;
        while( token != NULL ) {
                token = strtok( NULL, "/");
                path[count++] = token;
        }
        (*depth) = count-1;
        path[count] = NULL;
}

int initFAT() {

	//get env for FAT dir
	char *rawDisk = getenv("FAT16_FS_PATH");

	//open FAT disk file	
	inFile = open(rawDisk, O_RDWR);

	//initialize cwd
	cwdPath = (char *)"/";

	//get MBR	
	memset(&bpbcomm, 0, sizeof(bpbFat32));
	safe_read(inFile, (uint8_t *)&bpbcomm, sizeof(bpbFat32), 0x00);
	
	//get root directory info
	memset(&rootDir, 0, sizeof(dirEnt));
	rootDir = initializeRootDir(&bpbcomm, inFile);	

	/*int count;
	dirEnt * dirs = getDirEs(rootDir, &bpbcomm, inFile, bpbcomm.bpb_RootClus, &count);

	for(int i=0; i<count; i++) {
		print_dirEnt(&dirs[i]);
	}

	printf("SUBBBBBB\n");
	print_dirEnt(&dirs[1]);
	printf("SUBBBBBB\n");
	dirEnt * subdirs = getDirEs(dirs[0], &bpbcomm, inFile, dirs[0].dir_fstClusLO, &count);
	for(int i=0; i<count; i++) {
		print_dirEnt(&subdirs[i]);
	}
	free(dirs); 
	free(subdirs);  */
} 

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

int getFirstDataSec(bpbFat32 *bpbcomm, int N) {
	int root_sec = (( bpbcomm->bpb_rootEntCnt * 32 ) + (bpbcomm->bpb_bytesPerSec -1 )) / bpbcomm->bpb_bytesPerSec;
	int first_data_sec = bpbcomm->bpb_rsvdSecCnt + ( bpbcomm->bpb_numFATs * bpbcomm->bpb_FATSz32 ) + root_sec;
        int first_sec_of_cluster = (( N - 2) * bpbcomm->bpb_secPerClus ) + first_data_sec ;
	return first_sec_of_cluster;
}

dirEnt initializeRootDir(bpbFat32 *bpbcomm, int inFile) {
	dirEnt dirInfo;
	memset(&dirInfo, 0, sizeof(dirEnt));
	int first_data_sec = getFirstDataSec(bpbcomm, bpbcomm->bpb_RootClus);
	safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), first_data_sec*bpbcomm->bpb_bytesPerSec);
	return dirInfo;
}

int getDirEnts(dirEnt dirInfo, bpbFat32 *bpbcomm, int inFile) {
	/*int i=1;
	int first_data_sec = getFirstDataSec(bpbcomm, bpbcomm->bpb_RootClus);
	int root_offset = first_data_sec * bpbcomm->bpb_bytesPerSec;
	cout<<"Reading Directories in Root"<<endl;
	while(dirInfo.dir_name[0] != 0 ) {
		int next = i * sizeof(dirEnt);
		safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), root_offset+next);
		if(dirInfo.dir_fileSize != -1))
			print_dirEnt(&dirInfo);
		if(dirInfo.dir_attr == 0x10) {

			int count;
        		dirEnt * dirs = getDirEs(dirInfo, &bpbcomm, inFile, &count);

        		for(int i=0; i<count; i++) {
                		print_dirEnt(&dirs[i]);
        		}
        		free(dirs);
		if(dirInfo.dir_attr == 0x20) {
			dirEnt myFile;

			int first_sec_of_cluster = getFirstDataSec(bpbcomm, dirInfo.dir_fstClusLO);
	
			uint8_t buf[dirInfo.dir_fileSize];
			safe_read(inFile, (uint8_t *)&buf, dirInfo.dir_fileSize, first_sec_of_cluster*bpbcomm->bpb_bytesPerSec);
			printf("%s\n",buf);
		} 
		i++;
	}  */
}

dirEnt * getDirEs(dirEnt dirInfo, bpbFat32 *bpbcomm, int inFile, int cluster, int *count) {

	dirEnt * dirs = NULL;
	int i=0;
	int first_data_sec = getFirstDataSec(bpbcomm, cluster);
	int root_offset = first_data_sec * bpbcomm->bpb_bytesPerSec;
	int next = 32;
	while(dirInfo.dir_name[0] != 0 ){
		next += sizeof(dirEnt);
		safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), root_offset+next);
		if((dirInfo.dir_fileSize != -1) && (dirInfo.dir_fstClusLO != 0)) {
			dirs = (dirEnt *)realloc_or_free(dirs, sizeof *dirs * next);
			dirs[i] = dirInfo;
			i++;
		}
	} 
	*count = i-1;
	return dirs;
}

static void *realloc_or_free(void *ptr, size_t size) {
  void *tmp = realloc(ptr, size);
  if (tmp == NULL) {
    free(ptr);
  }
  return tmp;
}


dirEnt * OS_readDir(const char *dirpath) {
	dirEnt *dirs = NULL;

	lookUp(dirpath, READDIR, 0, &dirs); 

	if(dirs != NULL)
		return dirs;
	else 
		return NULL;

}

int OS_cd(const char *dirpath){

	dirEnt *dirs = NULL;
	int status = 0;

	lookUp(dirpath, CD, &status, &dirs);

        if(status == 1)
                return 1;
        else
                return -1;
}

void lookUp(const char *dirpath, int opType, int *status, dirEnt **entries) {

   	char *path_tokens[1000];
        int depth = 0;
	int cluster = 0;
        char *path;
	bool found = false;
	dirEnt lookupDir = rootDir;
	dirEnt *dirs = NULL;
	dirEnt prevCwd;
	char *prevPath;

	//save old paths
	if(opType == CD) {
		prevCwd = cwd;
		prevPath = cwdPath;
	}

	path = (char *) malloc (strlen(dirpath+1)*sizeof(char *));
	cluster = bpbcomm.bpb_RootClus;

        strcpy(path, dirpath);
        tokenize(path, path_tokens, &depth);

        for(int i=0; i<depth; i++) {
                //printf("Lookup Dir: %s\n", path_tokens[i]);
		int count = 0;
		dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);
		found = false;

		for(int j=0; j<count; j++) {
			//printf("Looking for %s %d in %s\n", path_tokens[i], (int)strlen(path_tokens[i]), dirs[j].dir_name);
			if(strncmp(path_tokens[i], (char *)dirs[j].dir_name, strlen(path_tokens[i])) == 0){

				printf("MATCH Found: %s\n", path_tokens[i]);
				lookupDir = dirs[j];
				cluster = dirs[j].dir_fstClusLO;
				if((i==depth-1) && dirs[j].dir_attr != 0x10) {
					if(opType == CD)
						*status = -1;
					return;
				} else {
					if(opType == READDIR) {
						dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);
					} else if(opType == CD) {
						cwd = lookupDir;
					}
					found = true;
					break;
				}
			} else {
				found = false;
			}
		}
	} //end of for
	if(found) {
		if(opType == CD) {
			cwdPath = (char *)dirpath;
			free(dirs);
			*status = 1;
		} else if(opType == READDIR) {
			*entries = dirs;
		}
	}
	else {
		if(opType == CD) {
			cwd = prevCwd;
			strcpy(cwdPath,prevPath);
			free(dirs);
			*status = -1;
		} else if(opType == READDIR) {
			entries = NULL;
		}
	}
}

/*int OS_cd(const char *dirpath){

   	char *path_tokens[1000];
        int depth = 0;
        int cluster = 0;
        char *path;
        bool found = false;
        dirEnt lookupDir = rootDir;
        dirEnt *dirs = NULL;
	int i, j;

	//save old paths
	dirEnt prevCwd = cwd;
	char *prevPath = cwdPath;

        path = (char *) malloc (strlen(dirpath+1)*sizeof(char *));
        cluster = bpbcomm.bpb_RootClus;

        strcpy(path, dirpath);
        tokenize(path, path_tokens, &depth);


        for(i=0; i<depth; i++) {
                //printf("Lookup Dir: %s\n", path_tokens[i]);
                int count = 0;
                dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);
                found = false;

                for(j=0; j<count; j++) {
                        //printf("Looking for %s %d in %s\n", path_tokens[i], (int)strlen(path_tokens[i]), dirs[j].dir_name);
                        if(strncmp(path_tokens[i], (char *)dirs[j].dir_name, strlen(path_tokens[i])) == 0){

                                //printf("MATCH Found: %s\n", path_tokens[i]);
                                lookupDir = dirs[j];
                                cluster = dirs[j].dir_fstClusLO;
                                if((i==depth-1) && dirs[j].dir_attr != 0x10) {
                                        return -1;
                                } else {
                                        //dirs = getDirEs(lookupDir, &bpbcomm, inFile, cluster, &count);
					cwd = lookupDir;
                                        found = true;
                                        break;
                                }
                        } else {
                                found = false;
                        }
                }
	} //end for
	if(found) {
		cwdPath = (char *)dirpath;
		free(dirs);
		return 1;
	}
	else {
		cwd = prevCwd;
		strcpy(cwdPath,prevPath);
		free(dirs);
		return -1;
	}
} */

int OS_open(const char *path) {

	


}
/*int OS_close(int fd);
int OS_read(int fildes, void *buf, int nbyte, int offset); */

int print_fat32(bpbFat32 *bpbInfo) {
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


int print_dirEnt(dirEnt *dirInfo) {
	cout<< "\n--------DIR_ENT--------" << endl;
      	printf("dir_name:\t\t%s %d\n",dirInfo->dir_name, dirInfo->dir_name[0]);
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
