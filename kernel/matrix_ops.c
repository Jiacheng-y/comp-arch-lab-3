#include "matrix_ops.h"


float **matmul(float **A, float **B, int A_rows, int A_cols, int B_rows, int B_cols)
{
    // TODO: Implement matrix multiplication
    if (A_cols != B_rows) {
        return NULL;
    }
    float **output = malloc(A_rows * sizeof(float *));
    for (int i = 0; i < A_rows; i++){
        output[i] = malloc(B_cols * sizeof(float)); 
        for (int j = 0; j < B_cols; j++){
            output[i][j] = 0.00f;
            for (int k = 0; k < A_cols; k++){
                output[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return output;
}

float **matmul_sparse(float **A, float **B, int A_rows, int A_cols, int B_rows, int B_cols)
{
    // TODO: Implement sparse matrix multiplication
    if (A_cols != B_rows) {
        return NULL;
    }
    float **output = malloc(A_rows * sizeof(float *));
}
