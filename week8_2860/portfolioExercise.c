//
// Starting code for the portfolio exercise. Some required routines are included in a separate
// file (ending '_extra.h'); this file should not be altered, as it will be replaced with a different
// version for assessment.
//
// Compile as normal, e.g.,
//
// > gcc -o portfolioExercise portfolioExercise.c
//
// and launch with the problem size N and number of threads p as command line arguments, e.g.,
//
// > ./portfolioExercise 12 4
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "portfolioExercise_extra.h"        // Contains routines not essential to the assessment.


typedef struct {
    int N;
    float **M;
    float *u;
    float *v;
    int start_matrix_row;
    int end_matrix_row;
    float local_vector_sum;
} Args; 

void* worker(void *arg){
    Args *a = (Args*) arg; 
    for (int row = a->start_matrix_row; row < a-> end_matrix_row; row++ ) {
        a->v[row] = 0.0f;

        for( int col=0; col<a->N; col++ )
            a->v[row] += a->M[row][col] * a->u[col];
    }
    return NULL; 
}

void *dot_product_worker(void *arg ) {
    Args *a = (Args*) arg; 
    a->local_vector_sum = 0.0f;
    for( int i=a->start_matrix_row; i<a->end_matrix_row; i++ ) 
        a->local_vector_sum+= a->v[i]*a->v[i];

    return NULL; 
}



//
// Main.
//
int main( int argc, char **argv )
{
    //
    // Initialisation and set-up.
    //

    // Get problem size and number of threads from command line arguments.
    int N, nThreads;
    if( parseCmdLineArgs(argc,argv,&N,&nThreads)==-1 ) return EXIT_FAILURE;

    // Initialise (i.e, allocate memory and assign values to) the matrix and the vectors.
    float **M, *u, *v;
    if( initialiseMatrixAndVector(N,&M,&u,&v)==-1 ) return EXIT_FAILURE;

    // For debugging purposes; only display small problems (e.g., N=8 and nThreads=2 or 4).
    if( N<=12 ) displayProblem( N, M, u, v );

    // Start the timing now.
    struct timespec startTime, endTime;
    clock_gettime( CLOCK_REALTIME, &startTime );

    //
    // Parallel operations, timed.
    //
    float dotProduct = 0.0f;        // You should leave the result of your calculation in this variable.

    // Step 1. Matrix-vector multiplication Mu = v.

    pthread_t threads[nThreads];
    Args args [nThreads];

    for (int i = 0; i < nThreads; i++) {
        args[i].N = N;
        args[i].M = M;
        args[i].u = u;
        args[i].v = v; 
        args[i].start_matrix_row = i*(N/nThreads);
        args[i].end_matrix_row = (i+1)*(N/nThreads); 
        pthread_create(&threads[i], NULL, worker, &args[i]);
    
    }
        
    for (int i = 0 ; i <nThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    

    // After completing Step 1, you can uncomment the following line to display M, u and v, to check your solution so far.
    if( N<=12 ) displayProblem( N, M, u, v );

    // Step 2. The dot product of the vector v with itself.

    for (int i = 0; i < nThreads; i++) {
        args[i].N = N;
        args[i].v = v; 
        args[i].start_matrix_row = i*(N/nThreads);
        args[i].end_matrix_row = (i+1)*(N/nThreads); 
        pthread_create(&threads[i], NULL, dot_product_worker, &args[i]);
    
    }

    for (int i = 0 ; i <nThreads; i++){
        pthread_join(threads[i], NULL);
    }
        
    
    for (int i = 0; i < nThreads; i++) {
        dotProduct += args[i].local_vector_sum;
    }



    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of parallel calculation: %f\n", dotProduct );

    //
    // Check against the serial calculation.
    //

    // Output final time taken.
    clock_gettime( CLOCK_REALTIME, &endTime );
    double seconds = (double)( endTime.tv_sec + 1e-9*endTime.tv_nsec - startTime.tv_sec - 1e-9*startTime.tv_nsec );
    printf( "Time for parallel calculations: %g secs.\n", seconds );

    // Step 1. Matrix-vector multiplication Mu = v.
    for( int row=0; row<N; row++ )
    {
        v[row] = 0.0f;              // Make sure the right-hand side vector is initially zero.

        for( int col=0; col<N; col++ )
            v[row] += M[row][col] * u[col];
    }

    // Step 2: The dot product of the vector v with itself
    float dotProduct_serial = 0.0f;
    for( int i=0; i<N; i++ ) dotProduct_serial += v[i]*v[i];

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of the serial calculation: %f\n", dotProduct_serial );

    //
    // Clear up and quit.
    //
    freeMatrixAndVector( N, M, u, v );

    return EXIT_SUCCESS;
}