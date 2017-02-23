//
// Created by thilina on 2/23/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIMUM_NUMBER 10

void print2DArray(int *array, int size){

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            printf(" %2d", *((array+i*size)+j));
        }
        printf("\n");
    }
}

void print1DArray(int *array, int size){

    for (int j = 0; j < size; ++j) {
        printf(" %d", *array++);
    }
}

int main() {

    const int N = 3;
    int i,j,temp=0;
    int mat[3][3] = {
            {2, 3, 2},
            {10, 3, 4},
            {3, 6, 1}
    };
    int eignVector[3];
    int eignVectorCopy[3] = {0, 0, 1};

    //mat = (int*) malloc(N*N*sizeof(int));
    //eignVector = (int*) malloc(N*sizeof(int));

    printf("\n**********************************  Matrix  *********************************\n");
    print2DArray(&mat, N);
    printf("\n*****************************************************************************\n");

    for (i = 0; i < N; ++i) {
        temp = 0;
        for (j = 0; j < N; ++j) {
            temp += mat[i][j]*eignVectorCopy[j];
        }
        eignVector[i] = temp;
    }
    memcpy(&eignVector, &eignVectorCopy, sizeof(int)*N);

    printf("\n*******************************  Eign Vector  *******************************\n");
    print1DArray(&eignVector, N);
    printf("\n*****************************************************************************\n");

    return 0;
}