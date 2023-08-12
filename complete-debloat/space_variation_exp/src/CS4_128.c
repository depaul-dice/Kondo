#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


/*
    This program reads a slab of data and then takes a userdefined step in every dimension
    and then reads the same sized slab again until it reaches the end in any dimension
*/


// Size of the chunks
#define CHUNK_X 20
#define CHUNK_Y 20

// Size of the file
#define DATASET_X 100
#define DATASET_Y 100

FILE* fd, *fd_in, *fd_use;

// These will be user inputs for simplicity defining as global variables

void dummy_select_hyperslab(int offset0, int offset1, int count0, int count1, int stepX, int stepY, int readSizeX, int readSizeY) {
  // print_offsets(offset0, offset1, fd);
}

int main(int argc, char **argv){
    fd = fopen("./output.txt", "a");
    fd_in = fopen("./inputs.txt", "a");
    fd_use = fopen("./inputs_used.txt", "a");
    int stepx, stepy, readSizeX, readSizeY, xMax, yMax;

    readSizeX = 1;
    readSizeY = 1;

    // scanf("%d\n", &stepx);
    // scanf("%d\n", &stepy);
    stepx = atoi(argv[1]);
    stepy = atoi(argv[2]);
    fprintf(fd_in, "%d %d\n", stepx, stepy);

    // printf("%d %d %d %d\n", stepx, stepy, readSizeX, readSizeY);

    xMax = 128;
    yMax = 128;
    // zMax = 50;
    
    int     dims_out[2];  

    int count[2];        /* Size of data to read from the dataset */
    int offset[2];      /* Offset of the data to read from the dataset */
    dims_out[0] = xMax;
    dims_out[1] = yMax;
    // dims_out[2] = zMax;
    offset[0] = 0;
    offset[1] = 0;
    // offset[2] = 0;

    // Define the size to read from the dataset
    // count[0] = readSizeZ;
    count[0] = readSizeX;
    count[1] = readSizeY;
    // int z = (unsigned long) dims_out[0];
    int x = (unsigned long) dims_out[0];
    int y = (unsigned long) dims_out[1];

    int i = 0;
    /* Raghavan added the variable i above. I feel every loop must have a iteration count variable for Daikon to be able to infer useful variants. */
    if(!(stepx==0 && stepy==0) && ((stepx>=stepy)&&(stepx<=2*stepy) || (6*stepx>=stepy)&&(2*stepx<=stepy)) && (stepx>=0 && stepy>=0)){
      fprintf(fd_use, "%d %d\n", stepx, stepy);
      while((offset[0]+readSizeX<=(x)) && (offset[1]+readSizeY<=(y)) ){
        // dummy_select_hyperslab(offset[0], offset[1], count[0], count[1], stepx, stepy, readSizeX, readSizeY);
        printf("%d,%d\n", offset[0], offset[1]);
          // actual read call here
          // offset[0]+= stepz;
          offset[0]+= stepx;
          offset[1]+= stepy;

    i++;
      }
    }
}
