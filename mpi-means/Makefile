COMMOM_ARGS= -Wall -std=c99 -fopenmp -g 
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
		ARGS= -lmpi  -lm $(COMMOM_ARGS)
    	COMPILER=gcc
    endif
    ifeq ($(UNAME_S),Darwin)
    	MPI_COMPILE_FLAGS = $(shell mpicc --showme:compile)
		MPI_LINK_FLAGS = $(shell mpicc --showme:link)
		ARGS= $(MPI_COMPILE_FLAGS) $(COMMOM_ARGS)
        COMPILER=clang-omp
endif

all:
	$(COMPILER) $(ARGS) io.h io.c -c
	$(COMPILER) $(ARGS) kmeans.h kmeans.c -c
	$(COMPILER) $(MPI_LINK_FLAGS) $(ARGS) main.c io.o kmeans.o -o mpi-means
clean:
	rm *.o  *.gch
	rm mpi-means
