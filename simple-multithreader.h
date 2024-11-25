#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main

int min(int x, int y) {
  return (x <= y ? x : y);
}

struct linear_args {
  int low;
  int high;
  std::function<void(int)> lambda;
};

struct matrix_args {
  int linear_low;
  int linear_high;

  int cols;

  std::function<void(int, int)> lambda;
};

void* linear_thread_func(void* args) {
  struct linear_args* lin_args = (struct linear_args*) args;

  // note, high is excluded
  for(int i = lin_args->low; i < lin_args->high; i++) {
    (lin_args->lambda)(i);
  }

  return NULL;
}

void* matrix_thread_func(void* args) {
  struct matrix_args* matrix_args = (struct matrix_args*) args;

  // note, high is excluded
  for(int i = matrix_args->linear_low; i < matrix_args->linear_high; i++) {

    // obtain the (row, col) from the linear index
    int row = i / matrix_args->cols;
    int col = i % matrix_args->cols;

    // printf("%d %d\n", row, col);

    (matrix_args->lambda)(row, col);
  }

  return NULL;
}

// parallel_for accepts a C++11 lambda function and runs the loop body (lambda) in  
// parallel by using ‘numThreads’ number of Pthreads to be created by the simple-multithreader 
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {

  // allocate array to store thread objects
  pthread_t* tid_array = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
  if (tid_array == NULL) {
    perror("Error while allocating pthread_t array");
    exit(1);
  }

  // allocate array to store linear arguments
  struct linear_args* linear_args_array = (struct linear_args*)  malloc( sizeof(struct linear_args) * numThreads );
  if (linear_args_array == NULL) {
    perror("Error while allocating linear_args array");
    exit(1);
  }

  // for(int i = low; i < high; i++)
  // (high - 1) - low + 1
  int total_elements = high - low;
  int chunk_size = (total_elements)/numThreads;

  // ~~~ Proof that high never goes out of bounds ~~~
  // note, chunk_size = floor( total_elements/numThreads )
  // floor( total_elements/numThreads ) <= total_elements/numThreads
  // so, chunk_size <= total_elements/numThreads
  // so, chunk_size * numThreads <= total_elements

  // high is (i + 1) * chunk_size
  // max value of i is numThreads - 1
  // so, max value of high is numThreads * chunk_size
  // so, high will never go out of bounds

  // ~~~ Proof that if total_elements % numThreads != 0, then some elements may be missed ~~~
  // when total_elements % numThreads = 0, total_elements/numThreads is an integer and floor(total_elements/numThreads) = total_elements/numThreads
  // so, chunk_size = total_elements/numThreads. so, chunk_size * numThreads = total_elements
  // so, the high value for the last thread is ((num_threads - 1) + 1) * chunk_size = total_elements. so, no elements are missed

  // but when total_elements % numThreads != 0, total_elements/numThreads is not an integer and floor(total_elements/numThreads) < total_elements/numThreads
  // so, chunk_size < total_elements/numThreads
  // so, chunk_size * numThreads < total_elements
  // so the high value for the last thread is ( (num_threads - 1) + 1 ) * chunk_size < total_elements. so, elements are definitely missed.
  // therefore, we need the last thread to cover some extra elements

  for(int i = 0; i < numThreads; i++) {
    linear_args_array[i].low = i * chunk_size;

    // make sure that you don't go beyond the array
    linear_args_array[i].high = ( i == numThreads - 1 ? total_elements : (i + 1) * chunk_size);
    linear_args_array[i].lambda = lambda;

    // printf("thread %d: %d %d\n", i, linear_args_array[i].low, linear_args_array[i].high);

    // create the thread
    if ( pthread_create( &tid_array[i], NULL, linear_thread_func, (void*) &linear_args_array[i] ) != 0 ) {
      perror("pthread_create error");
      exit(1);
    }
  }

  // wait for the threads
  for(int i = 0; i < numThreads; i++) {
    if ( pthread_join( tid_array[i], NULL ) != 0 ) {
      perror("pthread_join error");
      exit(1);
    }
  }

  // clean up 
  free(tid_array);
  free(linear_args_array);
}
 
// This version of parallel_for is for parallelizing two-dimensional for-loops, i.e., an outer for-i loop and  
// an inner for-j loop. Loop properties, i.e. low, high are mentioned below for both outer  
// and inner for-loops. The suffixes “1” and “2” represents outter and inner loop properties respectively.  

// note (row, col) in 2D array corresponds to row * num_cols + col in 1D array
// linear_idx in 1D has row = linear_idx/num_cols and col = linear_idx % num_cols 

// it's hard to deal with 2D coordinates of (row, col)
// so, we convert each (row, col) into a linear index and then store that in the matrix_args struct
// in matrix_thread_func(), we convert the linear index back into 2D (row, col)

void parallel_for(int low1, int high1,  int low2, int high2, std::function<void(int, int)>  &&lambda, int numThreads) {



  // allocate array to store thread objects
  pthread_t* tid_array = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
  if (tid_array == NULL) {
    perror("Error while allocating pthread_t array");
    exit(1);
  }

  // allocate array to store linear arguments
  struct matrix_args* matrix_args_array = (struct matrix_args*)  malloc( sizeof(struct matrix_args) * numThreads );
  if (matrix_args_array == NULL) {
    perror("Error while allocating matrix_args array");
    exit(1);
  }

  // for(int j = low2; j < high2; j++)
  // (high2 - 1) - low2 + 1
  int num_cols = high2 - low2;

  // start at (low1, low2) and end at (high1 - 1, high2 - 1) (included)
  int linear_start_idx = low1 * num_cols + low2;
  int linear_end_idx = (high1 - 1) * num_cols + (high2 - 1);

  // (end_idx) - start_idx + 1
  int total_elements = linear_end_idx - linear_start_idx + 1;
  int chunk_size = (total_elements)/numThreads;

  // printf("low1: %d high1: %d low2: %d high2: %d\n", low1, high1, low2, high2);
  // printf("cols: %d start: %d end: %d total: %d sz: %d\n", num_cols, linear_start_idx, linear_end_idx, total_elements, chunk_size);

  for(int i = 0; i < numThreads; i++) {

    matrix_args_array[i].linear_low = i * chunk_size;
    matrix_args_array[i].linear_high = ( i == numThreads - 1 ? total_elements : (i + 1) * chunk_size);
    matrix_args_array[i].cols = num_cols;
    matrix_args_array[i].lambda = lambda;

    // printf("thread %d: %d %d\n", i, matrix_args_array[i].linear_low ,  matrix_args_array[i].linear_high );

    // create threads
    if (pthread_create( &tid_array[i], NULL, matrix_thread_func, (void*) &matrix_args_array[i] ) != 0) {
      perror("pthread_create error");
      exit(1);
    }
  }

  // wait for threads
  for(int i = 0; i < numThreads; i++) {
    if (pthread_join( tid_array[i], NULL ) != 0) {
      perror("pthread_join error");
      exit(1);
    }
  }

  // clean up
  free(tid_array);
  free(matrix_args_array);
}

