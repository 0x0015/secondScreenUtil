#pragma once
#include <string>
#include <vector>

std::string readFileIntoString(std::string_view filename);
std::vector<std::string_view> splitString(std::string_view str, std::string_view delim);
