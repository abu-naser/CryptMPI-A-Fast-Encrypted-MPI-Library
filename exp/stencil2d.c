/*****Author - Saptarshi Bhowmik*****/

/*
 Updated by an16e@my.fsu.edu
 Changes:
  - Initialized the grid
  - Dummy computation added
  - Parameter to control the computation percentage
*/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#define WRITE_OTF2_TRACE 0

#if WRITE_OTF2_TRACE
#include <scorep/SCOREP_User.h>
#endif

#define calc_pe(a, b) (a + b * nx)
#define wrap_x(a) (((a) + nx) % nx)
#define wrap_y(a) (((a) + ny) % ny)

#define LEFT 1
#define RIGHT 2
#define TOP 3
#define BOTTOM 4
#define MATRIX 4

#define SKIP_ITE 20

volatile long counter = 0;

void matrix_multiply(volatile long m1[MATRIX][MATRIX],
                     volatile long m2[MATRIX][MATRIX],
                     volatile long r[MATRIX][MATRIX])
{
  int i, j, k;
  for (i = 0; i < MATRIX; i++)
  {
    for (j = 0; j < MATRIX; j++)
    {
      r[i][j] = 0;
      for (k = 0; k < MATRIX; k++)
        r[i][j] += m1[i][k] *
                   m2[k][j];
    }
  }
}

int main(int argc, char **argv)
{
  int myrank, numranks, i, j, l, m;
  struct timeval comm_start_time, comm_end_time;
  gettimeofday(&comm_start_time, NULL);
  MPI_Init(&argc, &argv);
  gettimeofday(&comm_end_time, NULL);
  double t = (double)(comm_end_time.tv_usec - comm_start_time.tv_usec) / 1000000 + (double)(comm_end_time.tv_sec - comm_start_time.tv_sec);

#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_OFF();
#endif
  MPI_Comm_size(MPI_COMM_WORLD, &numranks);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if (argc != 8)
  {
    if (!myrank)
      printf("\nThis is the stencil2D (aka halo2d) communication proxy. The correct usage is:\n"
             "%s nx ny bx by nvar MAX_ITER\n\n"
             "    nx, ny: layout of process grid in 2D\n"
             "    bx, by: grid size on each process\n"
             "    nvar: number of variables at each grid point\n"
             "    MAX_ITER: how many iters to run\n"
             "    total_com_time: Total time spent on computation\n\n",
             argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Request req[4];
  MPI_Request sreq[4];
  MPI_Status status[4];

  int nx = atoi(argv[1]);
  int ny = atoi(argv[2]);
  int bx = atoi(argv[3]);
  int by = atoi(argv[4]);
  int nvar = atoi(argv[5]);
  int MAX_ITER = atoi(argv[6]);
  double total_com_time = atof(argv[7]);
  volatile int index;

  if (nx * ny != numranks)
  {
    if (!myrank)
    {
      printf("\n nx * ny  does not equal number of ranks. \n");
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // figure out my coordinates
  int myXcoord = (myrank % nx);
  int myYcoord = (myrank % (nx * ny)) / nx;

  if (myrank == 0)
  {
    printf("Running stencil2d with diagonals on %d processors each with (%d, %d ) grid points with %d variables\n", numranks, bx, by, nvar);
  }

  double *left_block_out = (double *)calloc(by * bx * nvar, sizeof(double));
  double *right_block_out = (double *)calloc(by * bx * nvar, sizeof(double));
  double *left_block_in = (double *)calloc(by * bx * nvar, sizeof(double));
  double *right_block_in = (double *)calloc(by * bx * nvar, sizeof(double));

  double *bottom_block_out = (double *)calloc(bx * by * nvar, sizeof(double));
  double *top_block_out = (double *)calloc(bx * by * nvar, sizeof(double));
  double *bottom_block_in = (double *)calloc(bx * by * nvar, sizeof(double));
  double *top_block_in = (double *)calloc(bx * by * nvar, sizeof(double));

  volatile long r[MATRIX][MATRIX] = {{0, 0, 0, 0},
                                     {0, 0, 0, 0},
                                     {0, 0, 0, 0},
                                     {0, 0, 0, 0}};
  volatile long m1[MATRIX][MATRIX] = {{1, 1, 1, 1},
                                      {2, 2, 2, 2},
                                      {3, 3, 3, 3},
                                      {4, 4, 4, 4}};

  volatile long m2[MATRIX][MATRIX] = {{1, 1, 1, 1},
                                      {2, 2, 2, 2},
                                      {3, 3, 3, 3},
                                      {4, 4, 4, 4}};

  double startTime, stopTime;
  double istartTime, iendTime;
  double elapsedTime = 0.0;
  double ite_com_time = total_com_time / MAX_ITER;

#if WRITE_OTF2_TRACE
  SCOREP_RECORDING_ON();
  // Marks the beginning of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_Loop", SCOREP_USER_REGION_TYPE_COMMON);
  // Marks when to print a timer in simulation
  if (!myrank)
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_WallTime_stencil3d", SCOREP_USER_REGION_TYPE_COMMON);
#endif

  for (i = 0; i < MAX_ITER + SKIP_ITE; i++)
  {

#if WRITE_OTF2_TRACE
    // Marks compute region before messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_stencil3d_pre_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_stencil3d_pre_msg");
#endif

    if (i == SKIP_ITE)
    {
      MPI_Barrier(MPI_COMM_WORLD);
      startTime = MPI_Wtime();
    }

    /* Dummy computation */
    if (i >= SKIP_ITE && ite_com_time > 0.0)
    {
      istartTime = MPI_Wtime();
      for (j = 1;; j++)
      {

        index = (j + i) % 4;
        m1[0][0] = r[index][1];
        m2[0][1] = r[index][2];

        right_block_in[0] = (double)r[index][1];
        left_block_in[1] = (double)r[index][1];
        top_block_in[2] = (double)r[index][1];
        bottom_block_in[3] = (double)r[index][1];
        left_block_out[4] = (double)r[index][2];
        right_block_out[5] = (double)r[index][3];
        bottom_block_out[6] = (double)r[index][0];
        top_block_out[7] = (double)r[index][2];

        matrix_multiply(m1, m2, r);
        counter++;
        if (MPI_Wtime() - istartTime >= ite_com_time)
          break;
      }

      elapsedTime += MPI_Wtime() - istartTime;
    }

    // post receives: one for each direction and dimension
    MPI_Irecv(right_block_in, bx * by * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord + 1), myYcoord), RIGHT, MPI_COMM_WORLD, &req[RIGHT - 1]);
    MPI_Irecv(left_block_in, bx * by * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord - 1), myYcoord), LEFT, MPI_COMM_WORLD, &req[LEFT - 1]);
    MPI_Irecv(top_block_in, bx * by * nvar, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord + 1)), TOP, MPI_COMM_WORLD, &req[TOP - 1]);
    MPI_Irecv(bottom_block_in, bx * by * nvar, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord - 1)), BOTTOM, MPI_COMM_WORLD, &req[BOTTOM - 1]);

    // initiate sends: one for each direction and dimension
    MPI_Isend(left_block_out, bx * by * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord - 1), myYcoord), RIGHT, MPI_COMM_WORLD, &sreq[RIGHT - 1]);
    MPI_Isend(right_block_out, bx * by * nvar, MPI_DOUBLE, calc_pe(wrap_x(myXcoord + 1), myYcoord), LEFT, MPI_COMM_WORLD, &sreq[LEFT - 1]);
    MPI_Isend(bottom_block_out, bx * by * nvar, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord - 1)), TOP, MPI_COMM_WORLD, &sreq[TOP - 1]);
    MPI_Isend(top_block_out, bx * by * nvar, MPI_DOUBLE, calc_pe(myXcoord, wrap_y(myYcoord + 1)), BOTTOM, MPI_COMM_WORLD, &sreq[BOTTOM - 1]);

#if WRITE_OTF2_TRACE
    // Marks compute region for computation-communication overlap
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_stencil3d_overlap", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_stencil3d_overlap");
#endif

    //wait for all communication to complete
    MPI_Waitall(4, req, status);
    MPI_Waitall(4, sreq, status);

#if WRITE_OTF2_TRACE
    // Marks compute region after messaging
    SCOREP_USER_REGION_BY_NAME_BEGIN("TRACER_stencil3d_post_msg", SCOREP_USER_REGION_TYPE_COMMON);
    SCOREP_USER_REGION_BY_NAME_END("TRACER_stencil3d_post_msg");
#endif
  }

#if WRITE_OTF2_TRACE
  // Marks the end of code region to be repeated in simulation
  SCOREP_USER_REGION_BY_NAME_END("TRACER_Loop");
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  stopTime = MPI_Wtime();

#if WRITE_OTF2_TRACE
  // Marks when to print a timer in simulation
  if (!myrank)
    SCOREP_USER_REGION_BY_NAME_END("TRACER_WallTime_stencil3d");
  SCOREP_RECORDING_OFF();
#endif

  //finalized summary output
  if (myrank == 0 && MAX_ITER != 0)
  {
    printf("Finished %d iterations\n", MAX_ITER);
    printf("Time elapsed per iteration for grid size (%d,%d) x %d x 8: %f s\n",
           bx, by, nvar, (stopTime - startTime) / MAX_ITER);
    printf("Total Computation time %0.2lf s\nelapsed time = %lf s\neach iteration Computation time %lf s\ncomp iteration = %ld\n",
           total_com_time, elapsedTime, ite_com_time, counter);
    printf("Computation is %0.2lf percentage of total time %lf s\n",
           (elapsedTime * 100.0 / (stopTime - startTime)), (stopTime - startTime));
    printf("init-time t = %lf\n", t);
    printf("Dump dummy results:\n %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
           r[0][0], r[0][1], r[0][2], r[0][3], r[1][0], r[1][1], r[1][2], r[1][3], r[2][0], r[2][1], r[2][2], r[2][3], r[3][0], r[3][1], r[3][2], r[3][3]);
  }

  MPI_Finalize();
  free(left_block_out);
  free(right_block_out);
  free(left_block_in);
  free(right_block_in);
  free(bottom_block_out);
  free(top_block_out);
  free(bottom_block_in);
  free(top_block_in);
}
