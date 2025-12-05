#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        FILE *fp = fopen("test.txt", "rb");
        if (!fp) {
            printf("Cannot open file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        rewind(fp);

        char *buffer = (char*) malloc(size);
        fread(buffer, 1, size, fp);

        MPI_Send(&size, 1, MPI_LONG, 1, 0, MPI_COMM_WORLD);
        MPI_Send(buffer, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);

        printf("File sent via MPI (%ld bytes)\n", size);

        fclose(fp);
        free(buffer);
    }

    MPI_Finalize();
    return 0;
}
