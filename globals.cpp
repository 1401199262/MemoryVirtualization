#include "globals.h"

static void _split(const std::string& s, char delim,
    std::vector<std::string>& elems) {
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    _split(s, delim, elems);
    return elems;
}


std::string extract(std::string& values, int index, char delim = ' ') {
    if (values.length() == 0)
        return std::string("");

    std::vector<std::string> x = split(values, delim);
    try {
        return x.at(index);
    }
    catch (const std::out_of_range& e) {
        return std::string("");  // 要是访问超出范围的元素，我们就返回空串
    }
}

#pragma warning(disable:4996)

std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}
// convert wstring to string 
std::string to_byte_string(const std::wstring& input)
{
    //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

/*
std::string.find：从前往后查找子串或字符出现的位置。
std::string.rfind：从后往前查找子串或字符出现的位置。
*/