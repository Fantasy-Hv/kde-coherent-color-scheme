//
// Created by yian on 2026/3/10.
//

#ifndef DY_CONFIG_LOADER_LOGGER_H
#define DY_CONFIG_LOADER_LOGGER_H
#include <iostream>
#include <map>
#include <string>
using namespace std;

class Logger {
public:
    static void printMap(std::map<std::string, map<string,string>> &map) {
        // for (auto &item : map) {
        //     std::cout <<"[" << item.first <<"]" << std::endl;
        //     for (auto &subItem : item.second) {
        //         std::cout << subItem.first << " = " << subItem.second << std::endl;
        //     }
        //     std::cout << std::endl;
        // }
    }
    static void stdout_line(const string &str) {
        std::cout << str << std::endl;
    }
};


#endif //DY_CONFIG_LOADER_LOGGER_H