// implement your solution here



#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>

// split string p_line into a vector of strings (words)
// the delimiters are 1 or more whitespaces

std::vector<std::string>
split( const std::string & p_line)
{
  auto line = p_line + " ";
  bool in_str = false;
  std::string curr_word = "";
  std::vector<std::string> res;
  for( auto c : line) {
    if( isspace(c)) {
      if( in_str)
        res.push_back(curr_word);
      in_str = false;
      curr_word = "";
    }
    else {
      curr_word.push_back(c);
      in_str = true;
    }
  }
  return res;
}

// global variable used in stdin_readline()
unsigned char buffer[1024*1024]; //// 1MB sotrage to hold results of read()
int buff_size = 0; // stores how many characters are stored in buffer
int buff_pos = 0; // position in bufffer[] where we extract next char

// returns the next byte from stdin, or -1 on EOF
// returns characters stored in buffer[] until it is empty
// when buffer[] gets empty, it is replenished by calling read()

int 
fast_read() 
{
   // check if buffer[] is empty
  if (buff_pos >= buff_size) {
    // buffer is empty, let's replenish it
    buff_size = read(STDIN_FILENO, buffer, sizeof(buffer));
     // detect EOF
    if (buff_size <= 0) return -1;
     // reset position from where we'll return characters
    buff_pos = 0;
  }
  // return the next character from the buffer and update position
  return buffer[buff_pos++];
}

// reads in a line from STDIN
// reads until \n or EOF and result includes \n if present
// returns empty string on EOF

std::string
stdin_readline()
{
  std::string result;
  while (true) {
    int c = fast_read();
    if(c == -1) break;
    if(c == '\n') {
      if(result.size() > 0) break;
    }
    else {
      result.push_back(c);
    }
  }
  return result;
}

// returns true if a word is palindrome
// palindrome is a string that reads the same forward and backward
//    after converting all characters to lower case

bool
is_palindrome( const std::string & s)
{
  for( size_t i = 0 ; i < s.size() / 2 ; i ++)
    if( tolower(s[i]) != tolower(s[s.size()-i-1]))
      return false;
  return true;
}

// Returns the longest palindrome on standard input.
// In case of ties for length, returns the first palindrome found.
//
// Algorithm:
// Input is broken into lines, each line into words, and each word
// is checked to see if it is palindrome, and if it is, whether it
// is longer than the largest palindrome encountered so far.
//
// A word is a sequence of characters separated by white space
// white space is whatever isspace() says it is
//    i.e. ' ', '\n', '\r', '\t', '\n', '\f'

std::string
get_longest_palindrome()
{
  std::string max_pali;
  while(1) {
    std::string line = stdin_readline();
    if( line.size() == 0) break;
    auto words = split( line);
    for( auto word : words) {
      if( word.size() <= max_pali.size())
        continue;
      if( is_palindrome(word))
        max_pali = word;
    }
  }
  return max_pali;
}

int
main()
{
  std::string max_pali = get_longest_palindrome();
  printf("Longest palindrome: %s\n", max_pali.c_str());
  return 0;
}
