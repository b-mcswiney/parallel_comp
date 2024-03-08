#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define ROOT 0

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int parent_rank = (rank - 1) / 2; // Calculate parent rank

    if (rank != ROOT) {
        // Send message to parent
        int message = rank;
        MPI_Send(&message, 1, MPI_INT, parent_rank, 0, MPI_COMM_WORLD);
        printf("Process %d sent message to parent %d\n", rank, parent_rank);
    }

    if (2 * rank + 1 < size) {
        // Receive message from left child
        int left_child_rank = 2 * rank + 1;
        int received_message;
        MPI_Recv(&received_message, 1, MPI_INT, left_child_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received message from left child %d\n", rank, left_child_rank);
    }

    if (2 * rank + 2 < size) {
        // Receive message from right child
        int right_child_rank = 2 * rank + 2;
        int received_message;
        MPI_Recv(&received_message, 1, MPI_INT, right_child_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received message from right child %d\n", rank, right_child_rank);
    }

    MPI_Finalize();
    return 0;
}
