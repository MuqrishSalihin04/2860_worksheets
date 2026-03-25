//
// Performs the Jacobi iteration using pthreads. 
//
// This is the specimen answer to the worksheet question.
//
// Compile as normal, e.g.,
//
// > gcc -o parallelJacobi parallelJacobi.c
//
// (on some systems you may need to add '-lm' just after gcc to include the maths library),
// and launch with the problem size, the number of iterations to perform, and the number of
// threads, e.g.,
//
// > ./parallelJacobi 10000 100 4
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>


//
// Functions and struct used by threads.
//

// Struct for passing arguments to threads. For simplicity, re-use the same one for both operations.
typedef struct {
    int N;
    int start;
    int end;
    float *X;
    float *X_copy;
} threadArgs_t;

// Copy from X to X_copy.
void* copyX_perThread( void* arguments )
{
    threadArgs_t *args = (threadArgs_t*) arguments;

    for( int i=args->start; i<args->end; i++ )
        (args->X_copy)[i] = (args->X)[i];

    return NULL;
}

// Update X using values from X_copy.
void* updateX_perThread( void* arguments )
{
    threadArgs_t *args = (threadArgs_t*) arguments;

    for( int i=args->start; i<args->end; i++ )
    {
        if( i==0 || i==args->N-1 ) continue;
        (args->X)[i] = 0.5f * ( (args->X_copy)[i-1] + (args->X_copy)[i+1] );
    }

    return NULL;
}



//
// Functions used by the main thread.
//

// Initialise the solution vector to its initial state.
void initialiseX( float *X, int N )
{
    for( int i=0; i<N; i++ ) X[i] = 0.0f;
    X[0] = 1.0f;
}

// Parse the command line arguments. Returns 0 if successful, -1 for any error.
int parseCmdLineArgs( int argc, char **argv, int *N, int *nIters, int *nThreads )
{ 
    if( argc != 4 )
    {
        printf( "ERROR: Need three command line argument; the problem size N, the number of iterations, and the number of threads to use.\n" );
        return -1;
    }

    *N = atoi( argv[1] );
    if( *N < 1 )
    {
        printf( "ERROR: The problem size N ('%s') should be a positive integer.\n", argv[1] );
        return -1;
    }

    *nIters = atoi( argv[2] );
    if( *nIters<1 )
    {
        printf( "ERROR: The number of iterations ('%s') should be a positive integer.\n", argv[2] );
        return -1;
    }

    *nThreads = atoi( argv[3] );
    if( *nThreads < 1 )
    {
        printf( "ERROR: The number of threads ('%s') should be a positive integer.\n", argv[3] );
        return -1;
    }

    if( (*N)%(*nThreads) )
    {
        printf( "ERROR: The problem size N=%d must be an exact multiple of the number of threads %d.\n", *N, *nThreads );
        return -1;
    }

    return 0;
}


//
// Main.
//
int main( int argc, char **argv )
{
    //
    // Set-up the problem on the main thread.
    //
    int N, nIters, nThreads;
    if( parseCmdLineArgs(argc,argv,&N,&nIters,&nThreads)==-1 ) return EXIT_FAILURE;

    // Initialise the solution array and its copy.
    float
        *X = (float*) malloc( N*sizeof(float) ),
        *X_copy = (float*) malloc( N*sizeof(float) );
    
    if( !X || !X_copy )
    {
        printf( "ERROR: Could not allocate memory for the solution array and its copy.\n" );
        return EXIT_FAILURE;
    }

    initialiseX( X, N );

    // Start the timing now.
    struct timespec startTime, endTime;
    clock_gettime( CLOCK_REALTIME, &startTime );

    //
    // Parallel solver.
    //
    pthread_t *ids = (pthread_t*) malloc( nThreads*sizeof(pthread_t) );
    threadArgs_t *args = (threadArgs_t*) malloc( nThreads*sizeof(threadArgs_t) );

    // Set up the per-thread arguments. These will never change (for each thread).
    int NperThread = N / nThreads;
    for( int p=0; p<nThreads; p++ )
    {
        args[p].N = N;
        args[p].start = p * NperThread;
        args[p].end = (p+1) * NperThread;
        args[p].X = X;
        args[p].X_copy = X_copy;
    }

    for( int iter=0; iter<nIters; iter++ )
    {
        // Step 1. Copy C to X_copy in parallel.
        for( int p=0; p<nThreads; p++ )
            pthread_create( &ids[p], NULL, copyX_perThread, &args[p] );
    
        for( int p=0; p<nThreads; p++ ) pthread_join( ids[p], NULL );

        // Step 2. Update X using the values in X-copy.
        for( int p=0; p<nThreads; p++ )
            pthread_create( &ids[p], NULL, updateX_perThread, &args[p] );

        for( int p=0; p<nThreads; p++ ) pthread_join( ids[p], NULL );
    }

    //
    // Output timing, check result, clear up and quit
    //

    // Output final time taken.
    clock_gettime( CLOCK_REALTIME, &endTime );
    double seconds = (double)( endTime.tv_sec + 1e-9*endTime.tv_nsec - startTime.tv_sec - 1e-9*startTime.tv_nsec );
    printf( "Time for parallel calculations: %g secs.\n", seconds );

    // Check against the serial calculation.
    float *serialX = (float*) malloc( N*sizeof(float) );
    if( !serialX ) return EXIT_FAILURE;
    initialiseX( serialX, N );

    for( int iter=0; iter<nIters; iter++ )
    {
        // Copy solution array to the temporary 'copy' array.
        for( int i=0; i<N; i++ ) X_copy[i] = serialX[i];

        // Perform one iteration from the copy back to the solution vector.
        for( int i=1; i<N-1; i++ )
            serialX[i] = 0.5f * ( X_copy[i-1] + X_copy[i+1] );
    }

    // Display first few elements.
    printf( "Displaying first few elements of X compared to the serial prediction:\n" );
    for( int i=0; i<(N>10?10:N); i++ )
        printf( "i=%d:\tX=%f\t(serial=%f)\n", i, X[i], serialX[i] );

    // Check full solution. Allow small differences due to floating point arithmetic errors.
    for(int i=0; i<N; i++ )
        if( fabs(serialX[i]-X[i])>1e-3 )
        {
            printf( "Serial check FAILED for at least one element; first error found at index i=%d.\n", i );
            return EXIT_FAILURE;
        }
    
    printf( "Serial check passed.\n" );

    // Clear up and quit.
    free( X );
    free( X_copy );
    free( serialX );

    return EXIT_SUCCESS;
}