#include "matrix_ops.h"


float **matrix_new(int rows, int cols)
{
    if (rows <= 0 || cols <= 0) {
        return NULL;
    }

    /*
     * Allocate the array of row pointers.
     * calloc initializes every pointer to NULL.
     */
    float **matrix = calloc((size_t)rows, sizeof *matrix);

    if (matrix == NULL) {
        return NULL;
    }

    /*
     * Allocate each row.
     * calloc initializes every matrix element to zero.
     */
    for (int i = 0; i < rows; i++) {
        matrix[i] = calloc((size_t)cols, sizeof **matrix);

        if (matrix[i] == NULL) {
            matrix_free(matrix, rows);
            return NULL;
        }
    }

    return matrix;
}

void matrix_free(float **matrix, int rows)
{
    if (matrix == NULL) {
        return;
    }

    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }

    free(matrix);
}

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
    
    if (A == NULL || B == NULL ||
        A_rows <= 0 || A_cols <= 0 ||
        B_rows <= 0 || B_cols <= 0 ||
        A_cols != B_rows) {
        return NULL;
    }

    /*
     * Initialize pointers so cleanup is safe even if an allocation fails before all arrays have been created.
     */
    int *A_row_ptr = NULL;
    int *A_col_idx = NULL;
    float *A_values = NULL;

    int *B_row_ptr = NULL;
    int *B_col_idx = NULL;
    float *B_values = NULL;

    float **result = NULL;

    int A_nnz = 0;
    int B_nnz = 0;

    /*
     * Convert A and B into CSR arrays.
     */

    A_row_ptr = calloc(
        (size_t)A_rows + 1,
        sizeof(int)
    );

    B_row_ptr = calloc(
        (size_t)B_rows + 1,
        sizeof(int)
    );

    if (A_row_ptr == NULL || B_row_ptr == NULL) {
        goto allocation_failed;
    }

    /*
     * Count A's nonzeros.
     *
     * After processing row i, A_nnz is the cumulative number of nonzeros through that row.
     */
    for (int i = 0; i < A_rows; i++) {
        for (int j = 0; j < A_cols; j++) {
            if (A[i][j] != 0.0f) {
                A_nnz++;
            }
        }

        A_row_ptr[i + 1] = A_nnz;
    }

    /* Count B's nonzeros in the same way. */
    for (int i = 0; i < B_rows; i++) {
        for (int j = 0; j < B_cols; j++) {
            if (B[i][j] != 0.0f) {
                B_nnz++;
            }
        }

        B_row_ptr[i + 1] = B_nnz;
    }

    /*
     * Store A's nonzero values and their column indices.
     */
    int position = 0;

    for (int i = 0; i < A_rows; i++) {
        for (int j = 0; j < A_cols; j++) {
            if (A[i][j] != 0.0f) {
                A_values[position] = A[i][j];
                A_col_idx[position] = j;

                position++;
            }
        }
    }

    /* Store B's nonzero values and their column indices. */
    position = 0;

    for (int i = 0; i < B_rows; i++) {
        for (int j = 0; j < B_cols; j++) {
            if (B[i][j] != 0.0f) {
                B_values[position] = B[i][j];
                B_col_idx[position] = j;

                position++;
            }
        }
    }

    /*
     * Allocate the output and multiply.
     *
     * The output remains a normal float ** matrix.
     */
    result = calloc((size_t)A_rows, sizeof(float *));

    if (result == NULL) {
        goto allocation_failed;
    }

    for (int i = 0; i < A_rows; i++) {
        result[i] = calloc((size_t)B_cols, sizeof(float));

        if (result[i] == NULL) {
            goto allocation_failed;
        }
    }

    /*
     * For each nonzero A[i][k], visit the nonzeros in row k of B.
     */
    for (int i = 0; i < A_rows; i++) {

        for (int p = A_row_ptr[i];
             p < A_row_ptr[i + 1];
             p++) {

            int k = A_col_idx[p];
            float A_ik = A_values[p];

            for (int q = B_row_ptr[k];
                 q < B_row_ptr[k + 1];
                 q++) {

                int j = B_col_idx[q];
                float B_kj = B_values[q];

                result[i][j] += A_ik * B_kj;
            }
        }
    }

    /*
     * The CSR arrays are temporary.
     * The caller receives and owns result.
     */
    free(A_row_ptr);
    free(A_col_idx);
    free(A_values);

    free(B_row_ptr);
    free(B_col_idx);
    free(B_values);

    return result;

allocation_failed:

    /*
     * Release any output rows allocated before failure.
     * Unallocated row pointers are NULL because of calloc.
     */
    if (result != NULL) {
        for (int i = 0; i < A_rows; i++) {
            free(result[i]);
        }

        free(result);
    }

    free(A_row_ptr);
    free(A_col_idx);
    free(A_values);

    free(B_row_ptr);
    free(B_col_idx);
    free(B_values);

    return NULL;
}
