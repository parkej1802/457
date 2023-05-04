/// =========================================================================
/// Written by pfederl@ucalgary.ca in 2023, for CPSC457.
/// =========================================================================
/// You need to edit this file.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the analyzeDir() function as
/// defined in "analyzeDir.h".

#include "analyzeDir.h"

#include <cassert>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <iostream>
#include <sys/resource.h>
#include <sys/time.h>

/// check if path refers to a directory
static bool is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISDIR(buff.st_mode);
}

/// check if path refers to a file
static bool is_file(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISREG(buff.st_mode);
}

/// check if string ends with another string
static bool ends_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}


// ======================================================================
// You need to re-implement this function !!!!
// ======================================================================
//
// analyzeDir(n) computes stats about current directory
//   n = how many words and images to report in restuls
//
// The code below is incomplate/incorrect. Feel free to re-use any of it
// in your implementation, or delete all of it.

int num = 0;
std::string path;
int count = 0;
int nfiles = 0;
std::string largestFilePath;
long largestFileSize = -1;
long ndirs = 1;
long totalFileSize = 0;
std::vector<std::pair<std::string,int>> commonWords;
std::vector<ImageInfo> largestImages;
std::vector<std::string> dirs;
constexpr int MAX_WORD_SIZE = 1024;

std::string 
next_word(FILE * fp)
{
  std::string result;
  bool word = false;
  while(1) {
    int c = fgetc(fp);
    if(c == EOF) break;
    c = tolower(c);
    
    //word must have at least 5 letters and considering the spaces of words and searching in lower case letter
    if(! isalpha(c)) {
      if(word) {
        if(result.size() >= 5) { 
          break;
        }
        else {
          word = false;
          result.assign("");
        }
      }
    } 
    else {
      word= true;
      if(result.size() >= MAX_WORD_SIZE) {
        exit(-1);
      }
      result.push_back(c);
    }
  }
  //less than 5 letter will not be printed
  if (result.size() < 5) {
    result = "";
  }
  
  return result;
}

struct VacantResult {
  /* recursive count of all files */
  int nfiles;
  /* recursive count of all directories */
  int ndirs;
  //vacant dirs
  std::vector<std::string> dirs;
};

VacantResult vacant(const std::string & dir, int num) {
  VacantResult vacantResult = {0, 0, {}};
  auto dirp = opendir(dir.c_str());
  assert(dirp != nullptr);
  for (auto de = readdir(dirp); de != nullptr; de = readdir(dirp)) {
    std::string name = de->d_name;
    if (name == "." or name == "..") continue;
    std::string path = dir + "/" + name;
    if (is_file(path)) {
      //count the number of files
      nfiles++;
      struct stat buff;
      if (0 != stat(path.c_str(), &buff)) {
      }
      else {
        //get the size of the file
        //if we follow largest file size, we get to the largest file path
        long fileSize = long(buff.st_size);
        totalFileSize += fileSize;
        if (fileSize > largestFileSize) {
          largestFileSize = fileSize;
          largestFilePath = path;
        }
      }

    }
    //if it's directory count the number of directory
    if (is_dir(path)) {
      ndirs++;
      VacantResult subvacantResult = vacant(path, num);
      //if the directory has no files it returns the path of the directory
      if (subvacantResult.nfiles == 0){
        dirs.push_back(path);
      } 
    

    }

    //get the largest size of image in directory
    // let's see if this is an image and whether we can manage to get image info
    // by calling an external utility 'identify'

    std::string cmd = "identify -format '%w %h' " + path + " 2> /dev/null";
    std::string img_size;
    auto fp = popen(cmd.c_str(), "r");
    assert(fp);
    char buff[PATH_MAX];
    if( fgets(buff, PATH_MAX, fp) != NULL) {
      img_size = buff;
    }
    int status = pclose(fp);
    if( status != 0 or img_size[0] == '0') {
      img_size = "";
    }
    if( !img_size.empty()) {
      long w, h;
      if (2 == sscanf(img_size.c_str(), "%ld%ld", &w, &h)) {
        ImageInfo image = {path, w, h};
        largestImages.push_back(image);
      }
    }


    //file that ends with txt
    if (ends_with(path, ".txt")) {
      std::unordered_map<std::string,int> hist;
      FILE * fp = fopen(path.c_str(), "r");
      while(1) {
        auto w = next_word(fp);
        if(w.size() == 0) break;
        hist[w] ++;
      }
      fclose(fp);

      // first we place the words and counts into array (with count
      // negative to reverse the sort)
      std::vector<std::pair<int,std::string>> arr;
      for(auto & h : hist)
        arr.emplace_back(-h.second, h.first);
      // if we have more than N entries, we'll sort partially, since
      // we only need the first N to be sorted
      if(arr.size() > size_t(num)) {
        std::partial_sort(arr.begin(), arr.begin() + num, arr.end());
        // drop all entries after the first n
        arr.resize(num);
      } else {
        std::sort(arr.begin(), arr.end());
      }
      for(auto & a : arr)
        commonWords.push_back(std::pair(a.second, -a.first));

      }
  
  }
  closedir(dirp);
  vacantResult.nfiles += nfiles;
  return vacantResult;
}


Results analyzeDir(int n)
{
  
  //checking from current directory
  VacantResult vacantResult = vacant(".", n);

  Results res;
  
  //get the values of directories
  res.largest_file_path = largestFilePath;
  res.largest_file_size = largestFileSize;
  res.n_files = nfiles;
  res.n_dirs = ndirs;
  res.all_files_size = totalFileSize;

  //sorting by width * height = size
  std::sort(largestImages.begin(), largestImages.end(), [](const ImageInfo& a, const ImageInfo& b) {
    return (a.width * a.height) > (b.width * b.height);
  });
  int i = 0;
  //printing n numbers of images ranking by size
  for(const auto& img : largestImages) {
    if(n <= i) {
      break;
    }
    res.largest_images.push_back(img);
    i++;
  }

  //print n numbers of common words ranking by size
  std::sort(commonWords.begin(), commonWords.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
    return a.second > b.second;
  });
  int j = 0;
  for(const auto& word : commonWords) {
    
    if(n <= i) {
      break;
    }
    res.most_common_words.push_back(word);
    j++;
  }

  //print vacant directory
  res.vacant_dirs = dirs;
  return res;
}

