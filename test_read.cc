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

        int fd = 0;

        printf("\n--------------TEST1 READFILE----------------\n");
        fd = OS_open("/CONGRATSTXT");
	int numbytes = 11;
        char *buf = (char *)malloc(numbytes*sizeof(char));
        int bytes = OS_read(fd, buf, numbytes, 0);
	printf("\n%s\n",buf);
        OS_close(fd);


        printf("\n--------------TEST2 READFILE----------------\n");
        fd = OS_open("/PEOPLE/AG8T/GATE-C~1TXT");
	numbytes = 100;
        buf = (char *)malloc(numbytes*sizeof(char));
        bytes = OS_read(fd, buf, numbytes, 20);
	printf("\n%s\n",buf);
        OS_close(fd);

} 
