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

void* linear_thread_func(void* args) {
  struct linear_args* lin_args = (struct linear_args*) args;

  for(int i = lin_args->low; i < lin_args->high; i++) {
    (lin_args->lambda)(i);
  }

  return NULL;
}

// parallel_for accepts a C++11 lambda function and runs the loop body (lambda) in  
// parallel by using ‘numThreads’ number of Pthreads to be created by the simple-multithreader 
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {

  pthread_t* tid_array = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
  struct linear_args* linear_args_array = (struct linear_args*)  malloc( sizeof(struct linear_args) * numThreads );

  int total_elements = high - low + 1;
  int chunk_size = (total_elements)/numThreads;

  for(int i = 0; i < numThreads; i++) {
    linear_args_array[i].low = i * chunk_size;
    linear_args_array[i].high = min( (i + 1) * chunk_size, total_elements );
    linear_args_array[i].lambda = lambda;

    pthread_create( &tid_array[i], NULL, linear_thread_func, (void*) &linear_args_array[i] );
  }

  for(int i = 0; i < numThreads; i++) {
    pthread_join( tid_array[i], NULL );
  }

  free(tid_array);
  free(linear_args_array);
}


 
// This version of parallel_for is for parallelizing two-dimensional for-loops, i.e., an outter for-i loop and  
// an inner for-j loop. Loop properties, i.e. low, high are mentioned below for both outter  
// and inner for-loops. The suffixes “1” and “2” represents outter and inner loop properties respectively.  
void parallel_for(int low1, int high1,  int low2, int high2, 
         std::function<void(int, int)>  &&lambda, int numThreads) {

         }

