//
// Created by thilina on 2/20/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for clock_gettime( )
#include <errno.h> // for perror( )

#define MAXIMUM_NUMBER 10

#define GET_TIME(x) if (clock_gettime(CLOCK_MONOTONIC, &(x)) < 0) \
{ perror("clock_gettime( ):"); exit(EXIT_FAILURE); }

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

void printColumnMat(int *array, int size){

    for (int j = 0; j < size; ++j) {
        printf(" %d\n", *array++);
    }
}

void generateRandomSparseMat(int *mat, int size){

    int nnz = size*size*0.1;  // number of non-zero elements must be added
    int count = 0; // number of non-zero elements already added
    int randomRow, randomColumn;

    srand(time(NULL));
    while(count < nnz){
        randomRow = ((int)rand()%size);
        randomColumn = ((int)rand()%size);
        if(*((mat+randomRow*size)+randomColumn) == 0) { // if non-zero element was added earlier, skip this
            *((mat + randomRow * size) + randomColumn) = ((int) rand() % MAXIMUM_NUMBER) + 1;
            count++;
        }
    }
}

void generateRandomVector(int *vector, int size){
    srand(time(NULL));
    for (int j = 0; j < size; ++j) {
        *(vector+j) = ((int)rand()%MAXIMUM_NUMBER)+1;
    }
}

void convertToCRS(int *mat, int *values, int *columnIndexes, int *rowPointers, int size){

    int count = 0;
    int i,j;
    int rowPointerFound = 0;

    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            if(*((mat+i*size)+j) > 0){
                *values++ = *((mat+i*size)+j);
                *columnIndexes++ = j+1;
                if(!rowPointerFound){
                    *rowPointers++ = count+1;
                    rowPointerFound = 1;
                }
                count++;
            }
        }
        if(!rowPointerFound) // complete zero elements row
            *rowPointers++ = -1;
        rowPointerFound = 0;
    }
    *rowPointers++ = count+1; // end of columnIndexes
}

void sparseMatrixVectorMultiplication(int *matAValues, int *matAColumnIndexes, int *matARowPointers,
                          int *matBValues, int *matBColumnIndexes, int *matBRowPointers, int *matBNNZ , int *vector,  int size){

    int matARowBeginIndex, matAOffset,
        matAColumnIndex, matANextRowBeginIndex, matAOffsetToNextRow;

    int k, temp;

    for (int i = 0; i < size; ++i) {
        matARowBeginIndex = *(matARowPointers+i);

        if(matARowBeginIndex == -1){ // complete zero row
            *(matBRowPointers+i) = -1;
        }  else{ // non complete zero row

            matANextRowBeginIndex = *(matARowPointers+i+1);
            k=i+2;
            while(matANextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matANextRowBeginIndex = *(matARowPointers + k++);
            }

            matAOffset = matARowBeginIndex-1;
            matAOffsetToNextRow = matANextRowBeginIndex - 1;
            matAColumnIndex = *(matAColumnIndexes+matAOffset);

            temp = 0;
            while (matAOffset<matAOffsetToNextRow){
                temp += *(matAValues+matAOffset) * (*(vector+matAColumnIndex-1));
                matAOffset++;
                matAColumnIndex = *(matAColumnIndexes+matAOffset);
            }
            // updating associated arrays
            if(temp>0){ // not a complete zero row
                *(matBValues + *matBNNZ) = temp;
                *(matBColumnIndexes + *matBNNZ) = 1;
                *(matBRowPointers+i) = *matBNNZ+1;
                *matBNNZ += 1;
            } else{  // complete zero row
                *(matBRowPointers+i) = -1;
            }
        }
    }
}

void denseMatrixVectorMultiplication(int *matA, int *matB, int *vector, int size){
    int temp;
    for (int i = 0; i < size; ++i) {
        temp = 0;
        for (int j = 0; j < size; ++j) {
            temp += *((matA+i*size)+j) * (*(vector+j));
        }
        *(matB+i) = temp;
    }
}

float elapsed_time_msec(struct timespec *begin, struct timespec *end, long *sec, long *nsec) {
    if (end->tv_nsec < begin->tv_nsec) {
        *nsec = 1000000000 - (begin->tv_nsec - end->tv_nsec);
        *sec = end->tv_sec - begin->tv_sec -1;
    }
    else {
        *nsec = end->tv_nsec - begin->tv_nsec;
        *sec = end->tv_sec - begin->tv_sec;
    }
    return (float) (*sec) * 1000 + ((float) (*nsec)) / 1000000;
}

int main() {

    int N,NUM_OF_TEST_CASES;
    int i;
    int *matA, *matB, *vector;
    int *matAValues, *matAColumnIndexes, *matARowPointers, matANNZ,
        *matBValues, *matBColumnIndexes, *matBRowPointers, matBNNZ;

    /*Timing Variables*/
    struct timespec t0, t1, t2;
    unsigned long sec, nsec;
    float comp_time, totalTimeForSparseAlgo = 0.0, totalTimeForDenseAlgo = 0.0; // in milli seconds


    printf("Enter The Matrix Size : ");
    scanf("%d", &N);
    printf("\nEnter The Number Of Test Cases : ");
    scanf("%d", &NUM_OF_TEST_CASES);

    for (i = 0; i < NUM_OF_TEST_CASES; ++i) {
        matANNZ = 0;
        matBNNZ = 0;

        //for input matrices
        matA = (int*) malloc(N*N*sizeof(int));
        vector = (int*) malloc(N*sizeof(int));
        // to store the resulting matrix from dense algorithm
        matB = (int*) malloc(N*N*sizeof(int));

        // for sparse algorithm
        matARowPointers = (int*) malloc((N+1)*sizeof(int));
        matBValues = (int*) malloc(N*sizeof(int));
        matBColumnIndexes = (int*) malloc(N*sizeof(int));
        matBRowPointers = (int*) malloc((N+1)*sizeof(int));
        matANNZ = N*N*0.1;

        printf("\n*****************************************************************************\n");
        printf("\n*******************************  Test Case %d ********************************\n",i+1);
        printf("\n*****************************************************************************\n");
        printf("\n*************************  Original Sparse Matrix-A **************************\n");
        generateRandomSparseMat(matA, N);
        generateRandomVector(vector, N);
        // allocate memory for matA CRS associated arrays
        matAValues = (int*) malloc(matANNZ*sizeof(int));
        matAColumnIndexes = (int*) malloc(matANNZ*sizeof(int));
        print2DArray(matA, N);
        printf("\n*****************************************************************************\n");
        printf("\n**********************************  Vector **********************************\n");
        printColumnMat(vector, N);
        printf("\n*****************************************************************************\n");
        printf("\n********************** Converting Mat-A To CRS Format  **********************\n");
        convertToCRS(matA, matAValues, matAColumnIndexes, matARowPointers, N);
        printf("\nValues Array\n");
        print1DArray(matAValues, matANNZ);
        printf("\n\nColumn Indexes Array\n");
        print1DArray(matAColumnIndexes, matANNZ);
        printf("\n\nRow Pointers\n");
        print1DArray(matARowPointers, N+1);
        printf("\n\n*****************************************************************************\n");

        printf("\n*********************** Running Dense Matrix-Vector Multiplication  **********************\n");
        GET_TIME(t1);
        denseMatrixVectorMultiplication(matA, matB, vector, N);
        GET_TIME(t2);
        comp_time = elapsed_time_msec(&t1, &t2, &sec, &nsec);
        totalTimeForDenseAlgo += comp_time;
        printf("\nResulting Column Matrix\n");
        printColumnMat(matB, N);
        printf("\n\nElapsed Time For Dense Algorithm\n");
        printf("%f\n",comp_time);
        printf("\n\n*****************************************************************************\n");

        printf("\n*********************** Running Sparse Matrix-Vector Multiplication **********************\n");
        GET_TIME(t1);
        sparseMatrixVectorMultiplication(matAValues, matAColumnIndexes, matARowPointers,
                             matBValues, matBColumnIndexes, matBRowPointers, &matBNNZ, vector, N);
        GET_TIME(t2);
        comp_time = elapsed_time_msec(&t1, &t2, &sec, &nsec);
        totalTimeForSparseAlgo += comp_time;
        printf("\nResulting CRS Format\n");
        printf("\nValues Array\n");
        print1DArray(matBValues, matBNNZ);
        printf("\n\nColumn Indexes Array\n");
        print1DArray(matBColumnIndexes, matBNNZ);
        printf("\n\nRow Pointers\n");
        print1DArray(matBRowPointers, N+1);
        printf("\n\nElapsed Time For Sparse Algorithm\n");
        printf("%f\n",comp_time);
        printf("\n****************************  End Of Test Case %d ****************************",i+1);
        printf("\n*****************************************************************************\n");

        /* Deallocate allocated memory */
        free(matA);
        free(matAValues);
        free(matAColumnIndexes);
        free(matARowPointers);
        free(matB);
        free(matBValues);
        free(matBColumnIndexes);
        free(matBRowPointers);
        free(vector);
    }

    printf("Time For Dense Algo : %f\n",totalTimeForDenseAlgo/NUM_OF_TEST_CASES);
    printf("Time For Sparse Algo : %f\n",totalTimeForSparseAlgo/NUM_OF_TEST_CASES);

    return 0;
}