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
        int fd = 0;
        fd = OS_open("/CONGRATSTXT");
        printf("FD: %d\n", fd);

        printf("\n--------------TEST2 OPEN----------------\n");
        fd = 0;
        fd = OS_open("/PEOPLE/AG8T/GATE-C~1TXT");
        printf("FD: %d\n", fd);

        printf("\n--------------TEST3 OPEN----------------\n");
        fd = 0;
        fd = OS_open("/PEOPLE/AG8T");
        printf("FD: %d\n", fd);

        printf("\n--------------TEST4 OPEN----------------\n");
        fd = 0;
        fd = OS_open("/PEOPLE/AG8T/sample.txt");
        printf("FD: %d\n", fd);

} 
