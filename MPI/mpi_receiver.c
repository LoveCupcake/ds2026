#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 1) {
        long size;
        MPI_Recv(&size, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char *buffer = (char*) malloc(size);
        MPI_Recv(buffer, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        FILE *fp = fopen("received_mpi.txt", "wb");
        fwrite(buffer, 1, size, fp);

        printf("File received via MPI (%ld bytes)\n", size);

        fclose(fp);
        free(buffer);
    }

    MPI_Finalize();
    return 0;
}
