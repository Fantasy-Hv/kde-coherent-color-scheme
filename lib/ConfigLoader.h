//
// Created by yian on 2026/3/10.
//

#ifndef DY_CONFIG_LOADER_CONFIG_LOADER_H
#define DY_CONFIG_LOADER_CONFIG_LOADER_H
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Logger.h"
#include "ColorReader.h"
string trim(const string& str);
vector<string> split(const string& str, const string& separator);
string hex_to_string(int hex_value);
using namespace std;
class ConfigLoader {
    private:
        ColorReader color_reader;
    string target_file_path;
    string source_file_path;

    map<string, map<string, string>> parse_config(const string& filename) {
        map<string, map<string, string>> config;
        ifstream file(filename);
        string line;
        string current_section;

        while (getline(file, line)) {
            line = trim(line);

            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            if (line[0]=='/') {
                Logger::stdout_line("read color source path:" + line);
                color_reader = ColorReader(line);
                continue;
            }
            if (line[0]=='-') {
                continue;
            }
            // Check for section header [SectionName]
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                current_section = line.substr(1, line.length() - 2);
                continue;
            }

            // Parse key=value pair
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
    // $(index,value,saturation)
    string map_expression2config_value(const string &expression) {
        vector<string> params = split(expression.substr(2, expression.length()-3), ",");
        int color_index = stoi(params[0]);
        int saturation = stoi(params[2]);
        int value = stoi(params[1]);
        int color = color_reader.get_color(color_index);
        color = ColorReader::set_value(color,  value);
        color = ColorReader::set_saturation(color, saturation);
        struct RGB rgb = ColorReader::hex_to_rgb(color);
        string config_color_value = to_string(rgb.r) + "," + to_string(rgb.g) + "," + to_string(rgb.b);
        return config_color_value;
    }
    void write_config(const map<string, map<string, string>>& config_set) {
        Logger::stdout_line("writing config");
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
    public:
        ConfigLoader(const string& file_name, string  target_file) : color_reader(""), source_file_path(file_name),
                                                                     target_file_path(std::move(target_file)) {
        }


    void override() {
        map<string,map<string,string>> my_config = parse_config(source_file_path);
        map<string,map<string,string>> target_config = parse_config(target_file_path);
        cout  << "read my_config: " << endl;
        Logger::printMap(my_config);
        cout << "read target_config: " << endl;
            Logger::printMap(target_config);
        for (auto &section : my_config) {
            //section
            for (auto &item : section.second) { // key-value
                // Logger::stdout_line("processing config entry: " + section.first + " " + item.first + " " + item.second);
                string config_value = item.second;
                if (item.second[0] == '$') {
                    // $(index,value,saturation)
                    // Logger::stdout_line("mapping color");
                    config_value = map_expression2config_value(item.second);
                }
                // Logger::stdout_line("setting target config [" + section.first + "]" + item.first + "=" + config_value);
                target_config[section.first][item.first] = config_value;
            }
        }
        write_config(target_config);
    }
};


#endif //DY_CONFIG_LOADER_CONFIG_LOADER_H