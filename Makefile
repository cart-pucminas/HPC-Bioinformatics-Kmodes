
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CUDA_HOME=/home/guilherme.castro
    endif
    ifeq ($(UNAME_S),Darwin)
        #COMPILE_FLAGS=-ccbin=clang-omp++ -Xcompiler -fopenmp
        #LINK_FLAGS=-ccbin=clang-omp++ -Xcompiler -fopenmp
endif
export LANG=en_US.UTF-8
export LC_ALL=en_US
CUDA_HOME?=/Developer/NVIDIA/CUDA-7.5

COMPILE_FLAGS+=${COMMON_FLAGS}
COMMON_FLAGS=-g -std=c1x -Wall -o
CUDA_FLAGS=--ptxas-options=-v -arch=sm_30 -o
#MPI_FLAGS=-I$(shell mpicc --showme:incdirs) $(addprefix -L,$(shell mpicc --showme:libdirs)) -fopenmp
MPI_FLAGS= -fopenmp

CUDAC=${CUDA_HOME}/bin/nvcc
ICC=/opt/intel/bin/icc
CC=gcc
MPIC=mpicc

BASES = athaliana celegans Rattusnovergicus Musmusculus HomoSapiens gallus Drosophila
BASE_OBJC = objc/sequence.o objc/io.o
OBJC = objc/main.o ${BASE_OBJC}
# sour/opt/intel/bin/compilervars.sh intel64
all: kmodes kmodes_cuda kmodes_mpi kmodes_openmp

kmodes_cuda: kmodes sequence_cuda
	${CUDAC} -c ${CUDA_FLAGS}  objc/$@.o  src/$@.cu --shared
	${CUDAC} ${CUDA_FLAGS} bin/kmodes-cuda objc/$@.o objc/sequence_cuda.o objc/io.o objc/main.o
kmodes_mpi: kmodes emmc_power
	${MPIC} -c -fopenmp $(MPI_FLAGS) -lmpi  ${COMPILE_FLAGS} objc/$@.o src/$@.c
	${MPIC} -c -fopenmp $(MPI_FLAGS) -lmpi  ${COMPILE_FLAGS} objc/main_mpi.o src/main.c -D USE_MPI -D EMMC_POWER_MEASUREMENT
	${MPIC} -fopenmp $(MPI_FLAGS) -lmpi  ${LINK_FLAGS} -o bin/kmodes-mpi objc/emmc_power.o objc/$@.o objc/main_mpi.o ${BASE_OBJC}
kmodes_openmp: kmodes power
	${ICC} -c src/$@.c -qopt-report -fopenmp $(COMPILE_FLAGS) objc/$@.o
	${ICC} -c src/sequence.c $(COMPILE_FLAGS) objc/sequence_icc.o
	${ICC} -fopenmp objc/power.o objc/$@.o objc/sequence_icc.o objc/io.o objc/main.o -o bin/kmodes-openmp
kmodes: create_objc_dir main io sequence
	${CC} -c src/$@.c ${COMPILE_FLAGS} objc/$@.o
	${CC} -o bin/kmodes objc/kmodes.o ${OBJC}
emmc_power:
	${CC} -c src/$@.c -pthread ${COMPILE_FLAGS} objc/$@.o
power:
	${CC} -c src/$@.c -pthread ${COMPILE_FLAGS} objc/$@.o
sequence_cuda:
	${CUDAC} -c src/sequence.c ${CUDA_FLAGS} objc/$@.o
sequence:
	${CC} -c src/$@.c ${COMPILE_FLAGS} objc/$@.o
io:
	${CC} -c src/$@.c ${COMPILE_FLAGS} objc/$@.o
main:
	${CC} -c src/$@.c ${COMPILE_FLAGS} objc/$@.o
create_objc_dir:
	mkdir -p objc
extract_input:
	for base in $(BASES) ; do \
		COMMAND="tar -zxkvf input/$$base/clusters.tar.gz -C input/$$base/ --exclude ._* "; \
		echo $$COMMAND ; \
		$$COMMAND 2>/dev/null; true ;\
	done ; \

clean:
	for base in $(BASES) ; do \
		num=1 ; while [[ $$num -le 10 ]] ; do \
			rm -f input/$$base/cluster$$num.out; \
			rm -f input/$$base/train$$num; \
			rm -f input/$$base/testPos$$num; \
			rm -f input/$$base/testNeg$$num; \
			rm -f input/$$base/test$$num; \
			rm -f input/$$base/results$$num; \
			rm -f input/$$base/predictions$$num; \
			rm -f input/$$base/model$$num; \
			rm -f input/$$base/log$$num.txt; \
			rm -f input/$$base/cluster$$num.out; \
			rm -f input/$$base/cluster$$num ; \
			rm -f input/$$base/cluster$$num.out.binseq; \
			((num = num + 1)) ; \
		done ; \
			rm -f $$input/$$base/resultsFinal; \
			rm -f $$input/$$base/positivos.binseq; \
			rm -f $$input/$$base/negativos.binseq; \
			rm -f $$input/$$base/negativos; \
			rm -f $$input/$$base/positivos; \
	done ; \
	rm -f ${OBJC} objc/kmodes.o objc/kmodes_cuda.o bin/kmodes*
