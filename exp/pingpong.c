/**************************************
* an16e@my.fsu.edu                    *
* PingPong                            *
* Measures Uni-directional throughput *
**************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define FLOAT_PRECISION 5
#define FIELD_WIDTH 20
#define sz 5*1024*1024
#define MG 1048576.00
#define low 1 * 1024
#define high 4 * 1024 * 1024
#define times 10000
#define normal 0
#define SIZE_NUM 8
#define key_size 16
#define set_siv 0

static int lengths[SIZE_NUM + 10] = {32 * 1024, 64 * 1024, 128 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024, 2 * 1024 * 1024, 4 * 1024 * 1024};

int main(int argc, char **argv)
{

	MPI_Init(NULL, NULL);

	int world_rank, world_size, j, datasz, iteration, datasz1;
	char *sendbuf, *recvbuf;
	double t_start, t_end, t;

	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	sendbuf = (char *)malloc(sz * sizeof(char));
	recvbuf = (char *)malloc(sz * sizeof(char));

	memset(sendbuf, 'a', 4194304);
	memset(recvbuf, 'b', 4194304);

	// We are assuming at least 2 processes for this
	if (world_size != 2)
	{
		fprintf(stderr, "World size must be two for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	iteration = times;
	int indx;
	int local_var = 0;

	if (world_rank == 0)
		printf("\n# Size           Bandwidth (MB/s)\n");
	iteration = times;

	for (indx = 0; indx < SIZE_NUM; indx++)
	{
		if (datasz >= 1024 * 1024)
			iteration = 1000;
		datasz = lengths[indx];

		for (j = 1; j <= iteration + 20; j++)
		{
			if (j == 20)
			{
				t_start = MPI_Wtime();
			}
			if (world_rank == 0)
			{
				MPI_Send(sendbuf, datasz, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
				MPI_Recv(recvbuf, datasz, MPI_CHAR, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if (world_rank == 1)
			{
				MPI_Recv(recvbuf, datasz, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Send(sendbuf, datasz, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
			}
		}
		t_end = MPI_Wtime();
		if (world_rank == 0)
		{
			t = t_end - t_start;

			double tmp = (datasz * 2) / MG * iteration;
			fprintf(stdout, "%f\n", tmp / t);
			fflush(stdout);
		}
	}

	free(sendbuf);
	free(recvbuf);

	MPI_Finalize();
}
