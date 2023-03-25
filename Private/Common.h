#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

#define GENERIC_SUCCESS 0
#define GENERIC_FILE_ERROR -1
#define FAILURE_OUTPUT_DIRECTORY -2
#define GENERIC_PARSER_ERROR -3