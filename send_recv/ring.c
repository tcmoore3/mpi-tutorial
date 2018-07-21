#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int token = world_rank;
    if(world_rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
        printf("Process %d received token %d from process %d\n",
                world_rank, token, world_rank-1);
    }
    else {  // we are rank 0
        token = -1;
    }
    token = world_rank;
    MPI_Send(&token, 1, MPI_INT, (world_rank+1)%world_size, 0, MPI_COMM_WORLD);
    if(world_rank == 0) {
        MPI_Recv(&token, 1, MPI_INT, world_size-1, 0, MPI_COMM_WORLD, 
                MPI_STATUS_IGNORE);
        printf("Process %d received token %d from process %d\n",
                world_rank, token, world_size-1);
    }
    MPI_Finalize();
}
