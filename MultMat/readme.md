# Sequential and Parallel

## Compile

```make matrix_multiplication```

### Run 

```./matrix_multiplication -a 20 -b 40 -c 40 -d 50 -n 5```

For help and how to use flags run:

```./matrix_multiplication --help```

# Distributed

## Compile 

```mpicc -o mpi -Wall -fopenmp matrix_util.c mpi_scatter_gather.c solver.c strassen.c mpi_utils.c metrics.c -lm```

## Run

```mpiexec --hostfile ./hostfile -n 5 ./mpi -a 25 -b 30 -c 30 -d 20 -n 4```
