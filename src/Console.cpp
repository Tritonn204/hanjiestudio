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
  for (size_t i = 0; i < v[i].size(); i++) {
    for (size_t j = 0; j < v.size(); j++) {
      std::cout << v[j][i] << ' ';
    }
    std::cout << std::endl;
  }
}

void log(std::vector<int> v)
{
  for (size_t i = 0; i < v.size(); i++) {
    std::cout << v[i] << ' ';
  }
  std::cout << std::endl;
}

void error(std::string s)
{
  std::cout << "ERROR: " << s << std::endl;
}

void error(const char *c)
{
  std::cout << "ERROR: " << c << std::endl;
}
