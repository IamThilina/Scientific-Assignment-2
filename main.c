#include <stdio.h>
#include <stdlib.h>

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

    int rowPointer = *(rowPointers+(row-1));

    if(rowPointer == -1) // complete zero row
        return 0;

    int offset = (rowPointer-1);
    columnIndexes += offset;
    int columnIndex = *columnIndexes;

    if(columnIndex > column){
        //printf("\nMORE");
        return 0;
    } else if (columnIndex < column){
        //printf("LESS\n");
        while (columnIndex < column){
            columnIndex = *++columnIndexes;
            offset++;
            if(offset == *(rowPointers+row)-1 || offset == *(rowPointers+row+1)-1 || columnIndex > column) { // beginning of next row
                return 0;
            }
        }
        return  *(values+offset);
    } else{
        //printf("\nOn The Dot");
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
            rowPointer = *(rowPointers+ increment++);
        }

        offset = (rowPointer-1); // 1 to counter the way arrays are indexed, 1 to go back to previous row

        columnIndexes = realloc(columnIndexes, *NoOfNonZeroElements+1); //reallocating memory for columnIndexes
        values = realloc(values, *NoOfNonZeroElements+1); //reallocating memory for values
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
            } else{
                *(rowPointers+j) = offset+1;
            }
        }

    } else{  // adding to non-complete zero row

        offset = (rowPointer-1);
        int columnIndex = *(columnIndexes+offset);

        if(columnIndex > column){ // overriding a zero element

            columnIndexes = realloc(columnIndexes, *NoOfNonZeroElements+1); //reallocating memory for columnIndexes
            values = realloc(values, *NoOfNonZeroElements+1); //reallocating memory for values
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
                columnIndex = *(columnIndexes+ ++offset);
                if(offset == *(rowPointers+row)-1 || offset == *(rowPointers+row+1)-1 || columnIndex > column) {
                    // First 2 conditions : Offset is at next row.Element must be the last non zero element of the row. overriding a zero
                    // Last conditions : there are more elements in the required row. overriding a zero
                    columnIndexes = realloc(columnIndexes, *NoOfNonZeroElements+1); //reallocating memory for columnIndexes
                    values = realloc(values, *NoOfNonZeroElements+1); //reallocating memory for values
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
                } else{
                    continue;
                }
            }
        } else{
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

    const int N = 5;
    int i,j;

    //Q1 - d,e,f,g,h
    int NoOfNonZeroElements = 7;
    int mat[5][5] = {
            {1, 0, 0, 2, 0},
            {0, 0, 0, 4, 0},
            {0, 0, 0, 0, 0},
            {0, 8, 0, 0, 16},
            {0, 32, 0, 0, 64}
    };


    //Q1 - d,e,f,g,h
    /*
     * Value Array
     * 1 2 4 8 16 32 64
     * 2016 2 2016 4 2016 8 2016 16 32 2016
     *
     * Column Indexes
     * 1 4 4 2 5 2 5
     * 1 4 2 4 3 2 4 5 2 5
     *
     * RowPointers
     * 1 3 -1 4 6 8
     * 1 3  5 6 9 11
     *
     * Row Indexes
     * 1 2 4 5 3 1 2 4 4 5
     *
     * Column Pointers
     * 1 2 5 6 9 11
     * */


    //Q1 - d,e,f,g,h
    int *values, *columnIndexes, *rowPointers, *rowIndexes, *columnPointers;

    //Q1 - d,e,f,g,h
    values = (int*) malloc(NoOfNonZeroElements*sizeof(int));
    columnIndexes = (int*) malloc(NoOfNonZeroElements*sizeof(int));
    rowPointers = (int*) malloc(N*sizeof(int));
    rowIndexes = (int*) malloc(NoOfNonZeroElements*sizeof(int));
    columnPointers = (int*) malloc(N* sizeof(int));

    // Q1 - d,e,f,g,h
    printf("\n*************************  Original Sparse Matrix  **************************\n");
    print2DArray(&mat, N);
    printf("\n*****************************************************************************\n");

    printf("\n************************* Converting To CRS Format  *************************\n");
    convertToCRS(&mat, values, columnIndexes, rowPointers, N);

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

    printf("\nRow Indexes Array\n");
    print1DArray(rowIndexes, NoOfNonZeroElements);
    printf("\n\nColumn Pointers\n");
    print1DArray(columnPointers, N+1);
    printf("\n\n*****************************************************************************\n");

    return 0;
}