#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "kernel/matrix_ops.h"
#include "benchmark_common.h"

int main(int argc, char **argv) {
    return bench_run(argc, argv, 0);
}
