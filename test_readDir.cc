#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

#include "myFat32.cc"

int main() {

        printf("\n--------------TEST1 READDIR----------------\n");
        dirEnt *dirs = OS_readDir("/");
        if(dirs != NULL) {
                for(int i=0; i<100; i++) {
                        if(dirs[i].dir_attr == 0) {
				printf("TOTAL: %d\n", i);
                                break;
			}
                        //printDirEnt(&dirs[i]);
                }
        } else
        	printf("NULL\n");

        printf("\n--------------TEST2 READDIR----------------\n");
	dirs = NULL;
        dirs = OS_readDir("/PEOPLE");
	int i;
	if(dirs != NULL) {
                for(i=0; i<100; i++) {
                        if(dirs[i].dir_attr == 0) {
				printf("TOTAL: %d\n", i);
                                break;
			}
                        //printDirEnt(&dirs[i]);
                }
        } else
                printf("NULL\n");

        printf("\n--------------TEST3 READDIR----------------\n");
	dirs = NULL;
        dirs = OS_readDir("/PEOPLE/AG8T/");
        if(dirs != NULL) {
                for(int i=0; i<100; i++) {
                        if(dirs[i].dir_attr == 0) {
				printf("TOTAL: %d\n", i);
                                break;
			}
                        printDirEnt(&dirs[i]);
                }
        } else
                printf("NULL\n");

        printf("\n--------------TEST4 READDIR----------------\n");
	dirs = NULL;
	OS_cd("/PEOPLE/AG8T");
        dirs = OS_readDir("../..");
        if(dirs != NULL) {
                for(int i=0; i<100; i++) {
                        if(dirs[i].dir_attr == 0) {
				printf("TOTAL: %d\n", i);
                                break;
			}
                        //printDirEnt(&dirs[i]);
                }
        } else
                printf("NULL\n");
        printf("\n--------------TEST4 READDIR----------------\n");
	dirs = NULL;
	dirs = OS_readDir("/PEOPLE/vana");
        if(dirs == NULL) 
                printf("NULL\n");

	free(dirs);
} 
