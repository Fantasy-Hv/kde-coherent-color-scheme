//
// Created by yian on 2026/5/6.
//

#ifndef DY_CONFIG_LOADER_KDECOLORSLOADER_H
#define DY_CONFIG_LOADER_KDECOLORSLOADER_H
#include "Logger.h"
#include <string>
#include <map>
#include <vector>

#include "ColorReader.h"
#include "ioutils.h"
using namespace std;

class KdeColorsLoader {
    string source_file_path;
    string target_file_path1;
    string target_file_path2;
    ColorReader *color_reader;
    /**
     * @brief 将颜色表达式映射为实际配置值
     * @param expression 颜色表达式，格式为 $(index,value,saturation[,alpha])
     *                   - index: 颜色索引（0-15）
     *                   - value: 亮度百分比（0-100）
     *                   - saturation: 饱和度百分比（0-100）
     *                   - alpha : 透明度百分比（0-100）
     * @return RGB颜色值的字符串表示，格式为 "r,g,b[,a]"
     * @details 从颜色源读取指定索引的颜色，应用亮度和饱和度设置后转换为RGB格式,其中透明度是可选配置，需要按需转译
     */
    string parse_color_expression(const string &expression) {
        vector<string> params = split(expression.substr(2, expression.length()-3), ",");
        int color_index = stoi(trim(params[0]));
        int value = stoi(trim(params[1]));
        int saturation = stoi(trim(params[2]));

        int color = color_reader->get_color(color_index);
        color = ColorReader::set_value(color,  value);
        color = ColorReader::set_saturation(color, saturation);
        RGB rgb = ColorReader::hex_to_rgb(color);
        string config_color_value = to_string(rgb.r) + "," + to_string(rgb.g) + "," + to_string(rgb.b);

        if (params.size()==4) config_color_value.append("," + trim(params[3]));
        return config_color_value;
    }
    /**
 * @brief 使用源配置覆盖目标配置（核心功能）
 * @details 处理流程：
 *          1. 解析源配置和目标配置文件
 *          2. 遍历源配置的每个节和键值对
 *          3. 如果值是颜色表达式（以$开头），则转换为实际RGB值
 *          4. 将处理后的值写入目标配置映射表
 *          5. 将最终配置写入目标文件
 */
    void over_write_color(map<string,map<string,string>> &my_config_map,string &target_file_path) {
        map<string, map<string, string>> target_config = parse_ini_config(target_file_path);
        mergeIniMap(my_config_map,target_config);
        write_ini_config(target_config,target_file_path);
    }
    void mergeIniMap(map<string, map<string, string>>& prior,map<string, map<string, string>>&  infer ) {
        for (auto &section : prior) {
            //section
            for (auto &item : section.second) { // key-value

                string config_value = item.second;
                if (item.second[0] == '$') {
                    // 处理颜色表达式 $(index,value,saturation[，alpha])
                    config_value = parse_color_expression(item.second);
                }
                infer[section.first][item.first] = config_value;
            }
        }
    }
    public:
    KdeColorsLoader(ColorReader * const color_readerptr,string & target1, string & target2, string & source) : target_file_path1(target1), target_file_path2(target2), source_file_path(source) ,color_reader(nullptr){
        color_reader = color_readerptr;
    }

    void overload() {
        map<string,map<string,string>> my_config = parse_ini_config(source_file_path);
        over_write_color(my_config,target_file_path1);
        over_write_color(my_config,target_file_path2);
    }

};


#endif //DY_CONFIG_LOADER_KDECOLORSLOADER_H