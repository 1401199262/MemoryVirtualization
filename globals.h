#pragma once
#include <iostream>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include <cmath>
#include <vector>
#include <thread>

#include <tchar.h>
#include <sstream>
#include <algorithm>
#include <unordered_map>  
#include <map>
#include <fstream>
#include <codecvt>
#include <mutex>

std::vector<std::string> split(const std::string& s, char delim);
std::string extract(std::string& values, int index, char delim );

std::wstring to_wide_string(const std::string& input);
std::string to_byte_string(const std::wstring& input);
