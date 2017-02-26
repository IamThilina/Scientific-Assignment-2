//
// Created by thilina on 2/19/17.
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

void generateRandomSparseMat(int *mat, int size){

    int nnz = size*size*0.1;  // number of non-zero elements must be added
    int count = 0; // number of non-zero elements already added
    int randomRow, randomColumn;

    srand(time(NULL));
    while(count < nnz){
        randomRow = ((int)rand()%size);
        randomColumn = ((int)rand()%size);
        /*distinct number sequence is too large so that same (randomRow,randomColumn) combination will not be generated*/
        *((mat+randomRow*size)+randomColumn) = ((int)rand()%MAXIMUM_NUMBER)+1;
        count++;
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

void sparseMatrixAddition(int *matAValues, int *matAColumnIndexes, int *matARowPointers, int matANNZ,
                          int *matBValues, int *matBColumnIndexes, int *matBRowPointers, int matBNNZ,
                          int *matCValues, int *matCColumnIndexes, int *matCRowPointers, int *matCNNZ, int size){

    int matARowBeginIndex, matBRowBeginIndex,
            matAOffset, matBOffset,
            matAColumnIndex, matBColumnIndex,
            matANextRowBeginIndex, matBNextRowBeginIndex,
            matAOffsetToNextRow, matBOffsetToNextRow;

    int k, priorMatCNNZ;

    for (int i = 0; i < size; ++i) {
        matARowBeginIndex = *(matARowPointers+i);
        matBRowBeginIndex = *(matBRowPointers+i);
        priorMatCNNZ = *matCNNZ;

        if(matARowBeginIndex == -1 && matBRowBeginIndex == -1){ // both matrices have complete zero rows
            //just update the rowPointers

        } else if(matARowBeginIndex == -1){ // matA row is complete zero row. copy matB row to matC

            matBNextRowBeginIndex = *(matBRowPointers+i+1);
            k=i+2;
            while(matBNextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matBNextRowBeginIndex = *(matBRowPointers + k++);
            }

            matBOffset = matBRowBeginIndex-1;
            matBOffsetToNextRow = matBNextRowBeginIndex - 1;
            matBColumnIndex = *(matBColumnIndexes+matBOffset);

            while(matBOffset < matBOffsetToNextRow){ // copy all non-zero elements of matrix B to matC
                *(matCValues + *matCNNZ) = *(matBValues+matBOffset);
                *(matCColumnIndexes + *matCNNZ) = matBColumnIndex;
                *matCNNZ += 1;
                matBOffset++;
                matBColumnIndex = *(matBColumnIndexes+matBOffset);
            }
        } else if (matBRowBeginIndex == -1){ // matB row is complete zero row. copy matA row to matC

            matANextRowBeginIndex = *(matARowPointers+i+1);
            k=i+2;
            while(matANextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matANextRowBeginIndex = *(matARowPointers + k++);
            }

            matAOffset = matARowBeginIndex-1;
            matAOffsetToNextRow = matANextRowBeginIndex - 1;
            matAColumnIndex = *(matAColumnIndexes+matAOffset);

            while(matAOffset<matAOffsetToNextRow){ // copy all remaining non-zero elements of matrix A to matC
                *(matCValues + *matCNNZ) = *(matAValues+matAOffset);
                *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                *matCNNZ += 1;
                matAOffset++;
                matAColumnIndex = *(matAColumnIndexes+matAOffset);
            }
        } else{ // both rows are non zero

            matANextRowBeginIndex = *(matARowPointers+i+1);
            k=i+2;
            while(matANextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matANextRowBeginIndex = *(matARowPointers + k++);
            }

            matBNextRowBeginIndex = *(matBRowPointers+i+1);
            k=i+2;
            while(matBNextRowBeginIndex<0){ // find the pointer to next non complete zero row
                matBNextRowBeginIndex = *(matBRowPointers + k++);
            }

            // handle null rows

            matAOffset = matARowBeginIndex-1;
            matBOffset = matBRowBeginIndex-1;
            matAOffsetToNextRow = matANextRowBeginIndex - 1;
            matBOffsetToNextRow = matBNextRowBeginIndex - 1;
            matAColumnIndex = *(matAColumnIndexes+matAOffset);
            matBColumnIndex = *(matBColumnIndexes+matBOffset);

            while (matAOffset<matAOffsetToNextRow || matBOffset < matBOffsetToNextRow){

                if(matAOffset<matAOffsetToNextRow && matBOffset < matBOffsetToNextRow){ // non zero elements remaining in both matrix row i
                    if(matAColumnIndex < matBColumnIndex){ // just copy matA Element at (i,matAColumnIndex) to matC
                        *(matCValues + *matCNNZ) = *(matAValues+matAOffset);
                        *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                        *matCNNZ += 1;
                        matAOffset++;
                        matAColumnIndex = *(matAColumnIndexes+matAOffset);
                    } else if(matAColumnIndex > matBColumnIndex){  // just copy matB Element at (i,matBColumnIndex) to matC
                        *(matCValues + *matCNNZ) = *(matBValues+matBOffset);
                        *(matCColumnIndexes + *matCNNZ) = matBColumnIndex;
                        *matCNNZ += 1;
                        matBOffset++;
                        matBColumnIndex = *(matBColumnIndexes+matBOffset);
                    } else{ // Add matA Element at (i,matAColumnIndex) and matB Element at (i,matBColumnIndex) to matC
                        *(matCValues + *matCNNZ) = *(matAValues+matAOffset) + *(matBValues+matBOffset);
                        *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                        *matCNNZ += 1;
                        matAOffset++;
                        matAColumnIndex = *(matAColumnIndexes+matAOffset);
                        matBOffset++;
                        matBColumnIndex = *(matBColumnIndexes+matBOffset);
                    }
                } else if(matAOffset<matAOffsetToNextRow){  // non zero elements remaining only in matrix A row i
                    while(matAOffset<matAOffsetToNextRow){ // copy all remaining non-zero elements of matrix A to matC
                        *(matCValues + *matCNNZ) = *(matAValues+matAOffset);
                        *(matCColumnIndexes + *matCNNZ) = matAColumnIndex;
                        *matCNNZ += 1;
                        matAOffset++;
                        matAColumnIndex = *(matAColumnIndexes+matAOffset);
                    }
                } else{ // non zero elements remaining only in matrix B row i
                    while(matBOffset < matBOffsetToNextRow){ // copy all remaining non-zero elements of matrix B to matC
                        *(matCValues + *matCNNZ) = *(matBValues+matBOffset);
                        *(matCColumnIndexes + *matCNNZ) = matBColumnIndex;
                        *matCNNZ += 1;
                        matBOffset++;
                        matBColumnIndex = *(matBColumnIndexes+matBOffset);
                    }
                }
            }
        }


        // updating rowPointers
        if(priorMatCNNZ<*matCNNZ){ // not a complete zero row
            *(matCRowPointers+i) = priorMatCNNZ+1;
        } else{  // complete zero row
            *(matCRowPointers+i) = -1;
        }

    }

    // readjust the memory allocation for matC associated arrays
    //matCValues = realloc(matCValues, *matCNNZ);
    //matCColumnIndexes = realloc(matCColumnIndexes, *matCNNZ);

}

void denseMatrixAddition(int *matA, int *matB, int *matC, int size){
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            *((matC+i*size)+j) = *((matA+i*size)+j) + *((matB+i*size)+j);
        }
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
    int *matA, *matB, *matC;
    int *matAValues, *matAColumnIndexes, *matARowPointers, matANNZ, *matBValues, *matBColumnIndexes,
        *matBRowPointers, matBNNZ, *matCValues, *matCColumnIndexes, *matCRowPointers, matCNNZ;

   /*Timing Variables*/
    struct timespec t0, t1, t2;
    unsigned long sec, nsec;
    float comp_time, totalTimeForSparseAlgo = 0.0, totalTimeForDenseAlgo = 0.0; // in milli seconds

    /*Elapsed Time Calculation Code*/
    /*GET_TIME(t0);
    // do initializations, setting-up etc
    GET_TIME(t1);
    // do computation
    GET_TIME(t2);
    comp_time = elapsed_time_msec(&t1, &t2, &sec, &nsec);*/
    printf("Enter The Matrix Size : ");
    scanf("%d", &N);
    printf("\nEnter The Number Of Test Cases : ");
    scanf("%d", &NUM_OF_TEST_CASES);

    for (i = 0; i < NUM_OF_TEST_CASES; ++i) {
        matANNZ = 0;
        matBNNZ = 0;
        matCNNZ = 0;

        //for input matrices
        matA = (int*) calloc(N*N, sizeof(int)); // allocate memory and initialize to zero
        matB = (int*) calloc(N*N, sizeof(int)); // allocate memory and initialize to zero
        // to store the resulting matrix from dense algorithm
        matC = (int*) malloc(N*N*sizeof(int));
        // for sparse algorithm
        matARowPointers = (int*) malloc((N+1)*sizeof(int));
        matBRowPointers = (int*) malloc((N+1)*sizeof(int));
        matCRowPointers = (int*) malloc((N+1)*sizeof(int));
        matANNZ = N*N*0.1;
        matBNNZ = N*N*0.1;

        printf("\n*****************************************************************************\n");
        printf("\n*******************************  Test Case %d ********************************\n",i+1);
        printf("\n*****************************************************************************\n");
        printf("\n*************************  Original Sparse Matrix-A **************************\n");
        generateRandomSparseMat(matA, N);
        // allocate memory for matA CRS associated arrays
        matAValues = (int*) malloc(matANNZ*sizeof(int));
        matAColumnIndexes = (int*) malloc(matANNZ*sizeof(int));
        print2DArray(matA, N);
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

        printf("\n*************************  Original Sparse Matrix-B *************************\n");
        generateRandomSparseMat(matB, N);
        // allocate memory for matB CRS associated arrays
        matBValues = (int*) malloc(matBNNZ*sizeof(int));
        matBColumnIndexes = (int*) malloc(matBNNZ*sizeof(int));
        // allocate memory for matC CRS associated arrays
        matCValues = (int*) malloc((matANNZ + matBNNZ)*sizeof(int));
        matCColumnIndexes = (int*) malloc((matANNZ + matBNNZ)*sizeof(int));
        print2DArray(matB, N);
        printf("\n*****************************************************************************\n");
        printf("\n*********************** Converting Mat-B To CRS Format  *********************\n");
        convertToCRS(matB, matBValues, matBColumnIndexes, matBRowPointers, N);
        printf("\nValues Array\n");
        print1DArray(matBValues, matBNNZ);
        printf("\n\nColumn Indexes Array\n");
        print1DArray(matBColumnIndexes, matBNNZ);
        printf("\n\nRow Pointers\n");
        print1DArray(matBRowPointers, N+1);
        printf("\n\n*****************************************************************************\n");

        printf("\n*********************** Running Dense Matrix Addition  **********************\n");
        GET_TIME(t1);
        denseMatrixAddition(matA, matB, matC, N);
        GET_TIME(t2);
        comp_time = elapsed_time_msec(&t1, &t2, &sec, &nsec);
        totalTimeForDenseAlgo += comp_time;
        printf("\nResulting Matrix-C\n");
        print2DArray(matC, N);
        printf("\n\nElapsed Time For Dense Algorithm\n");
        printf("%f\n",comp_time);
        printf("\n\n*****************************************************************************\n");

        printf("\n*********************** Running Sparse Matrix Addition  **********************\n");
        GET_TIME(t1);
        sparseMatrixAddition(matAValues, matAColumnIndexes, matARowPointers, matANNZ,
                             matBValues, matBColumnIndexes, matBRowPointers, matBNNZ,
                             matCValues, matCColumnIndexes, matCRowPointers, &matCNNZ, N);
        GET_TIME(t2);
        comp_time = elapsed_time_msec(&t1, &t2, &sec, &nsec);
        totalTimeForSparseAlgo += comp_time;
        printf("\nResulting CRS Format\n");
        printf("\nValues Array\n");
        print1DArray(matCValues, matCNNZ);
        printf("\n\nColumn Indexes Array\n");
        print1DArray(matCColumnIndexes, matCNNZ);
        printf("\n\nRow Pointers\n");
        print1DArray(matCRowPointers, N+1);
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
        free(matC);
        free(matCValues);
        free(matCColumnIndexes);
        free(matCRowPointers);
    }

    printf("Time For Dense Algo : %f\n",totalTimeForDenseAlgo/NUM_OF_TEST_CASES);
    printf("Time For Sparse Algo : %f\n",totalTimeForSparseAlgo/NUM_OF_TEST_CASES);

    return 0;
}