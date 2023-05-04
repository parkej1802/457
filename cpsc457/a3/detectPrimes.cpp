/// ============================================================================
/// Copyright (C) 2023 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// ============================================================================
///
/// You must modify this file and then submit it for grading to D2L.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the detect_primes() function as
/// defined in "detectPrimes.h".

#include "detectPrimes.h"
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>

// C++ barrier class (from lecture notes).
// You do not have to use it in your implementation. If you don't use it, you
// may delete it.
class simple_barrier {
  std::mutex m_;
  std::condition_variable cv_;
  int n_remaining_, count_;
  bool coin_;

  public:
  simple_barrier(int count = 1) { init(count); }
  void init(int count)
  {
    count_ = count;
    n_remaining_ = count_;
    coin_ = false;
  }
  bool wait()
  {
    if (count_ == 1) return true;
    std::unique_lock<std::mutex> lk(m_);
    if (n_remaining_ == 1) {
      coin_ = !coin_;
      n_remaining_ = count_;
      cv_.notify_all();
      return true;
    }
    auto old_coin = coin_;
    n_remaining_--;
    cv_.wait(lk, [&]() { return old_coin != coin_; });
    return false;
  }
};

// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static bool is_prime(int64_t n, int thread_id, int n_threads)
{
  // handle trivial cases
  if (n < 2) return false;
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) return false; // handle multiples of 2
  if (n % 3 == 0) return false; // handle multiples of 3
  // try to divide n by every number 5 .. sqrt(n)
  int64_t i = 5 + 6 * thread_id;
  int64_t max = sqrt(n);
  while (i <= max) {
    if (n % i == 0) return false;
    if (n % (i + 2) == 0) return false;
    i += 6 * n_threads;
  }
  // didn't find any divisors, so it must be a prime
  return true;
}

void thread_function(int thread_id, int n_threads, const std::vector<int64_t> &nums, simple_barrier &barrier,
std::vector<int64_t> &result, std::atomic<std::size_t> &next_index, std::atomic<bool> 
&global_finished, std::atomic<bool> &cancel, std::atomic<int> &threads_count) {
  
  //repeat forever
  while (true) {
    //serial task - pick one thread using barrier
    if (barrier.wait()) {
      cancel = false;
    }
    //get the next number from nums[]
    barrier.wait();

    //if no more numbers left:
    //set global_finished=true to indicate to all threads to quit
    if (global_finished) {
      break;
    }
    
    //divide work for each thread
    //parallel task – executed by all threads, via barrier
    bool isPrime = is_prime(nums[next_index - 1], thread_id, n_threads);

    //if global_finished flag is set, exit thread
    //otherwise do the work assigned above and record per-thread result
    if (!isPrime) {
      cancel = true;
      threads_count++;
    }

    //serial task – pick one thread using barrier
    barrier.wait();

    //combine the per-thread results and update the result[] array if necessary
    if (!cancel && threads_count == 0 && barrier.wait()) {
      result.push_back(nums[next_index - 1]);
    }

    if (barrier.wait()) {
      //no more numbers left then exit
      if (next_index >= nums.size() - 1) {
        global_finished = true;
      } 
      else {
        next_index++;
        threads_count = 0;
      }
    }
  }
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.
std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
  //prepare memory for each thread
  //initialize empty array result[] – this could be a global variable
  //set global_finished = false – make it atomic to be safe
  std::vector<int64_t> result;
  std::atomic<bool> global_finished{false};
  simple_barrier barrier(n_threads);
  std::atomic<size_t> next_index{0};
  std::atomic<bool> cancel{false};
  std::atomic<int> threads_count{0};
  std::vector<std::thread> threads;

  //start N threads, each runs thread_function() on its own memory
  for (int i = 0; i < n_threads; i++) {
    threads.emplace_back(thread_function, i, n_threads, std::cref(nums), std::ref(barrier), std::ref(result),
    std::ref(next_index), std::ref(global_finished), std::ref(cancel), std::ref(threads_count));
  }
  //join N threads
  for (auto &thread : threads) {
    thread.join();
  }
  return result;
}