/// -------------------------------------------------------------------------------------
/// this is the only file you need to edit
/// -------------------------------------------------------------------------------------
///
/// (c) 2023, Pavol Federl, pfederl@ucalgary.ca
/// Do not distribute this file.

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <unordered_map>
#include <set>

struct Partition {
  int tag;
  int64_t size, addr;
};

typedef std::list<Partition>::iterator PartitionRef;
typedef std::set<PartitionRef>::iterator TreeRef;

struct scmp {
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const {
    if (c1->size == c2->size)
      return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};

// I suggest you implement the simulator as a class, like the one below.
// If you decide not to use this class, feel free to remove it.
struct Simulator {
  // all partitions, in a linked list
  std::list<Partition> all_blocks;
  // quick access to all tagged partitions
  std::unordered_map<long, std::vector<PartitionRef>> tagged_blocks;
  // sorted partitions by size/address
  std::set<PartitionRef,scmp> free_blocks;
  int64_t pageSize;

  Simulator(int64_t page_size)
  {
    // constructor
    pageSize = page_size;
  }

  void allocate(int tag, int size)
  {
    // Pseudocode for allocation request:
    // - search through the list of partitions from start to end, and
    //   find the largest partition that fits requested size
    //     - in case of ties, pick the first partition found
    // - if no suitable partition found:
    //     - get minimum number of pages from OS, but consider the
    //       case when last partition is free
    //     - add the new memory at the end of partition list
    //     - the last partition will be the best partition
    // - split the best partition in two if necessary
    //     - mark the first partition occupied, and store the tag in it
    //     - mark the second partition free

    PartitionRef largest_partition;
    bool partition_found = false;

    if (all_blocks.empty()) {
      // this is the initial case when we have no partitions at all
      // create a new partition with the size to be multiple of the page size
      // update page requested in results
      // push back this partition to the all_blocks and free_blocks
      int64_t memory = ((size + pageSize - 1) / pageSize) * pageSize;
      all_blocks.push_back({-1, memory, 0});
      free_blocks.insert(all_blocks.begin());
    }

    if(!free_blocks.empty()){
        TreeRef it = free_blocks.begin(); 
        // this will give you an iterator to the largest free partition
        // if this partition is big enough, we set the partition found as true
        // and required largest free partition as free_blocks.begin()
        if ((*it)->size >= size) {
          partition_found = true;
          largest_partition = *it;
        }
    }
    
    // check free partition is not found
    if (!partition_found) {
      PartitionRef last_partition = std::prev(all_blocks.end());
      // if last block in all_partitions is free
      // calculate number of page and size of memory that is needed for block
      // remove last partition from the free_blocks
      if (last_partition->tag == -1) {
        int page_request = 1 + ((size - all_blocks.back().size - 1) / pageSize);
        int block_request = page_request * pageSize;

        free_blocks.erase(std::prev(all_blocks.end()));
        all_blocks.back().size += block_request;
        largest_partition = std::prev(all_blocks.end());
        free_blocks.insert(largest_partition);
      }
      // if its not free, calculate memory
      // add new partition with memory and set largest partition to new partition
      else {
        int page_request = 1 + ((size - 1) / pageSize);
        int block_request = page_request * pageSize;
        all_blocks.push_back({-1, block_request, all_blocks.back().addr + all_blocks.back().size});
        largest_partition = std::prev(all_blocks.end());
        free_blocks.insert(largest_partition);
      }
    }

    // if size of largest partition is greater than requested size
    // create new parition with that size
    // remove the largest partition from free block and update address and size of largest parition
    if (size < largest_partition->size) {
      PartitionRef new_partition = all_blocks.insert(largest_partition, {tag, size, largest_partition->addr});

      free_blocks.erase(largest_partition);
      largest_partition->addr += size;
      largest_partition->size -= size;

      // check if next partition is free then merge
      if (std::next(largest_partition) != all_blocks.end()) {
        if (std::next(largest_partition)->tag == -1) { 
          largest_partition->size += std::next(largest_partition)->size; 
          free_blocks.erase(std::next(largest_partition));
          all_blocks.erase(std::next(largest_partition)); 
        }
      }

      // add new partition to tagged block and update largest partition
      tagged_blocks[tag].push_back(new_partition); 
      free_blocks.insert(largest_partition); 
    }
    // remove the largest_partition from free block and set tag and add largest partition to tagged block
    else if (size == largest_partition->size) {
      free_blocks.erase(largest_partition);
      largest_partition->tag = tag;
      tagged_blocks[tag].push_back(largest_partition);
    } 
  }

  void deallocate(int tag)
  {
    // Pseudocode for deallocation request:
    // - for every partition
    //     - if partition is occupied and has a matching tag:
    //         - mark the partition free
    //         - merge any adjacent free partitions

    std::vector<PartitionRef> partitions_with_tag = tagged_blocks[tag];
    PartitionRef current;

    for (auto &current : partitions_with_tag) {
      current->tag = -1;

      if (current != all_blocks.begin() && std::prev(current)->tag == -1) {
        // find the previous of the current block
        // do a left merge
        // erase the previous of the current block from all blocks
        current->size += std::prev(current)->size;
        current->addr = std::prev(current)->addr;
        free_blocks.erase(std::prev(current));
        all_blocks.erase(std::prev(current));
      }

      if (current != std::prev(all_blocks.end()) && std::next(current)->tag == -1) {
        // find the next of the current block
        // do a right merge
        // erase next of the current block from all blocks 
        current->size += std::next(current)->size;
        free_blocks.erase(std::next(current));
        all_blocks.erase(std::next(current));
      }
      // insert current block to free_blocks
      free_blocks.insert(current); 
    }
    // erase all blocks associated with the tag in tagged_blocks
    tagged_blocks.erase(tag);
  }

  MemSimResult getStats()
  {
    MemSimResult result;
    if (!free_blocks.empty()) {
      PartitionRef largest_partition = *free_blocks.begin();
      result.max_free_partition_size = largest_partition->size;
      result.max_free_partition_address = largest_partition->addr;
    } 
    else {
        result.max_free_partition_size = 0;
        result.max_free_partition_address = 0;
    }

    // calculate total number of pages requested
    int64_t total_size = 0;
    for (const auto &partition : all_blocks) {
      total_size += partition.size;
    }
    result.n_pages_requested = (total_size + pageSize - 1) / pageSize;

    return result;
  }

  
  void check_consistency()
  {
    // mem_sim() calls this after every request to make sure all data structures
    // are consistent. Since this will probablly slow down your code, I suggest
    // you comment out the call below before submitting your code for grading.
    std::cout << "All_Blocks " << std::endl;
    for (PartitionRef ab = all_blocks.begin(); ab != all_blocks.end(); ab++) {
      std::cout << "tag: " << ab->tag << "  size: " << ab->size << "  addr: " << ab->addr << std::endl;
    }
    std::cout << "Free Blocks" << std::endl;
    for (TreeRef fb = free_blocks.begin(); fb != free_blocks.end(); fb++){
      std::cout << "tag: " << (*fb)->tag << "  size: " << (*fb)->size << "  addr: " << (*fb)->addr << std::endl;
    }
    
    std::cout << "Tagged Blocks" << std::endl;
    for (const auto &tb: tagged_blocks) {
      std::cout << "tag: " << tb.first << std::endl;
    }
  }
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // if you decide to use the simulator class above, you likely do not need
  // to modify the code below at all
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    //sim.check_consistency();
  }
  return sim.getStats();
}

