#include "stringutil.hpp"
#include <iostream>
#include <string>
#include <cstddef>

// Returns last index of x if it is present.
// Else returns -1.
int findLastIndex(std::string& str, char x)
{
    // Traverse from right
    for (int i = str.length() - 1; i >= 0; i--)
        if (str[i] == x)
            return i;

    return -1;
}

std::string pathDir(const std::string& str)
{
  std::size_t found = str.find_last_of("/\\");
  return str.substr(0,found+1);
}

const char *combineChars(const char *a, const char *b)
{
  const char* result = (std::string(a) + std::string(b)).c_str();
  return result;
}
