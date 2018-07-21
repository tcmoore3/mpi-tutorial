#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // make sure we have at least 2 processes running this program
    if (world_size < 2) {
        fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int number;
    if (world_rank == 0) {
        number = -1;
        // MPI_Send(
            // void* data,
            // int count,
            // MPI_Datatype datatype,
            // int destination,
            // int tag,
            // MPI_Comm communicator)
        MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
    else if (world_rank == 1) {
        // MPI_Recv(
            // void* data,
            // int count,
            // MPI_Datatype datatype,
            // int source,
            // int tag,
            // MPI_Comm communicator,
            // MPI_Status* status)
        MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process 1 received number %d from process 0\n", number);
    }
    MPI_Finalize();
}
