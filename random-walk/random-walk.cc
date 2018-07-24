#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <mpi.h>

using namespace std;

typedef struct {
    int location;
    int num_steps_left_in_walk;
} Walker;

void decompose_domain(int domain_size,
                      int world_rank,
                      int world_size,
                      int* subdomain_start,
                      int* subdomain_size) {
    if(world_size > domain_size) {
        // Assume the domain size is greater than the world size.
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    *subdomain_start = domain_size / world_size * world_rank;
    *subdomain_size = domain_size / world_size;
    if(world_rank == world_size-1) {
        // Give remainder to last process
        *subdomain_size += domain_size % world_size;
    }
}

void initialize_walkers(int num_walkers_per_proc,
                        int max_walk_size,
                        int subdomain_start,
                        int subdomain_size,
                        vector<Walker>* incoming_walkers) {
    Walker walker;
    for(int i = 0; i < num_walkers_per_proc; i++) {
        // Initialize walkers to the middle of the domain
        walker.location = subdomain_start;
        walker.num_steps_left_in_walk = (rand() / (float)RAND_MAX) * max_walk_size;
        incoming_walkers->push_back(walker);
    }
}

void walk(Walker* walker,
          int subdomain_start,
          int subdomain_size,
          int domain_size,
          vector<Walker>* outgoing_walkers) {
    while(walker->num_steps_left_in_walk > 0) {
        if(walker->location == subdomain_start + subdomain_size) {

            // Handle the case where the walker is at the end of the domain by wrapping
            if(walker->location == domain_size) {
                walker->location = 0;
            }
            outgoing_walkers->push_back(*walker);
            break;
        }
        else {
            walker->num_steps_left_in_walk--;
            walker->location++;
        }
    }
}

void send_outgoing_walkers(vector<Walker>* outgoing_walkers,
                           int world_rank,
                           int world_size) {
    // Send the data as an array of MPI_BYTEs to the next process
    // The last process sends to process 0
    MPI_Send((void*)outgoing_walkers->data(),
             outgoing_walkers->size() * sizeof(Walker),
             MPI_BYTE,
             (world_rank+1) % world_size,
             0,
             MPI_COMM_WORLD);
    // Now clear the outgoing walkers
    outgoing_walkers->clear();
}

void receive_incoming_walkers(vector<Walker>* incoming_walkers,
                              int world_rank,
                              int world_size) {
    MPI_Status status;
    
    // Receive from the process before you
    // If you are process 0, receive from the last process
    int incoming_rank = 
        (world_rank == 0) ? world_size - 1 : world_rank - 1;
    MPI_Probe(incoming_rank, 0, MPI_COMM_WORLD, &status);

    // Resize the incoming walker buffer based on how much data is incoming
    int incoming_walkers_size;
    MPI_Get_count(&status, MPI_BYTE, &incoming_walkers_size);
    incoming_walkers->resize(incoming_walkers_size / sizeof(Walker));
    MPI_Recv((void*)incoming_walkers->data(),
             incoming_walkers_size,
             MPI_BYTE,
             incoming_rank,
             0,
             MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
}

int main(int argc, char** argv) {
    int domain_size, max_walk_size, num_walkers_per_proc;
    if (argc < 4) {
        cerr << "Usage: random_walk domain_size max_walk_size "
             << "num_walkers_per_proc" << endl;
        exit(1);
    }

    // get the variables from the call to main()
    domain_size = atoi(argv[1]);
    max_walk_size = atoi(argv[2]);
    num_walkers_per_proc = atoi(argv[3]);
    MPI_Init(NULL, NULL);
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    srand(time(NULL) * world_rank);
    int subdomain_start, subdomain_size;
    vector<Walker> incoming_walkers, outgoing_walkers;

    // Find your part of the domain
    decompose_domain(domain_size, world_rank, world_size,
            &subdomain_start, &subdomain_size);

    // Initialize walkers in the subdomain
    initialize_walkers(num_walkers_per_proc, max_walk_size,
            subdomain_start, subdomain_size, &incoming_walkers);

    while(!all_walkers_finished) {
        // Process all incoming walkers
        for(int i = 0; i < incoming_walkers.size(); i++) {
            walk(&incoming_walkers[i], subdomain_start, subdomain_size,
                    domain_size, &outgoing_walkers);
        }

        // Send all outgoing walkers to the next process
        send_outgoing_walkers(&outgoing_walkers, world_rank, world_size);

        // Receive all the new incoming walkers
        receive_incoming_walkers(&incoming_walkers, world_rank, world_size);
    }

}
