//
// Created by yian on 2026/5/6.
//
using namespace std;
#include <map>
#include <string>
#include <fstream>
#ifndef DY_CONFIG_LOADER_UTILS_H
#define DY_CONFIG_LOADER_UTILS_H
/**
 * @brief 去除字符串首尾的空白字符（包括空格、制表符、回车、换行）
 * @param str 要处理的原始字符串
 * @return 去除首尾空白后的字符串，如果全为空白则返回空字符串
 */
inline string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

/**
 * @brief 按指定分隔符分割字符串
 * @param str 要分割的原始字符串
 * @param separator 分隔符字符串
 * @return 分割后的字符串向量
 * @details 例如：split("a,b,c", ",") 返回 ["a", "b", "c"]
 */
 inline vector<string> split(const string& str, const string& separator) {
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




/**
     * @brief 解析INI格式的配置文件为嵌套映射表
     * @param filename 配置文件路径
     * @return 配置映射表，结构为 [section][key] = value
     * @details 支持节标记[SectionName]、键值对key=value、注释(#或;开头)
     *          特殊行：以'/'开头的行为颜色源文件路径，以'-'开头的行被忽略
     */
inline map<string, map<string, string>> parse_ini_config(const string& filename) {
    map<string, map<string, string>> config;
    ifstream file(filename);
    string line;
    string current_section;

    while (getline(file, line)) {
        line = trim(line);

        // 跳过空行和注释行（#或;开头）
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        // 检查节标题 [SectionName]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            current_section = line.substr(1, line.length() - 2);
            continue;
        }

        // 解析键值对 key=value
        size_t pos = line.find('=');
        if (pos != string::npos && !current_section.empty()) {
            string key = trim(line.substr(0, pos));
            string value = trim(line.substr(pos + 1));
            config[current_section][key] = value;
        }
    }

    file.close();
    return config;
}

/**
     * @brief 将配置映射表写入目标文件（INI格式）
     * @param config_set 要写入的配置映射表，结构为 [section][key] = value
     * @details 使用截断模式打开文件，完全覆盖原有内容
     */
inline void write_ini_config(const map<string, map<string, string>>& config_set,const string& target_file_path) {
    ofstream target_writer(target_file_path,ios_base::trunc);
    if (target_writer.is_open()) {
        for (const auto &section : config_set) {
            target_writer << "[" << section.first << "]" << endl; // 写入节名称
            for (const auto &key_value : section.second) {
                target_writer << key_value.first << "=" << key_value.second << endl; // 写入键值对
            }
        }
    }
}


#endif //DY_CONFIG_LOADER_UTILS_H