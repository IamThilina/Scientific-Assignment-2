//
// Created by thilina on 2/19/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void generateRandomSparseMat(int *mat, int size){

    int nnz = size*size*0.1;  // number of non-zero elements must be added
    int count = 0; // number of non-zero elements already added
    int randomRow, randomColumn;

    srand(time(NULL));
    while(count < nnz){
        randomRow = ((int)rand()%size);
        randomColumn = ((int)rand()%size);
        //printf("(%d,%d)\n",randomRow,randomColumn);
        if(*((mat+randomRow*size)+randomColumn) == 0) { // if non-zero element was added earlier, skip this
            *((mat + randomRow * size) + randomColumn) = ((int) rand() % MAXIMUM_NUMBER) + 1;
            count++;
        }
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

int getMatElementAt(int row, int column, int *values, int * columnIndexes, int *rowPointers){

    int rowBeginIndex = *(rowPointers+(row-1));

    if(rowBeginIndex == -1) // complete zero row
        return 0;

    int offset = (rowBeginIndex-1);
    int scanningColumn = *(columnIndexes + offset);

    int nextRowBeginIndex = *(rowPointers+row);
    int k=1;
    while(nextRowBeginIndex<0){ // find the pointer to next non complete zero row
        nextRowBeginIndex = *(rowPointers + row + k++);
    }
    int offsetToNextRow = nextRowBeginIndex - 1;

    if(scanningColumn > column){
        return 0;
    } else if (scanningColumn < column){
        while (scanningColumn < column){
            offset++;
            scanningColumn = *(columnIndexes + offset);
            if(offset == offsetToNextRow || scanningColumn > column) { // beginning of next row or scanning column exceeds searching column
                return 0;
            }
        }
        return  *(values+offset);
    } else{
        return *(values+offset);
    }
}

void setMatElementAt(int row, int column, int *values, int * columnIndexes, int *rowPointers, int newValue,
                     int *NoOfNonZeroElements, int size){

    int offset;
    int temp = *values;
    int rowPointer = *(rowPointers+(row-1));

    if(rowPointer == -1){ // adding to complete zero row

        int increment = row;
        while(rowPointer == -1){
            rowPointer = *(rowPointers+ increment);
            increment++;
        }

        offset = (rowPointer-1);

        //updating values and columnIndexes
        for (int i = *NoOfNonZeroElements; i > offset; i--) {
            *(columnIndexes+i) = *(columnIndexes +i-1);
            *(values+i) = *(values+i-1);
        }
        *(columnIndexes+offset) = column;
        *(values+offset) = newValue;
        *NoOfNonZeroElements += 1;

        //updating the rowPointers
        for (int j = row-1; j <= size; ++j) {
            if(*(rowPointers+j) != -1){
                *(rowPointers+j) += 1;
            } else if(j == row-1){
                *(rowPointers+j) = offset+1;
            } else
                continue;
        }

    } else{  // adding to non-complete zero row

        offset = (rowPointer-1);
        int columnIndex = *(columnIndexes+offset);

        int nextRowBeginIndex = *(rowPointers+row);
        int k=1;
        while(nextRowBeginIndex<0){ // find the pointer to next non complete zero row
            nextRowBeginIndex = *(rowPointers + row + k++);
        }
        int offsetToNextRow = nextRowBeginIndex - 1;

        if(columnIndex > column){ // overriding a zero element

            //updating values and columnIndexes
            for (int i = *NoOfNonZeroElements; i > offset; i--) {
                *(columnIndexes+i) = *(columnIndexes +i-1);
                *(values+i) = *(values+i-1);
            }
            *(columnIndexes+offset) = column;
            *(values+offset) = newValue;
            *NoOfNonZeroElements += 1;

            //updating the rowPointers
            for (int j = row; j <= size; ++j) {
                if(*(rowPointers+j) != -1){
                    *(rowPointers+j) += 1;
                }
            }
        } else if (columnIndex < column){
            while (columnIndex < column){
                offset++;
                columnIndex = *(columnIndexes+ offset);
                if(offset == offsetToNextRow || columnIndex > column) {
                    // First condition : Offset is at next row.Element must be the last non zero element of the row. overriding a zero
                    // Second condition : there are more elements in the required row. overriding a zero
                    for (int i = *NoOfNonZeroElements; i > offset; i--) {
                        *(columnIndexes+i) = *(columnIndexes +i-1);
                        *(values+i) = *(values+i-1);
                    }
                    *(columnIndexes+offset) = column;
                    *(values+offset) = newValue;
                    *NoOfNonZeroElements += 1;

                    //updating the rowPointers
                    for (int j = row; j <= size; ++j) {
                        if(*(rowPointers+j) != -1){
                            *(rowPointers+j) += 1;
                        }
                    }
                    break;
                } else if(columnIndex == column){ //overriding an existing element
                    *(columnIndexes+offset) = column;
                    *(values+offset) = newValue;
                    break;
                } else{ // columnIndex < column
                    continue;
                }
            }
        } else{ //overriding an existing element
            *(values+offset) = newValue;
        }
    }
}

void convertToCCSFromCRS(int *values, int *columnIndexes, int *rowPointers, int *rowIndexes, int *columnPointers,
                         int size){

    int count = 0;
    int scanningColumn,scanningRow;
    int ColumnPointerFound = 0;
    int offset;
    int firstNonZeroColumnFound = 0;
    int OUT_OF_BOUNDS = 0;

    /*for (scanningColumn = 0; scanningColumn < size; ++scanningColumn) {
        for (scanningRow = 0; scanningRow < size; ++scanningRow) {
            offset = *(rowPointers+scanningRow);

            if(offset == -1) // empty row
                continue;

            firstNonZeroColumn = *(columnIndexes+offset-1);
            printf(" %d ",firstNonZeroColumn);
            if(firstNonZeroColumn < scanningColumn+1){
                while(firstNonZeroColumn < scanningColumn) {
                    if(offset == *(rowPointers+scanningRow+1)) { // beginning of the next row
                        OUT_OF_BOUNDS =1;
                        break;
                    }
                    firstNonZeroColumn = *(columnIndexes+offset);
                    offset++;
                }
                if(!OUT_OF_BOUNDS) {
                    *rowIndexes++ = scanningRow + 1;
                    OUT_OF_BOUNDS = 0;
                }
                if(!ColumnPointerFound){
                    count++;
                    *columnPointers++ = count;
                    ColumnPointerFound =1;
                }
            } else if(firstNonZeroColumn > scanningColumn+1){
                continue;
            } else {
                *rowIndexes++ = scanningRow+1;
                if(!ColumnPointerFound){
                    count++;
                    *columnPointers++ = count;
                    ColumnPointerFound =1;
                }
            }
        }
        if(!ColumnPointerFound) {
            *columnPointers++ = -1;
        }
    }*/

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if(getMatElementAt(j+1, i+1, values, columnIndexes, rowPointers) > 0){
                //printf("\nFOUND %d\n at %d,%d", getMatElementAt(j+1, i+1, values, columnIndexes, rowPointers),j+1,i+1);
                count++;
                *rowIndexes++ = j+1;
                if(!firstNonZeroColumnFound){
                    *columnPointers++ = count;
                    firstNonZeroColumnFound = 1;
                }
            }
        }

        if(!firstNonZeroColumnFound)
            *columnPointers++ = -1;
        else
            firstNonZeroColumnFound = 0;
    }
    *columnPointers++ = count+1; // end of columnIndexes
}

int main() {

    int N;
    int i,j;
    int *mat, *values, *columnIndexes, *rowPointers, *rowIndexes, *columnPointers;
    int NoOfNonZeroElements = 0;

    printf("Enter The Matrix Size : ");
    scanf("%d", &N);
    mat = (int*) calloc(N*N, sizeof(int));  // allocate memory and initialize to zero
    rowPointers = (int*) malloc((N+1)*sizeof(int));
    columnPointers = (int*) malloc((N+1)* sizeof(int));
    NoOfNonZeroElements = N*N*0.1;

    printf("\n*************************  Original Sparse Matrix  **************************\n");
    generateRandomSparseMat(mat, N);
    values = (int*) malloc((NoOfNonZeroElements+N)*sizeof(int));
    columnIndexes = (int*) malloc((NoOfNonZeroElements+N)*sizeof(int));
    rowIndexes = (int*) malloc((NoOfNonZeroElements+N)*sizeof(int));
    print2DArray(mat, N);
    printf("\n*****************************************************************************\n");

    printf("\n************************* Converting To CRS Format  *************************\n");
    convertToCRS(mat, values, columnIndexes, rowPointers, N);

    printf("\nValues Array\n");
    print1DArray(values, NoOfNonZeroElements);
    printf("\n\nColumn Indexes Array\n");
    print1DArray(columnIndexes, NoOfNonZeroElements);
    printf("\n\nRow Pointers\n");
    print1DArray(rowPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    printf("\n******************** Setting Diagonal Elements to 2016  *********************\n");
    for (i = 1; i <= N; i++) {
        setMatElementAt(i, i, values, columnIndexes, rowPointers, 2016, &NoOfNonZeroElements, N);
    }

    printf("\nValues Array\n");
    print1DArray(values, NoOfNonZeroElements);
    printf("\n\nColumn Indexes Array\n");
    print1DArray(columnIndexes, NoOfNonZeroElements);
    printf("\n\nRow Pointers\n");
    print1DArray(rowPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    printf("\n************************* Converting To CCS Format  *************************\n");
    convertToCCSFromCRS(values, columnIndexes, rowPointers, rowIndexes, columnPointers, N);

    printf("\nValues Array\n");
    print1DArray(values, NoOfNonZeroElements);
    printf("\n\nRow Indexes Array\n");
    print1DArray(rowIndexes, NoOfNonZeroElements);
    printf("\n\nColumn Pointers\n");
    print1DArray(columnPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    /*releasing allocated memory blocks*/
    /*free(mat);
    free(columnIndexes);
    free(rowPointers);
    free(rowIndexes);
    free(columnPointers);
    free(values);*/

    return 0;
}