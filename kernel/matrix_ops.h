#ifndef MATRIX_OPS_H
#define MATRIX_OPS_H

#include <stdio.h>
#include <stdlib.h>

float **matmul(float **A, float **B, int A_rows, int A_cols, int B_rows, int B_cols);
float **matmul_sparse(float **A, float **B, int A_rows, int A_cols, int B_rows, int B_cols);
void matmul_sparse_inner(float **output, int A_rows, const int *A_row_ptr, const int *A_col_idx, const float *A_values, const int *B_row_ptr, const int *B_col_idx, const float *B_values);
void matmul_inner(float **output, float **A, float **B, int A_rows, int A_cols, int B_rows, int B_cols);

#endif /* MATRIX_OPS_H */