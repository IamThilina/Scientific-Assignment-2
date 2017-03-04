//
// Created by thilina on 2/25/17.
//

#include <stdio.h>
#include <stdlib.h>
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

void printPageRanks(double *array, int size){

    for (int j = 0; j < size; ++j) {
        printf(" page-%d => %f\n", j+1,*array++);
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

    const int N = 1000;
    double computedEignValue = 0.0;
    int i,j, iteration = 0;
    double *probabilityTransitionMat;
    double temp,aggregatedRelativeChange = 1;
    double *pageRankVector, *pageRankVectorCopy;
    double ERROR_SENSITIVITY = 0.0000000001;

    probabilityTransitionMat = (double*) calloc(N*N, sizeof(double)); // allocate memory and initialize to zero
    pageRankVector = (double*) malloc(N*sizeof(double));
    pageRankVectorCopy = (double*) malloc(N*sizeof(double));

    initializeProbabilityTransitionMatrix(probabilityTransitionMat, N);
    initializePageRankVector(pageRankVectorCopy, N);

    printf("\n\n***********************  Probability Transition Matrix  *********************\n\n");
    print2DArray(probabilityTransitionMat, N);
    printf("\n\n*****************************************************************************\n");

    while (aggregatedRelativeChange > ERROR_SENSITIVITY){
        computedEignValue = 0;
        iteration++;
        printf("\n*****************************  Iteration No : %d  ***************************\n",iteration);
        for (i = 0; i < N; ++i) {
            temp = 0.0;
            for (j = 0; j < N; ++j) {
                temp += *(pageRankVectorCopy+j) * *((probabilityTransitionMat+j*N)+i);
            }
            *(pageRankVector+i) = temp;
            // taking infinite norm
            if(fabs(temp)>computedEignValue)
                computedEignValue = fabs(temp);
        }

        aggregatedRelativeChange = 0.0;
        for (i = 0; i < N; ++i) {
            *(pageRankVector+i) = *(pageRankVector+i)/computedEignValue;  // normalizing the vector
            aggregatedRelativeChange += fabs(*(pageRankVector+i) - *(pageRankVectorCopy+i)); // calculate the relative change between last two iterations
            *(pageRankVectorCopy+i) = *(pageRankVector+i);
        }

        printf("\nPage Ranks\n");
        print1DArray(pageRankVector, N);
        printf("\n\nAggregated Relative Cahnge\n");
        printf("%f",aggregatedRelativeChange);
        printf("\n*****************************************************************************\n");
    }

    printf("\n\n*****************************  Final Page Ranks  ****************************\n\n");
    printPageRanks(pageRankVector, N);
    printf("\n\n*****************************************************************************\n");

    /*releasing allocated memory blocks*/
    free(probabilityTransitionMat);
    free(pageRankVector);
    free(pageRankVectorCopy);

    return 0;
}