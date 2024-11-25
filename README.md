# Group 121
Pratyush Gangwar (Solo)

# Github
https://github.com/Pratyush-Gangwar/Assignment-5-SimpleMultiThreader

# Working of 1D multithreading
- struct linear_args
    - The low and high attributes represent the interval of indices [low, high) that the thread corresponding to this struct would operate on. 
    - These attributes are set in such a way that these intervals never overlap. 
    - The lambda attribute is the 'operation' that this thread would perform. 

- linear_thread_func()
    - It has a for-loop that applies linear_args->lambda on each index in the interval [low, high)

- parallel_for(low, high)
    - Firstly, an array storing thread objects and an array storing linear_args are allocated.
    - Then, the total number of elements and chunk size are calculated.

    - It creates numThreads - 1 additional threads.
    - The lower bound for the i-th thread is i * chunkSize and the upper bound is (i + 1) * chunkSize 
    - Each thread executes the same lambda function 

    - The main thread which called parallel_for() also does some work.
    - The lower bound is set to (numThreads - 1) * chunkSize and the upper bound to totalElements. 
    - By setting the upper bound to totalElements, we ensure that no elements are missed if totalElements % numThreads != 0.

    - Finally, we wait for the all the threads to finish and then clean up the allocated arrays.

# Working of 2D multithreading
It's hard to deal with 2D indices such as (row, col) but its way easier to deal with 1D indices such as [low, high). 
Recall that 2D arrays are laid out as 1D arrays in memory - whether in column major or row major format. 
And so, it makes sense to assign linear indices to 2D indices and vice versa. 
The 2D index (row, col) has the 1D index row * num_cols + col and the 1D index linear_idx has the 2D index (linear_idx/num_cols, col = linear_idx % num_cols).

- struct matrix_args
    - The linear_low and linear_high attributes represent the interval of indices [linear_low, linear_high) that the thread corresponding to this struct would operate on. The 2D low and high indices of each thread are converted to linear indices before being stored in here.
    - These attributes are set in such a way that these intervals never overlap. 
    - The lambda attribute is the 'operation' that this thread would perform. 

- matrix_thread_func()
    - It has a for-loop that iterates over the interval [linear_low, linear_high)
    - Each linear index is converted to the corresponding 2D index and then matrix_args->lambda is applied on that 2D index

- parallel_for(low1, high1, low2, high2)
    - Firstly, an array storing thread objects and an array storing matrix_args are allocated.
    - Note that the starting index is (low1, low2) and the ending index is (high1 - 1, high2 - 1) (included). 
    - These are converted to linear indices and then the total number of elements and chunk size are calculated.

    - It creates numThreads - 1 additional threads.
    - The linear lower bound for the i-th thread is i * chunkSize and the linear upper bound is (i + 1) * chunkSize 
    - Each thread executes the same lambda function 

    - The main thread which called parallel_for() also does some work.
    - The linear lower bound is set to (numThreads - 1) * chunkSize and the linear upper bound to totalElements. 
    - By setting the upper bound to totalElements, we ensure that no elements are missed if totalElements % numThreads != 0.

    - Finally, we wait for the all the threads to finish and then clean up the allocated arrays.