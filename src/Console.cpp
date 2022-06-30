#include <iostream>
#include "Console.hpp"
#include <vector>

void log(int i)
{
  std::cout << i << std::endl;
}

void log(const char* s)
{
  std::cout << s << std::endl;
}

void log(std::string s)
{
  std::cout << s << std::endl;
}

void log(std::vector<std::vector<int>> v)
{
  for (int i = 0; i < v.size(); i++) {
    for (int j = 0; j < v[i].size(); j++) {
      std::cout << v[i][j] << ' ';
    }
    std::cout << std::endl;
  }
}

void error(std::string s)
{
  std::cout << "ERROR: " << s << std::endl;
}

void error(const char *c)
{
  std::cout << "ERROR: " << c << std::endl;
}
