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
        printf("BEFORE FD: %d count: %d\n", fd, fdCount);
        fd = OS_open("/CONGRATSTXT");
        print_dirEnt(&fdTable[fd]);
        printf("AFTER FD: %d count: %d\n", fd, fdCount);

        printf("\n--------------TEST2 OPEN----------------\n");
        fd = 0;
        printf("BEFORE FD: %d count: %d\n", fd, fdCount);
        fd = OS_open("/PEOPLE/AG8T/GATE-C~1TXT");
        print_dirEnt(&fdTable[fd]);
        printf("AFTER FD: %d count: %d\n", fd, fdCount);

        printf("\n--------------TEST3 OPEN----------------\n");
        fd = 0;
        printf("BEFORE FD: %d count: %d\n", fd, fdCount);
        fd = OS_open("/PEOPLE/AG8T");
        printf("AFTER FD: %d count: %d\n", fd, fdCount);

        printf("\n--------------TEST4 OPEN----------------\n");
        fd = 0;
        printf("BEFORE FD: %d count: %d\n", fd, fdCount);
        fd = OS_open("/PEOPLE/AG8T/sample.txt");
        printf("AFTER FD: %d count: %d\n", fd, fdCount);

} 
