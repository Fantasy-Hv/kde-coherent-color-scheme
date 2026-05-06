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
    /**
     * @brief 打印配置映射表（调试用，当前已禁用）
     * @param map 要打印的配置映射表，结构为 [section][key] = value
     */
    static void printMap(std::map<std::string, map<string,string>> &map) {
        // for (auto &item : map) {
        //     std::cout <<"[" << item.first <<"]" << std::endl;
        //     for (auto &subItem : item.second) {
        //         std::cout << subItem.first << " = " << subItem.second << std::endl;
        //     }
        //     std::cout << std::endl;
        // }
    }
    /**
     * @brief 输出日志信息到标准输出并换行
     * @param str 要输出的日志字符串
     */
    static void stdout_line(const string &str) {
        // std::cout << str << std::endl;
    }
};


#endif //DY_CONFIG_LOADER_LOGGER_H