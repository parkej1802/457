// ======================================================================
// You must modify this file and then submit it for grading to D2L.
// ======================================================================
//
// count_pixels() counts the number of pixels that fall into a circle
// using the algorithm explained here:
//
// https://en.wikipedia.org/wiki/Approximations_of_%CF%80
//
// count_pixels() takes 2 paramters:
//  r         =  the radius of the circle
//  n_threads =  the number of threads you should create
//
// Currently the function ignores the n_threads parameter. Your job is to
// parallelize the function so that it uses n_threads threads to do
// the computation.

#include "calcpi.h"
#include <vector>
#include <pthread.h>

//holding values for thread
struct threadValue {
  int start;
  int end;
  int r;
  uint64_t count;
};

//this is the thread work that it will run each loop
void* threadwork(void* arg) 
{
  threadValue* value = (threadValue*)arg;
  double rsq = double(value->r) * value->r;
  uint64_t count = 0;
  for (double x = 1; x <= value->r; x++) {
    for (double y = value->start; y <= value->end; y++) {
      if (x*x + y*y <= rsq) count ++;
    }
  }
  value->count = count;
  pthread_exit(NULL);
}

uint64_t count_pixels(int r, int n_threads) 
{
  //create two vector size of threads
  //do the counting
  std::vector<pthread_t> threads(n_threads);
  std::vector<threadValue> threadv(n_threads);
  uint64_t count = 0;
  //i is current thread and radius / (number of thread) will divide work for each thread
  for (int i = 0; i < n_threads; i++) {
    int start = i * r / n_threads;
    int end = (i + 1) * r / n_threads - 1;
    //get the values of start, end, r and count for threadwork to be done
    //create thread based on that values
    threadv[i] = {start, end, r, count}; 
    pthread_create(&threads[i], NULL, threadwork, &threadv[i]);
  }

  // wait until all works to be done and add count
  for (int i = 0; i < n_threads; i++) {
    pthread_join(threads[i], NULL);
    count += threadv[i].count; 
  }
  return count * 4 + 1;
}