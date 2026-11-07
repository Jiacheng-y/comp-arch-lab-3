CC = gcc
CFLAGS = -I. -lm

HDF5_FLAGS = -I/usr/include/hdf5/serial -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5
COMMON_HDRS = ./utils/data_utils.h ./kernel/conv.h ./kernel/matrix_ops.h ./kernel/linear.h ./kernel/functional.h 
COMMON_SRC = ./utils/data_utils.c ./kernel/conv.c ./kernel/functional.c ./kernel/matrix_ops.c ./kernel/linear.c 

# Unity test framework
UNITY_FILES = ./tests/unity/unity.c
TEST_FILES = ./tests/all_tests.c ./tests/test_conv.c ./tests/test_functional.c ./tests/test_linear.c ./tests/test_matrix_ops.c
TEST_EXECUTABLES = $(patsubst %.c,%,$(TEST_FILES))

BENCH_CPU ?= 0
BENCH_CORE ?= S0-C0

BENCH_TOPLEV ?= /classes/ece5755/pmu-tools/toplev.py
BENCH_TIME ?= /usr/bin/time

BENCH_CASES ?= 1 2 3 4 5
BENCH_REPS ?= 100
BENCH_SPARSE_REPS ?= $(BENCH_REPS)
BENCH_TRIALS ?= 3

# Preserve the original flags unless explicitly overridden.
BENCH_CFLAGS ?= $(CFLAGS)

BENCH_RUN ?= $(shell date +%Y%m%d_%H%M%S_%N)
BENCH_OUT ?= results/$(BENCH_RUN)

BENCH_PROGRAMS = benchmark_matmul benchmark_sparse

BENCH_DEPS = \
	perf/benchmark_common.h \
	kernel/matrix_ops.c \
	kernel/matrix_ops.h

# Performance
MATMUL_TARGETS = matmul_naive matmul_sparse

.PHONY: test
test: all_tests

.PHONY: all_tests
all_tests: 
	$(CC) -o tests/$@ $(TEST_FILES) $(UNITY_FILES) $(COMMON_SRC) $(CFLAGS) $(HDF5_FLAGS) 
	./tests/$@

.PHONY : $(MATMUL_TARGETS)
$(MATMUL_TARGETS):
	$(CC) -o $@ ./perf/$@.c $(COMMON_SRC) $(CFLAGS) $(HDF5_FLAGS)
	/classes/ece5755/pmu-tools/toplev.py --core S0-C0 -l1 -v --no-desc taskset -c 0 ./$@

.PHONY: clean
clean:
	rm -f $(TEST_EXECUTABLES) $(GRADING_EXECUTABLES) $(GRADING_TESTS_OUTPUT) $(MATMUL_TARGETS)

.PHONY: bench_build bench_check bench_time bench_l2 bench_clean bench_force

# Prevent concurrent measurements.
.NOTPARALLEL:

bench_build: $(BENCH_PROGRAMS)

benchmark_matmul: perf/benchmark_matmul.c $(BENCH_DEPS) bench_force
	$(CC) $(BENCH_CFLAGS) -I. -std=c11 perf/benchmark_matmul.c kernel/matrix_ops.c -lm -o $@

benchmark_sparse: perf/benchmark_sparse.c $(BENCH_DEPS) bench_force
	$(CC) $(BENCH_CFLAGS) -I. -std=c11 perf/benchmark_sparse.c kernel/matrix_ops.c -lm -o $@

bench_check: bench_build
	@set -eu; for input in $(BENCH_CASES); do \
		./benchmark_sparse $$input 1 --verify; \
	done

bench_time: bench_build
	@mkdir -p "$(BENCH_OUT)"
	@printf 'input,implementation,trial,repetitions,real_s,user_s,sys_s,maxrss_kb\n' > "$(BENCH_OUT)/runtime.csv"
	@set -eu; for input in $(BENCH_CASES); do \
		trial=1; while [ $$trial -le $(BENCH_TRIALS) ]; do \
			if [ $$((trial % 2)) -eq 1 ]; then order='matmul sparse'; else order='sparse matmul'; fi; \
			for implementation in $$order; do \
				reps=$(BENCH_REPS); \
				if [ "$$implementation" = sparse ]; then reps=$(BENCH_SPARSE_REPS); fi; \
				"$(BENCH_TIME)" -a -o "$(BENCH_OUT)/runtime.csv" \
					-f "$$input,$$implementation,$$trial,$$reps,%e,%U,%S,%M" \
					-p taskset -c $(BENCH_CPU) ./benchmark_$$implementation $$input $$reps \
					> "$(BENCH_OUT)/input$${input}_$${implementation}_trial$${trial}.txt"; \
			done; trial=$$((trial + 1)); \
		done; \
	done
	@awk -F, 'BEGIN { OFS=","; print "input,implementation,trial,repetitions,real_s,real_s_per_call" } NR>1 { print $$1,$$2,$$3,$$4,$$5,$$5/$$4 }' "$(BENCH_OUT)/runtime.csv" > "$(BENCH_OUT)/runtime_per_call.csv"
	@echo "Timing results: $(BENCH_OUT)"

bench_l2: bench_build
	@mkdir -p "$(BENCH_OUT)"
	@set -eu; for input in $(BENCH_CASES); do \
		trial=1; while [ $$trial -le $(BENCH_TRIALS) ]; do \
			if [ $$((trial % 2)) -eq 1 ]; then order='matmul sparse'; else order='sparse matmul'; fi; \
			for implementation in $$order; do \
				reps=$(BENCH_REPS); \
				if [ "$$implementation" = sparse ]; then reps=$(BENCH_SPARSE_REPS); fi; \
				stem="$(BENCH_OUT)/input$${input}_$${implementation}_trial$${trial}"; \
				"$(BENCH_TOPLEV)" --core $(BENCH_CORE) -l2 -v --force-cpu spr --no-desc -x, \
					-o "$$stem.toplev.csv" \
					taskset -c $(BENCH_CPU) ./benchmark_$$implementation $$input $$reps \
					> "$$stem.txt" 2> "$$stem.stderr.log"; \
			done; trial=$$((trial + 1)); \
		done; \
	done
	@echo "Level 2 profiling results: $(BENCH_OUT)"

bench_clean:
	rm -f $(BENCH_PROGRAMS)