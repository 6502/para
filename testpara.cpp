#include <stdio.h>
#include <atomic>
#include <time.h>
#include <unistd.h>
#include "para.h"

int main(int argc, const char *argv[]) {
    Para para;
    std::atomic_int gi{0};
    std::atomic_int tid{0};
    para.run([&](){
        int id = tid++;
        printf("%i: Starting\n", id);
        for (int i=gi++; i<10; i=gi++) {
            sleep(1);
            printf("%i: Done (%i)\n", id, i);
        }
        printf("%i: Stopping\n", id);
    });
    printf("-------------------\n");
    gi = 0;
    tid = 0;
    para.run([&](){
        int id = tid++;
        printf("%i: Starting\n", id);
        for (int i=gi++; i<10; i=gi++) {
            sleep(1);
            printf("%i: Done (%i)\n", id, i);
        }
        printf("%i: Stopping\n", id);
    });

    return 0;
}