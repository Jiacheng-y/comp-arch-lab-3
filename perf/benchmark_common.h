#ifndef BENCHMARK_COMMON_H
#define BENCHMARK_COMMON_H
#include "kernel/matrix_ops.h"
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* New benchmark helpers only. No original function is redefined. */
static inline void bench_fail(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}
static inline float **bench_alloc(int rows, int cols)
{
    float **a = calloc((size_t)rows, sizeof *a);
    if (!a) bench_fail("Allocation failed");
    for (int i = 0; i < rows; ++i) {
        a[i] = calloc((size_t)cols, sizeof **a);
        if (!a[i]) bench_fail("Allocation failed");
    }
    return a;
}
static inline void bench_free(float **a, int rows)
{
    if (!a) return;
    for (int i = 0; i < rows; ++i) free(a[i]);
    free(a);
}
static inline uint32_t bench_random(uint32_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}
/* Select an exact count of nonzero cells, or dense blocks, without replacement.
 * The five supplied cases are divisible by block and have exact percentages. */
static inline float **bench_generate(int n, int zeros, int block,
                                     uint32_t seed, int *nnz)
{
    int side = n / block;
    int count = side * side;
    int active = count * (100 - zeros) / 100;
    int *positions = malloc((size_t)count * sizeof *positions);
    float **a = bench_alloc(n, n);
    if (!positions) bench_fail("Position allocation failed");
    for (int i = 0; i < count; ++i) positions[i] = i;
    for (int i = count - 1; i > 0; --i) {
        int j = (int)(bench_random(&seed) % (uint32_t)(i + 1));
        int tmp = positions[i]; positions[i] = positions[j]; positions[j] = tmp;
    }
    for (int p = 0; p < active; ++p) {
        int y = (positions[p] / side) * block;
        int x = (positions[p] % side) * block;
        for (int dy = 0; dy < block; ++dy)
            for (int dx = 0; dx < block; ++dx) {
                int v = (int)(bench_random(&seed) % 18) + 1;
                a[y + dy][x + dx] = (v <= 9 ? v : -(v - 9)) / 9.0f;
            }
    }
    *nnz = active * block * block;
    free(positions);
    return a;
}

/* create three plain arrays. Pointer parameters return their addresses. */
static inline void bench_to_csr(float **a, int rows, int cols,
                               int **row_ptr, int **col_idx, float **values)
{
    *row_ptr = calloc((size_t)rows + 1, sizeof **row_ptr);
    *col_idx = NULL;
    *values = NULL;
    if (!*row_ptr) bench_fail("CSR row allocation failed");
    int nnz = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) nnz += a[i][j] != 0.0f;
        (*row_ptr)[i + 1] = nnz;
    }
    if (nnz) {
        *col_idx = malloc((size_t)nnz * sizeof **col_idx);
        *values = malloc((size_t)nnz * sizeof **values);
        if (!*col_idx || !*values) bench_fail("CSR value allocation failed");
    }
    int p = 0;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            if (a[i][j] != 0.0f) {
                (*col_idx)[p] = j;
                (*values)[p++] = a[i][j];
            }
}

static inline long bench_integer(const char *s, long upper)
{
    char *end;
    errno = 0;
    long value = strtol(s, &end, 10);
    if (errno || end == s || *end || value < 1 || value > upper)
        bench_fail("Invalid case or repetition count");
    return value;
}

static inline int bench_run(int argc, char **argv, int sparse)
{
    if (argc != 3 && argc != 4)
        bench_fail("Usage: executable CASE REPEATS [--verify]");
    int verify = argc == 4;
    if (verify && strcmp(argv[3], "--verify")) bench_fail("Unknown option");
    int id = (int)bench_integer(argv[1], 5);
    long repeats = bench_integer(argv[2], 10000000);
    const int sizes[] = {400, 400, 400, 100, 800};
    const int zero_pct[] = {20, 95, 95, 95, 95};
    int n = sizes[id - 1], zeros = zero_pct[id - 1];
    int block = id == 3 ? 20 : 1;
    int an, bn;
    float **a = bench_generate(n, zeros, block, 12345, &an);
    float **b = bench_generate(n, zeros, block, 67890, &bn);
    int *ar = NULL, *ac = NULL, *br = NULL, *bc = NULL;
    float *av = NULL, *bv = NULL;
    if (sparse || verify) {
        /* Conversion occurs once, before the repetition loop. */
        bench_to_csr(a, n, n, &ar, &ac, &av);
        bench_to_csr(b, n, n, &br, &bc, &bv);
    }
    if (verify) {
        /* Separate correctness mode: do not time/profile this mode. */
        float **expected = matmul(a, b, n, n, n, n);
        float **actual = bench_alloc(n, n);
        matmul_sparse_inner(actual, n, ar, ac, av, br, bc, bv);
        if (!expected) bench_fail("Dense multiplication failed");
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (!isfinite(actual[i][j]) ||
                    fabsf(actual[i][j] - expected[i][j]) >
                    1e-4f + 1e-5f * fabsf(expected[i][j]))
                    bench_fail("FAIL: sparse result differs from dense result");
        bench_free(expected, n); bench_free(actual, n);
        printf("PASS: case %d, all %d output elements compared\n", id, n * n);
    } else {
        volatile double checksum = 0;
        for (long r = 0; r < repeats; ++r) {
            float **c = bench_alloc(n, n);
            if (sparse) {
                matmul_sparse_inner(c, n, ar, ac, av, br, bc, bv);
            } else { 
                matmul_inner(c, a, b, n, n, n, n);
            }
            if (!c) bench_fail("Multiplication failed");
            checksum += c[r % n][(r / n) % n];
            bench_free(c, n);
        }
        printf("implementation=%s case=%d n=%d repeats=%ld A_zeros_pct=%.6f "
               "B_zeros_pct=%.6f checksum=%.9g\n",
               sparse ? "sparse" : "matmul", id, n, repeats,
               100.0 * (1.0 - (double)an / n / n),
               100.0 * (1.0 - (double)bn / n / n), 
               (double)checksum);
    }
    free(ar); free(ac); free(av); free(br); free(bc); free(bv);
    bench_free(a, n); bench_free(b, n);
    return 0;
}
#endif
