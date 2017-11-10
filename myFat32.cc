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

int main() {

	initFAT();
	//tokenizePath("/mnt/Media");
} 

/*void tokenizePath(const char *dirpath) {
	//char *path = (char *)dirpath;
	//printf("%s\n\n", path);
        char *dir;
	

        dir = strtok(path, "/");
	printf("%s\n\n", dir);
        while( dir != NULL ) {

                dir = strtok( NULL, "/" );
		int count;
		dirEnt * dirs = getDirEs(rootDir, &bpbcomm, inFile, &count);

		for(int i=0; i<count; i++) {
			print_dirEnt(&dirs[i]);
		}
		free(dirs);
		printf("Path: %s \t", dir);
        }
} */

int initFAT() {

	char *rawDisk = getenv("FAT16_FS_PATH");
	
	cout << "Hello FAT16\n";
	cout << "Reading File: " << rawDisk << endl;

	int inFile;
	inFile = open(rawDisk, O_RDWR);

	bpbFat32 bpbcomm;
	memset(&bpbcomm, 0, sizeof(bpbFat32));
	safe_read(inFile, (uint8_t *)&bpbcomm, sizeof(bpbFat32), 0x00);
	print_fat32(&bpbcomm);


	dirEnt rootDir = initializeRootDir(&bpbcomm, inFile);	
	print_dirEnt(&rootDir);

	getDirEnts(rootDir, &bpbcomm, inFile);
	/*int count;
	dirEnt * dirs = getDirEs(rootDir, &bpbcomm, inFile, &count);

	for(int i=0; i<count; i++) {
		print_dirEnt(&dirs[i]);
	}
	free(dirs); */
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
		if((dirInfo.dir_fileSize != -1) && (dirInfo.dir_name[0] != 0))
			print_dirEnt(&dirInfo);
		if(dirInfo.dir_attr == 0x10) {

			int count;
        		dirEnt * dirs = getDirEs(dirInfo, &bpbcomm, inFile, &count);

        		for(int i=0; i<count; i++) {
                		print_dirEnt(&dirs[i]);
        		}
        		free(dirs);

		}
		if(dirInfo.dir_attr == 0x20) {
			dirEnt myFile;

			int first_sec_of_cluster = getFirstDataSec(bpbcomm, dirInfo.dir_fstClusLO);
	
			uint8_t buf[dirInfo.dir_fileSize];
			safe_read(inFile, (uint8_t *)&buf, dirInfo.dir_fileSize, first_sec_of_cluster*bpbcomm->bpb_bytesPerSec);
			printf("%s\n",buf);
		} 
		i++;
	}  */
	dirEnt * dirs = (dirEnt *)malloc(100 * sizeof(dirEnt));
	int i=0;
	int first_data_sec = getFirstDataSec(bpbcomm, bpbcomm->bpb_RootClus);
	int root_offset = first_data_sec * bpbcomm->bpb_bytesPerSec;
	cout<<"Reading Directories in Root"<<endl;
	int next = 32;
	while(dirInfo.dir_name[0] != 0 ){
		next += sizeof(dirEnt);
		safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), root_offset+next);
		if((dirInfo.dir_fileSize != -1) ){
			if(dirInfo.dir_attr == 0x10) {

				int count;
        			dirEnt * dirs = getDirEs(dirInfo, bpbcomm, inFile, &count);

        			for(int j=0; j<count; j++) {
                			print_dirEnt(&dirs[j]);
        			}
        			free(dirs);
			}
			else 
				print_dirEnt(&dirInfo);
			i++;
		}
	} 
}

dirEnt * getDirEs(dirEnt dirInfo, bpbFat32 *bpbcomm, int inFile, int *count) {

	dirEnt * dirs = (dirEnt *)malloc(100 * sizeof(dirEnt));
	int i=0;
	int first_data_sec = getFirstDataSec(bpbcomm, bpbcomm->bpb_RootClus);
	int root_offset = first_data_sec * bpbcomm->bpb_bytesPerSec;
	cout<<"DIR-E\n"<<endl;
	int next = 32;
	while(dirInfo.dir_name[0] != 0 ){
		next += sizeof(dirEnt);
		safe_read(inFile, (uint8_t *)&dirInfo, sizeof(dirEnt), root_offset+next);
		if((dirInfo.dir_fileSize != -1) ){
			dirs[i] = dirInfo;
			i++;
		}
	} 
	*count = i-1;
	return dirs;

}


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
