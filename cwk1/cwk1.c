//
// Starting code for coursework 1. Requires "cwk1_extra.h", which should be placed in the same directory.
//
// Compile as per the instructions in Lecture 2, or use the provided makefile.
//


//
// Standard includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


//
// The set itself and the routines to initialise, destroy and print it are defined
// in the file cwk1_extra.h that you should also have downloaded. Although you are
// free to inspect this file, do not alter the routines it contains (or replace them
// with your own versions), as they will be replaced as part of the assessment.
//
// For your information, cwk1_extra.h includes the following global variables:
//
// int setSize;                 // The current size of the set = number of items it contains.
// int maxSetSize;              // The maximum size for the set (which should not be altered).
// int *set;                    // The set itself, an array of size maxSetSize.
//
// It also contains the following routines:
//
// int initSet( int maxSize );  // Initialises the set to the given size.
//                              // Returns 0 if successful, -1 for an allocation error.
//
// void destroySet();           // Destroys the set (i.e. deletes all associated resources).
//
// void printSet();             // Prints the set.
//
#include "cwk1_extra.h"




//
// Add a value to the set if it does not currently exist.
//
void addToSet( int value )
{
    int should_return = 0;


    #pragma omp critical
    {
    // Cannot exceed the maximum size.
    if( setSize==maxSetSize ) should_return = 1;

    // Since sets should not have duplicates, first check this value is not already in the set.
    for( int i=0; i<setSize; i++ )
        if( set[i]==value )
        {
            should_return = 1;
        }
    }

    if( should_return == 1) return;

    #pragma omp critical
    {
    	// Only reach this point if the value was not found and there is room to add to the set.
    	set[setSize] = value;
    	setSize++;
    }
}

//
// Remove a value from the set, if it exists, and shuffle the remaining values so the set remains contiguous.
//
void removeFromSet( int value )
{
    // int i;

    // Find where the index in the set corresponding to the value, if any.
    int index = -1;
    #pragma omp parallel for
    for( int i=0; i<setSize; i++ )
        if( set[i]==value )
            index = i;

    // If found, 'remove'. Here, 'removal' is achieved by moving all values later in the set down by one index,
    // and also reducing the set size by one.
    if( index!= -1 )
    {
        float set_temp[setSize];

        #pragma omp parallel for
        for ( int i=index; i<setSize; i++)
            set_temp[i] = set[i];


        #pragma omp parallel for
        for( int i=index; i<setSize-1; i++ )
            set[i] = set_temp[i+1];

        setSize--;
    }
}


//
// Parallel sort.
//
void sortSet()
{
    for( int i = 0; i < setSize - 1; i++)
    {
        // Update odd and evens at different times.
        // See lecture 5 redBlack code
        #pragma omp parallel for
        for( int j = i % 2; j < setSize - 1; j+=2)
        {
            if( set[j] > set[j+1] )
            {
                int temp = set[j];
                set[j] = set[j+1];
                set[j+1] = temp;
            }
        }
    }
}


//
// Main.
//
int main( int argc, char **argv )
{
    int i;

    // Get from the command line the maximum set size, the number of values to add, the number to remove,
    // and whether or not to sort the set (in that order). You do not need to alter this section of the code.
    if( argc!=5 )
    {
        printf( "Need 4 command line arguments: The maximum set size; how many values to add initially; how many" );
        printf( " to then remove; and whether or not to sort the set at the end [0/1].\n" );
        return EXIT_FAILURE;
    }

    int
        maxSetSize  = atoi(argv[1]),
        initSetSize = atoi(argv[2]),
        numToRemove = atoi(argv[3]),
        sortYesNo   = atoi(argv[4]);

    if( maxSetSize<=0 || initSetSize<0 || numToRemove<0 || sortYesNo<0 || sortYesNo>1 )
    {
        printf( "Invalid arguments. Check that: the max. size > 0; the numbers to add/remove are non-negative;" );
        printf( " and the option to sort is either 0 or 1.\n" );
        return EXIT_FAILURE;
    }

    // Initialise the set. Returns -1 if could not allocate memory.
    if( initSet(maxSetSize)==-1 ) return EXIT_FAILURE;

    // Seed the psuedo-random number generator to the current time.
    srand( time(NULL) );

    // Add random numbers in the range 0 to maxSetSize-1 inclusive.

    // ###### THIS IS WHERE QUESTION 1 HAPPENS ######
    #pragma omp parallel for
    for( i=0; i<initSetSize; i++ )
    {
        addToSet( rand()%maxSetSize );
    }

    printf( "Attempted to add %i random values. Current state of set:\n", initSetSize );
    printSet();

    // Remove values from the set; random values from the same range as they were added.
    for( i=0; i<numToRemove; i++ )
        removeFromSet( rand()%maxSetSize );

    printf( "\nRemoved up to %i random values if present. Current state of set:\n", numToRemove );
    printSet();

    // Finally, sort the set in increasing order.
    if (sortYesNo==1 )
    {
        sortSet();
        printf( "\nCalled sortSet(). Current state of set:\n" );
        printSet();
    }

    // You MUST call this function just before finishing - do NOT remove, or change the definition of destroySet(),
    // as it will be changed with a different version for assessment.
    destroySet();

    return EXIT_SUCCESS;
}
