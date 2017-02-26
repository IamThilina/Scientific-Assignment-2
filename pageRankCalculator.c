//
// Created by thilina on 2/25/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAXIMUM_NUMBER 10
#define MAX_LINKS_FROM_PAGES 5

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

    int nnz; // number of non-zero values in a column
    int count; // number of non-zero element already added
    int randomRow;

    srand(time(NULL));
    for (int i = 0; i < size; ++i) {
        nnz = ((int)rand()%MAX_LINKS_FROM_PAGES)+5; // nnz take a value 5 to 20
        //printf("%f\n",(double)1/nnz);
        count = 0;
        while (count < nnz){
            randomRow = ((int)rand()%size);
            //printf("%d ",randomRow);
            *((mat+randomRow*size)+i) = (double)1/nnz;
            count++;
        }
        //printf("\n");
    }
}

void initializePageRankVector(double *vect, int size){
    //memset(vect, 1, size*sizeof(double));
    for (int i = 0; i < size; ++i) {
        *(vect+i) = (double) 1/size;
    }
}

int main() {

    const int N = 10;
    int i,j,converged=0, iteration=0;
    double *probabilityTransitionMat;
    double temp, delta = 0.001;
    double *pageRankVector, *pageRankVectorCopy;

    //printf("\n**********************************  Matrix  *********************************\n\n");
    probabilityTransitionMat = (double*) calloc(N*N, sizeof(double)); // allocate memory and initialize to zero
    pageRankVector = (double*) malloc(N*sizeof(double));
    pageRankVectorCopy = (double*) malloc(N*sizeof(double));

    initializeProbabilityTransitionMatrix(probabilityTransitionMat, N);
    initializePageRankVector(pageRankVectorCopy, N);

    while (!converged){
        iteration++;
        printf("\n*****************************  Iteration No : %d  ***************************\n",iteration);
        for (i = 0; i < N; ++i) {
            temp = 0.0;
            for (j = 0; j < N; ++j) {
                temp += *(pageRankVectorCopy+j) * *((probabilityTransitionMat+j*N)+i);
            }
            *(pageRankVector+i) = temp;
        }
        printf("\nPage Ranks\n");
        print1DArray(pageRankVector, N);

        // checking if converged
        for (int k = 0; k < N; ++k) {
            if(fabs(*(pageRankVector+k) - *(pageRankVectorCopy + k)) < delta) {
                if (k == N-1)
                    converged = 1;
                else
                    continue;
            }
            else
                break;
        }
        memcpy(pageRankVectorCopy,pageRankVector, N);
        printf("\n\n*****************************************************************************\n");
    }

    /*releasing allocated memory blocks*/
    free(probabilityTransitionMat);
    free(pageRankVector);
    free(pageRankVectorCopy);

    return 0;
}