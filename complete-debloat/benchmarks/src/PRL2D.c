#include "Benchmark.h"
#include "string.h"

int main(int argc, char *argv[]){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);

    if( (sW >= 3 && sW <= 30 && sH >= 10 && sH <= 35) ){
        // printf("%d, %d\n", sW, sH);
        _Peripheral(argc, argv);
    }
    return 0;
}