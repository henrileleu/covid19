#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

template<typename Out>

void split(const std::string &s, char delim, Out result);

std::vector<std::string> split(const std::string &s, char delim);