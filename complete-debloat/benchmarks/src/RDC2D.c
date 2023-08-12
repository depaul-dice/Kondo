#include "Benchmark.h"
#include "string.h"

int main(int argc, char *argv[]){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);

    if( (sW >= 3 && sW <= 40 && sH >= 10 && sH <= 50) ){
        _RightDiagonEdges(argc, argv);
    }
}