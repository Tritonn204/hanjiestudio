#ifndef Console_hpp
#define Console_hpp

#include <iostream>
#include <vector>

void log(int i);
void log(const char *s);
void log(std::string s);
void log(std::vector<std::vector<int>> v);
void log(std::vector<int> v);

void error(std::string s);
void error(const char *s);

#endif //Console_hpp
