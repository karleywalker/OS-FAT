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

        printf("\n--------------TEST1 OPEN----------------\n");
        int fd = -1;
        fd = OS_open("/CONGRATSTXT");
        printf("FD: %d\n", fd);

        printf("\n--------------TEST1 CLOSE----------------\n");
        OS_close(fd);
        printf("FD Count: %d \n", fdCount);

        printf("\n--------------TEST2 OPEN----------------\n");
        fd = OS_cd("/PEOPLE/AG8T");
        fd = -1;
        fd = OS_open("GATE-C~1TXT");
        printf("FD: %d\n", fd);

        printf("\n--------------TEST3 OPEN----------------\n");
        fd = OS_cd("../../");
        fd = -1;
        fd = OS_open("CONGRATSTXT");
        printf("FD: %d\n", fd);

        printf("\n--------------TEST2 CLOSE----------------\n");
        OS_close(fd);
        printf("FD Count: %d \n", fdCount);


        printf("\n--------------TEST3 CLOSE----------------\n");
        OS_close(100);
        printf("FD Count: %d \n", fdCount);
} 
