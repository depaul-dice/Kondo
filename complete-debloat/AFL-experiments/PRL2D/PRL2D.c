#include "Benchmark.h"
#include "string.h"

FILE* fd, *fd_in, *fd_use;


int main(int argc, char *argv[]){
    // int sW = atoi(argv[1]);
    // int sH = atoi(argv[2]);
    fd = fopen("./output.txt", "a");
    int sW, sH;
    scanf("%d\n", &sW);
    scanf("%d\n", &sH);

    if( (sW >= 3 && sW <= 30 && sH >= 10 && sH <= 35) ){
        // printf("%d, %d\n", sW, sH);
        _Peripheral(sW, sH, fd);
    }
    return 0;
}