//
// Vector addition in serial. Compile with: gcc -Wall -o vectorAddition_serial vectorAddition_serial.c
//

// Includes.
#include <stdio.h>
#include <stdlib.h>

// n is the size of all three vectors. For simplicity, use statically allocated arrays
// [rather than dynamically allocated using malloc()/free()]
#define n 20

int main()
{
	int* x = (int*) malloc(n * sizeof(int));
	int* y = (int*) malloc(n * sizeof(int));
	int a = 5, success=1;

    for(int i = 0; i < n; i++) x[i] = y[i] = i;

    #pragma omp parallel for
    for(int i = 0; i < n; i++)
    {
        y[i] = a*x[i] + y[i];
        printf("Thread num: %i Max Theads: %i \n", omp_get_thread_num(), omp_get_max_threads());
    }

    for( int i = 0; i < n; i++)
    {
        if( y[i] != (5*i) +  i)
        {
            success = 0;
        }
    }

    free(x);
    free(y);

    if(success == 1)
        printf( "Addition Successful.\n" );

    else
    {
        printf( "Addition failed.\n" );
    
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
