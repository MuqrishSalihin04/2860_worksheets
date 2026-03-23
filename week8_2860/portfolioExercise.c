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
    for( int row = thread _id; row < N; row +=nThreads)
        v[row] = 0.0f; 

        for (int col=0, col < N; col ++){
             v[row] += M[row][col] * u[col];}
    }        
    
    // After completing Step 1, you can uncomment the following line to display M, u and v, to check your solution so far.
     if( N<=12 ) displayProblem( N, M, u, v );

    // Step 2. The dot product of the vector v with itself.



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