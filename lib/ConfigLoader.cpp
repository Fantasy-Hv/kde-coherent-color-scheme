//
// Created by yian on 2026/3/10.
//

#include "ConfigLoader.h"
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

vector<string> split(const string& str, const string& separator) {
    vector<string> result;
    size_t start = 0;
    size_t end = str.find(separator);

    while (end != string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + separator.length();
        end = str.find(separator, start);
    }

    result.push_back(str.substr(start));
    return result;
}

string hex_to_string(int hex_value) {
    const char* hex_chars = "0123456789ABCDEF";
    string result = "#";

    for (int i = 5; i >= 0; i--) {
        result += hex_chars[(hex_value >> (i * 4)) & 0xF];
    }

    return result;
}


