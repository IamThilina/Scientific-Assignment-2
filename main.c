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

int main() {

    const int N = 5;
    const int NoOfNonZeroElements = 7;

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
     *
     * Column Indexes
     * 1 4 4 2 5 2 5
     *
     * RowPointers
     * 1 3 -1 4 6
     * */

    int values[NoOfNonZeroElements];
    int columnIndexes[NoOfNonZeroElements];
    int rowPointers[N];

    int count = 0;
    int i,j;
    int rowPointerFound = 0;

    print2DArray(&mat, N);

    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            if(mat[i][j] > 0){
                values[count] = mat[i][j];
                columnIndexes[count] = j+1;
                if(!rowPointerFound){
                    rowPointers[i] = count+1;
                    rowPointerFound = 1;
                }
                count++;
            }
        }
        if(!rowPointerFound) // complete zero elements row
            rowPointers[i] = -1;
        rowPointerFound = 0;
    }

    printf("Values Array\n");
    print1DArray(&values, NoOfNonZeroElements);
    printf("\nColumn Indexes Array\n");
    print1DArray(&columnIndexes, NoOfNonZeroElements);
    printf("\nRow Pointers\n");
    print1DArray(&rowPointers, N);

    return 0;
}