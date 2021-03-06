//
// Created by thilina on 2/23/17.
//

#include <stdio.h>
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
        printf(" %.10f", *array++);
    }
}

int main() {

    const int N = 3;
    const double trueEignValue = 11.000000;
    int i,j,iteration=0;
    double mat[3][3] = {
            {2.0, 3.0, 2.0},
            {10.0, 3.0, 4.0},
            {3.0, 6.0, 1.0}
    };
    double computedEignValue = 0.0;
    double temp, absoluteError,relativeError = 1;
    double eignVector[3] = {0.0, 0.0, 1.0};
    double eignVectorCopy[3] = {0.0, 0.0, 1.0};
    double ERROR_SENSITIVITY = 0.0000000001;

    printf("\n**********************************  Matrix  *********************************\n");
    print2DArray(&mat, N);
    printf("\n*****************************************************************************\n");

    while (relativeError > ERROR_SENSITIVITY){
        computedEignValue = 0;
        iteration++;

        printf("\n*****************************  Iteration No : %d  ***************************\n",iteration);
        for (i = 0; i < N; ++i) {
            temp = 0.0;
            for (j = 0; j < N; ++j) {
                temp += mat[i][j]*eignVectorCopy[j];
            }
            eignVector[i] = temp;
            if(fabs(temp)>computedEignValue)
                computedEignValue = fabs(temp);
        }
        // normalizing eign vector produced in this iteration
        for (i = 0; i < N; ++i) {
            eignVector[i] = eignVector[i]/computedEignValue;
            eignVectorCopy[i] = eignVector[i];
        }

        // error calculation
        absoluteError =fabs(computedEignValue - trueEignValue);
        relativeError = absoluteError/fabs(trueEignValue);
        printf("\nComputed Eign Vector\n");
        print1DArray(&eignVector,N);
        printf("\n\nComputed Eign Value\n");
        printf("%.10f",computedEignValue);
        printf("\n\nAbsolute Error\n");
        printf("%.10f",absoluteError);
        printf("\n\nRelative Error\n");
        printf("%.10f",relativeError);
        printf("\n*****************************************************************************\n");
    }

    return 0;
}