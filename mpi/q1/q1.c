/* Comp 4510 - MPI Array Addition Example
 * This code sums up the elements of a vector of integers, distributing
 * the work across multiple processes.
 * Note: for this to work correctly, N (below) must be divisible by
 * the number of processes being used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

/* Constants */

#define N (1 << 4)    // (2^4) size of vector (must be divisible by number of processes)
#define PRINT_VECS 1  // flag so we can turn off printing when N is large
#define MAX_RAND 100  // max value of elements generated for array

/* Prototypes */
void init_vec(int *vec, int len);
void print_vec(const char *label, int *vec, int len);
int sum_chunk(int *vec, int low_index, int high_index);

/* Functions */

// Fills a vector with random integers in the range [0, MAX_RAND)
void init_vec(int *vec, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        vec[i] = rand() % MAX_RAND;
    }    
}

// Prints the given vector to stdout
void print_vec(const char *label, int *vec, int len)
{
#if PRINT_VECS
    printf("%s", label);
    
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%d ", vec[i]);
    }
    printf("\n\n");
#endif
}

// Sums up the chunk of the vector from low_index up to
// (but not including) high_index
int sum_chunk(int *vec, int low_index, int high_index)
{
    int i;
    int my_sum = 0;
    for (i = low_index; i < high_index; i++)
    {
        my_sum += vec[i];
    }

    return my_sum;
}

int main(int argc, char *argv[])
{
    // Declare process-related vars (note: each process has its own copy)
    // and initialize MPI
    int my_rank;
    int num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //grab this process's rank
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); //grab the total num of processes

    // Declare arrays for data and results
    // Note: each process has its own copy
    int vec[N]; // input vector
    double start_time; // use these for timing
    double stop_time;

    // Calculat the number of elements each process will sum
    int chunk_size = N / num_procs;
    
    // Create local vec for each process
    int local_vec[chunk_size];

    // Process 0 fills the vector and starts the timer
    if (!my_rank)
    {
        printf("Number of processes: %d\n", num_procs);
        printf("N: %d\n", N);
        printf("Chunk size: %d\n", chunk_size);
        srand(time(NULL));
        init_vec(vec, N);
        print_vec("Initial vector:\n", vec, N);
        start_time = MPI_Wtime(); // can use this function to grab a
                                  // timestamp (in seconds)
    }

    // Scatter part of the vec array to the different processes
    MPI_Scatter(vec, chunk_size, MPI_INT, &local_vec, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    
    // Compute the sum of our chunk & print it
    int my_sum = sum_chunk(local_vec, 0, chunk_size);
    printf("Result from process %d: %d\n", my_rank, my_sum);

	// Have MPI sum the values from the different processes
	int total;
	MPI_Reduce(&my_sum, &total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Process 0 prints out the final total
    // (and timing stats)
    if (!my_rank)
    {
        stop_time = MPI_Wtime();
        printf("Final result from process %d: %d\n", my_rank, total);
        printf("Total time (sec): %f\n", stop_time - start_time);
    }

    // Shutdown MPI (important - don't forget!)
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}
