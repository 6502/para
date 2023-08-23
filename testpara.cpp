#include <stdio.h>
#include <atomic>
#include <time.h>
#include <unistd.h>
#include "para.h"

int main(int argc, const char *argv[]) {
    std::atomic_int gj{0};
    Para::run([&](int i, int n){
        printf("%i: Starting\n", i);
        for (int j=gj++; j<10; j=gj++) {
            sleep(1);
            printf("%i: Done (%i)\n", i, j);
        }
        printf("%i: Stopping\n", i);
    });
    printf("-------------------\n");
    Para::run([&](int i, int n){
        printf("%i: Starting\n", i);
        for (int j=10*i/n,j1=10*(i+1)/n; j<j1; j++) {
            sleep(1);
            printf("%i: Done (%i)\n", i, j);
        }
        printf("%i: Stopping\n", i);
    });

    return 0;
}
