// #include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>

#define FileName "/home/rohan/Documents/Research/HDF5SyntheticBenchmark/Synthetic10.h5"

int X = 128;
int Y = 128;

enum AccessPattern{ Peripheral, LeftDiagonEdges, RightDiagonEdges, LeftDiagonSteps, Hole,  Quit};

void _Peripheral(int argc, char** argv){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);
    
    // printf("TOP SECTION:\n");
    for (int j = 0; j < sH; j++){
        for(int k =0; k<X; k++){
            printf("%d,%d\n",k, j);
        }
    }   
    // printf("BOTTOM SECTION:\n");
    for (int j = Y- sH; j < Y; j++){
        for(int k =0; k<X; k++){
            printf("%d,%d\n",k, j);
        }
    }
    // printf("LEFT SECTION:\n");
    for (int j = sH; j < Y-sH; j++){
        for(int k =0; k<sW; k++){
            printf("%d,%d\n",k, j);
        }
    }
    // printf("RIGHT SECTION:\n");
    for (int j = sH; j < Y-sH; j++) {
        for(int k = X-sW; k < X; k++){
            printf("%d,%d\n",k, j);
        }
    }
}

void _Peripheral3D(int argc, char** argv){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);
    int sZ = atoi(argv[3]);
    
    for (int i = 0; i < sZ; i++){
        // printf("TOP SECTION:\n");
        for (int j = 0; j < sH; j++){
            for(int k =0; k<X; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }   
        // printf("BOTTOM SECTION:\n");
        for (int j = Y- sH; j < Y; j++){
            for(int k =0; k<X; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
        // printf("LEFT SECTION:\n");
        for (int j = sH; j < Y-sH; j++){
            for(int k =0; k<sW; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
        // printf("RIGHT SECTION:\n");
        for (int j = sH; j < Y-sH; j++) {
            for(int k = X-sW; k < X; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
    }
}

void _LeftDiagonEdges(int argc, char** argv){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);
    // printf("TOP RIGHT:\n");
    for (int j = 0; j < sH; j++){
        for(int k = X - sW; k < X; k++){
            printf("%d,%d\n",k, j);
        }
    }
        
    // printf("BOTTOM LEFT:\n");
    for (int j = Y - sH; j < Y; j++) {
        for(int k =0; k<sW; k++){
            printf("%d,%d\n",k, j);
        }
    }
}

void _LeftDiagonEdges3D(int argc, char** argv){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);
    int sZ = atoi(argv[3]);

    // printf("TOP RIGHT:\n");
    for (int i = 0; i < sZ; i++){
        for (int j = 0; j < sH; j++){
            for(int k = X - sW; k < X; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
            
        // printf("BOTTOM LEFT:\n");
        for (int j = Y - sH; j < Y; j++) {
            for(int k =0; k<sW; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
    }
}

void _RightDiagonEdges(int argc, char** argv){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);

// printf("TOP Left:\n");
    for (int j = 0; j < sH; j++){
        for(int k = 0; k < sW; k++){
            printf("%d,%d\n",k, j);
        }
    }
        
    // printf("BOTTOM Right:\n");
    for (int j = Y - sH; j < Y; j++) {
        for(int k = X - sW; k < X; k++){
            printf("%d,%d\n",k, j);
        }
    }
}

void _RightDiagonEdges3D(int argc, char** argv){
    int sW = atoi(argv[1]);
    int sH = atoi(argv[2]);
    int sZ = atoi(argv[3]);

    // printf("TOP Left:\n");
    for (int i = 0; i < sZ; i++){
        for (int j = 0; j < sH; j++){
            for(int k = 0; k < sW; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
            
        // printf("BOTTOM Right:\n");
        for (int j = Y - sH; j < Y; j++) {
            for(int k = X - sW; k < X; k++){
                printf("%d,%d,%d\n",k, j, i);
            }
        }
    }
}

// void _LeftDiagonSteps();

// void _Hole();

// hid_t getOpenFile();
// hid_t getDataset(hid_t file, char* DatasetName);
// hid_t getDataspace(hid_t dataset);