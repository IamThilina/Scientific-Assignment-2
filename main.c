#include <stdio.h>

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

void setMatElementAt(int row, int column, int *values, int * columnIndexes, int *rowPointers, int newValue, int *NoOfNonZeroElements, int size){

    int offset;
    int rowPointer = *(rowPointers+(row-1));

    if(rowPointer == -1){ // adding to complete zero row

        int increment = row;
        while(rowPointer == -1){
            rowPointer = *(rowPointers+ increment++);
        }

        offset = (rowPointer-1); // 1 to counter the way arrays are indexed, 1 to go back to previous row

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
            //updating values and columnIndexes
            for (int i = *NoOfNonZeroElements; i > offset; i--) {
                *(columnIndexes+i) = *(columnIndexes +i-1);
                *(values+i) = *(values+i-1);
            }
            *(columnIndexes+offset) = column;
            *(values+offset) = newValue;
            *NoOfNonZeroElements++;

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

                    //updating the rowPointers
                    for (int j = row; j <= size; ++j) {
                        if(*(rowPointers+j) != -1){
                            *(rowPointers+j) += 1;
                        }
                    }
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

void convertToCCSFromCRS(int *values, int *columnIndexes, int *rowPointers, int *rowIndexes, int *columnPointers, int size){

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
}

int main() {

    const int N = 5;
    int NoOfNonZeroElements = 7;

    int mat[5][5] = {
            {1, 0, 0, 2, 0},
            {0, 0, 0, 4, 0},
            {0, 0, 0, 0, 0},
            {0, 8, 0, 0, 16},
            {0, 32, 0, 0, 64}
    };

    /*
     * Value Array
     * 1 2 4 8 16 32 64
     * 1 2 4 2017 8 16 32 64
     *
     * Column Indexes
     * 1 4 4 2 5 2 5
     * 1 4 4 3 2 5 2 5
     *
     * RowPointers
     * 1 3 -1 4 6 8
     * 1 3 4 5 7 9
     *
     * Row Indexes
     * 1 4 5 1 2 4 5
     *
     * Column Pointers
     * 1 2 -1 4 6 8
     * */

    int values[NoOfNonZeroElements];
    int columnIndexes[NoOfNonZeroElements];
    int rowPointers[N];
    int rowIndexes[NoOfNonZeroElements];
    int columnPointers[N];

    printf("\nOriginal Sparse Matrix\n");
    print2DArray(&mat, N);

    printf("\n\nConverting To CRS Format\n");
    convertToCRS(&mat, &values, &columnIndexes, &rowPointers, N);

    printf("\nValues Array\n");
    print1DArray(&values, NoOfNonZeroElements);
    printf("\n\nColumn Indexes Array\n");
    print1DArray(&columnIndexes, NoOfNonZeroElements);
    printf("\n\nRow Pointers\n");
    print1DArray(&rowPointers, N+1);
    printf("\n\n");

    printf("Element at (3,3) is %d",getMatElementAt(3, 3, &values, &columnIndexes, &rowPointers));

    printf("\n\nSetting Element at (3,3) to 2017");
    setMatElementAt(3, 3, &values, &columnIndexes, &rowPointers, 2017, &NoOfNonZeroElements, N);
    printf("\n\n");
    printf("Element at (3,3) is %d",getMatElementAt(1, 3, &values, &columnIndexes, &rowPointers));

    printf("\n======================\n");
    printf("\nValues\n");
    print1DArray(&values, NoOfNonZeroElements);
    printf("\nColumn Indexes\n");
    print1DArray(&columnIndexes, NoOfNonZeroElements);
    printf("\nRow Pointers\n");
    print1DArray(&rowPointers, N+1);


    printf("\n\nConverting To CCS Format");
    convertToCCSFromCRS(&values, &columnIndexes, &rowPointers, &rowIndexes, &columnPointers, N);

    printf("\n\nRow Indexes Array\n");
    print1DArray(&rowIndexes, NoOfNonZeroElements);
    printf("\n\nColumn Pointers\n");
    print1DArray(&columnPointers, N);

    return 0;
}