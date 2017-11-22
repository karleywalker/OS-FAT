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

        printf("\n--------------TEST1 CD----------------\n");
        printf("Before CD: %s\n", cwdPath);
        int status = OS_cd("/");
        printf("Status: %d\n", status);
        printf("After CD: %s\n", cwdPath); 

        printf("\n--------------TEST2 CD----------------\n");
        printf("Before CD: %s\n", cwdPath);
        status = OS_cd("/CONGRATSTXT");
        printf("Status: %d\n", status);
        printf("After CD: %s\n", cwdPath); 

        printf("\n--------------TEST3 CD----------------\n");
        printf("Before CD: %s\n", cwdPath);
        status = OS_cd("/PEOPLE");
        if(status != -1)
                printDirEnt(&cwd);
        printf("Status: %d\n", status);
        printf("After CD: %s\n", cwdPath); 

        printf("\n--------------TEST4 CD----------------\n");
        printf("Before CD: %s\n", cwdPath);
        status = OS_cd("AG8T");
        if(status != -1)
                printDirEnt(&cwd);
        printf("Status: %d\n", status);
        printf("After CD: %s\n", cwdPath); 

        printf("\n--------------TEST5 CD----------------\n");
        printf("Before CD: %s\n", cwdPath);
        status = OS_cd("../YYZ5W");
        if(status != -1)
                printDirEnt(&cwd);
        printf("Status: %d\n", status);
        printf("After CD: %s\n", cwdPath); 

        printf("\n--------------TEST6 CD----------------\n");
        printf("Before CD: %s\n", cwdPath);
        status = OS_cd("/PEOPLE/AG8T/GATE-C~1TXT");
        printf("Status: %d\n", status);
        printf("After CD: %s\n", cwdPath); 

} 
