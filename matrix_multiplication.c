#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

long rows_num_1, columns_num_1, rows_num_2, columns_num_2, num_threads;
double **matrix1, **matrix2, **matrix3;
pthread_mutex_t lock;

double ** allocate_matrix( long rows_num, long columns_num )
{
  double * vals = (double *) malloc( rows_num * columns_num * sizeof(double) );
  double ** ptrs = (double **) malloc( rows_num * sizeof(double*) );
  long i;
  for (i = 0; i < rows_num; ++i) {
    ptrs[ i ] = &vals[ i * columns_num ];
  }
  return ptrs;
}

void init_matrix( double **matrix, long rows_num, long columns_num )
{
  long i, j;

  for (i = 0; i < rows_num; ++i) {
    for (j = 0; j < columns_num; ++j) {
      matrix[ i ][ j ] = rand() % 10;
    }
  }
}

void init_matrix_zeros( double **matrix, long rows_num, long columns_num )
{
  long i, j;

  for (i = 0; i < rows_num; ++i) {
    for (j = 0; j < columns_num; ++j) {
      matrix[ i ][ j ] = 0;
    }
  }
}

void print_matrix( double **matrix, long rows_num, long columns_num )
{
  long i, j;

  for (i = 0; i < rows_num; ++i) {
    for (j = 0; j < columns_num-1; ++j) {
      printf( "%lf, ", matrix[ i ][ j ] );
    }
    printf( "%lf", matrix[ i ][ j ] );
    putchar( '\n' );
  }
}

void * worker_rows( void *arg )
{
  long i, j, k, tid, portion_size, row_start, row_end;
  double sum;
  tid = *(long *)(arg);
  portion_size = rows_num_1 / num_threads;
  row_start = tid * portion_size;
  row_end = (tid+1) * portion_size;
  if (tid == num_threads-1 && row_end != rows_num_1) {row_end = rows_num_1;}
  for (i = row_start; i < row_end; ++i) {
    for (j = 0; j < columns_num_2; ++j) {
      sum = 0;
      for (k = 0; k < columns_num_1; ++k) {
        sum += matrix1[ i ][ k ] * matrix2[ k ][ j ];
      }
      matrix3[ i ][ j ] = sum;
    }
  }
}

void * worker_columns( void *arg )
{
  long i, j, k, tid, portion_size, column_start, column_end;
  double * sum = (double *) malloc( rows_num_1 * sizeof(double) );
  tid = *(long *)(arg);
  portion_size = columns_num_1 / num_threads;
  column_start = tid * portion_size;
  column_end = (tid+1) * portion_size;
  if (tid == num_threads-1 && column_end != columns_num_1) {column_end = columns_num_1;}
  pthread_mutex_lock(&lock);
  for (i = column_start; i < column_end; ++i) {
    for (j = 0; j < columns_num_2; ++j) {
      for (k = 0; k < rows_num_1; ++k) {
        matrix3[ k ][ j ] += matrix1[ k ][ i ] * matrix2[ i ][ j ];
      }
    }
  }
  pthread_mutex_unlock(&lock);
}

void * worker_blocks( void *arg )
{
  long i, j, k, tid, portion_size_columns, portion_size_rows, column_start, column_end, num_of_blocks, row_start, row_end, num_of_column_blocks;
  tid = *(long *)(arg);
  num_of_blocks = (long)pow(2,num_threads);
  num_of_column_blocks = (long)num_of_blocks/2;
  if(num_of_blocks == 1) {num_of_column_blocks = 1;}
  portion_size_columns = (long)(columns_num_1/num_of_column_blocks);
  portion_size_rows = (long)(rows_num_1/2);
  if(num_of_blocks == 1) {portion_size_rows = rows_num_1;}
  column_start = (tid%num_of_column_blocks) * portion_size_columns;
  column_end = ((tid)%num_of_column_blocks + 1) * portion_size_columns;
  row_start = (int)(tid/num_of_column_blocks) * portion_size_rows;
  row_end = (int)(((tid)/num_of_column_blocks)+1) * portion_size_rows;
  pthread_mutex_lock(&lock);
  for(int k=row_start; k < row_end; k++) {
		for(int h=0; h < columns_num_2; h++) {
      for(int l=column_start; l<column_end; l++) {
        matrix3[k][h] += matrix1[k][l]*matrix2[l][h];
      }
		}
	}
  pthread_mutex_unlock(&lock);
}

int main( int argc, char *argv[] )
{
  long i;
  double sum = 0;
  struct timeval tstart, tend;
  double exectime;
  pthread_t * threads;
  if (argc != 2) {
    printf("Error: Not enought arguments\n");
    return -1;
  }
  rows_num_1 = atoi( argv[1] );
  columns_num_1 = rows_num_1;
  rows_num_2 = rows_num_1;
  columns_num_2 = columns_num_1;
  num_threads = 1;
  matrix1 = allocate_matrix( rows_num_1, columns_num_1 );
  matrix2 = allocate_matrix( rows_num_2, columns_num_2 );
  matrix3 = allocate_matrix( rows_num_1, columns_num_2 );
  init_matrix( matrix1, rows_num_1, columns_num_1 );
  init_matrix( matrix2, rows_num_2, columns_num_2 );
  for (num_threads=num_threads; num_threads<=128;num_threads=num_threads*2) {
    threads = (pthread_t *) malloc( num_threads * sizeof(pthread_t) );
    gettimeofday( &tstart, NULL );
    for ( i = 0; i < num_threads; ++i ) {
        long *tid;
        tid = (long *) malloc( sizeof(long) );
        *tid = i;
        pthread_create( &threads[i], NULL, worker_rows, (void *)tid );
    }
    for ( i = 0; i < num_threads; ++i ) {
        pthread_join( threads[i], NULL );
    }
    gettimeofday( &tend, NULL );
    exectime = (tend.tv_sec - tstart.tv_sec) * 1000.0;
    exectime += (tend.tv_usec - tstart.tv_usec) / 1000.0;
    printf( "Number of threads: %ld\tExecution time:%.5lf sec\n",num_threads, exectime/1000.0);
  }
  for (num_threads=1; num_threads<=128;num_threads=num_threads*2) {
    init_matrix_zeros(matrix3, rows_num_1, columns_num_2);
    threads = (pthread_t *) malloc( num_threads * sizeof(pthread_t) );
    gettimeofday( &tstart, NULL );
    for ( i = 0; i < num_threads; ++i ) {
        long *tid;
        tid = (long *) malloc( sizeof(long) );
        *tid = i;
        pthread_create( &threads[i], NULL, worker_columns, (void *)tid );
    }
    for ( i = 0; i < num_threads; ++i ) {
        pthread_join( threads[i], NULL );
    }
    gettimeofday( &tend, NULL );
    /*if (num_threads == 128) {
      print_matrix(matrix3, rows_num_1, columns_num_1);
    }*/
    exectime = (tend.tv_sec - tstart.tv_sec) * 1000.0;
    exectime += (tend.tv_usec - tstart.tv_usec) / 1000.0;  
    printf( "Number of threads: %ld\tExecution time:%.5lf sec\n",num_threads, exectime/1000.0);
    init_matrix_zeros(matrix3, rows_num_1, columns_num_2);
  }
  for (num_threads=0; num_threads<=7; num_threads++) {
    init_matrix_zeros(matrix3, rows_num_1, columns_num_2);
    threads = (pthread_t *) malloc( (long)pow(2, num_threads) * sizeof(pthread_t) );
    gettimeofday( &tstart, NULL );
    for ( i = 0; i < (long)pow(2, num_threads); ++i ) {
        long *tid;
        tid = (long *) malloc( sizeof(long) );
        *tid = i;
        pthread_create( &threads[i], NULL, worker_blocks, (void *)tid );
    }
    for ( i = 0; i < (long)pow(2, num_threads); ++i ) {
        pthread_join( threads[i], NULL );
    }
    gettimeofday( &tend, NULL );
    /*if ((long)pow(2, num_threads) == 128) {
      print_matrix(matrix3, rows_num_1, columns_num_1);
    }*/
    exectime = (tend.tv_sec - tstart.tv_sec) * 1000.0;
    exectime += (tend.tv_usec - tstart.tv_usec) / 1000.0;   
    printf( "Number of threads: %ld\tExecution time:%.5lf sec\n", (long)pow(2, num_threads), exectime/1000.0);
    init_matrix_zeros(matrix3, rows_num_1, columns_num_2); 
  }
  return 0;
}