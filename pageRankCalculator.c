//
// Created by thilina on 2/25/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXIMUM_NUMBER 10

void print2DArray(double *array, int size){

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            printf(" %2f", *((array+i*size)+j));
        }
        printf("\n");
    }
}

void print1DArray(double *array, int size){

    for (int j = 0; j < size; ++j) {
        printf(" %f", *array++);
    }
}

void initializeProbabilityTransitionMatrix(double *mat, int size){

}

void initializePageRankVector(double *vect, int size){

}

int main() {

    const int N = 1000;
    int i,j,iteration=0;
    double *probabilityTransitionMat;
    double temp, absoluteError,relativeError = 1;
    double *pageRankVector, *pageRankVectorCopy;

    printf("\n**********************************  Matrix  *********************************\n");
    probabilityTransitionMat = (double*) malloc(N*N*sizeof(double));
    pageRankVector = (double*) malloc(N*sizeof(double));
    pageRankVectorCopy = (double*) malloc(N*sizeof(double));

    initializeProbabilityTransitionMatrix(probabilityTransitionMat, N);
    initializePageRankVector(pageRankVectorCopy, N);

    while (iteration < 10){
        iteration++;
        printf("\n*****************************  Iteration No : %d  ***************************\n",iteration);
        for (i = 0; i < N; ++i) {
            temp = 0.0;
            for (j = 0; j < N; ++j) {
                temp += *(pageRankVectorCopy+j) * *((probabilityTransitionMat+j*N)+i);
            }
            *(pageRankVector+i) = temp;
        }
        memcpy(pageRankVectorCopy,pageRankVector, N);
        printf("\nPage Ranks\n");
        print1DArray(pageRankVector, N);
        printf("\n*****************************************************************************\n");
    }

    /*releasing allocated memory blocks*/
    free(probabilityTransitionMat);
    free(pageRankVector);
    free(pageRankVectorCopy);

    return 0;
}