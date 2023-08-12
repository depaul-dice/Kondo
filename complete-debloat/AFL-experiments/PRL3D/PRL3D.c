#include "Benchmark.h"
#include "string.h"
FILE* fd, *fd_in, *fd_use;

int main(int argc, char *argv[]){
    // int sW = atoi(argv[1]);
    // int sH = atoi(argv[2]);
    // int sZ = atoi(argv[3]);
    fd = fopen("./output.txt", "a");
    int sW, sH, sZ;
    scanf("%d\n", &sW);
    scanf("%d\n", &sH);
    scanf("%d\n", &sZ);

    if( (sW >= 3 && sW <= 30 && sH >= 0 && sH <= 20 && sZ >= 0 && sZ <= 30) ){
        // printf("%d, %d\n", sW, sH);
        _Peripheral3D(sW, sH, sZ, fd);
    }
    return 0;
}