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
                        if(dirs[i].dir_attr == 0)
                                break;
                        print_dirEnt(&dirs[i]);
                }
        } else
        	printf("NULL\n");

        printf("\n--------------TEST2 READDIR----------------\n");
        dirs = OS_readDir("/PEOPLE");
	int i;
	if(dirs != NULL) {
                for(i=0; i<100; i++) {
                        if(dirs[i].dir_attr == 0)
                                break;
                        print_dirEnt(&dirs[i]);
                }
        } else
                printf("NULL\n");
	printf("TOTAL: %d\n", i);

        /*printf("\n--------------TEST3 READDIR----------------\n");
        dirs = OS_readDir("/PEOPLE/AG8T/");
        if(dirs != NULL) {
                for(int i=0; i<100; i++) {
                        if(dirs[i].dir_attr == 0)
                                break;
                        print_dirEnt(&dirs[i]);
                }
        } else
                printf("NULL\n");
	*/

	free(dirs);
} 
