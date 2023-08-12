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

    if( (sW >= 3 && sW <= 40 && sH >= 10 && sH <= 50) ){
        _RightDiagonEdges(sW, sH, fd);
    }
}