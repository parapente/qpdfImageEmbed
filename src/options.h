#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <unordered_map>
#include <variant>

std::unordered_map<std::string, std::variant<std::string, int>>
readCLIOptions(int argc, char *argv[]);

#endif // OPTIONS_H