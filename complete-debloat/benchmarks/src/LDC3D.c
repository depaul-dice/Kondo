#include "Benchmark.h"
#include "string.h"

int main(int argc, char *argv[]){

    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);
    int sZ = atoi(argv[3]);

    if( (sW >= 3 && sW <= 30 && sH >= 0 && sH <= 20 && sZ >= 0 && sZ <= 30) ){
        _LeftDiagonEdges3D(argc, argv);
    }
    return 0;
}