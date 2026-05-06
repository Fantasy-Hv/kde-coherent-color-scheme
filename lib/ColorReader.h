//
// Created by yian on 2026/3/10.
//

#ifndef DY_CONFIG_LOADER_COLOR_UTILS_H
#define DY_CONFIG_LOADER_COLOR_UTILS_H
#define COLOR_COUNT 16

#include <string>
#include <fstream>
#include <algorithm>
#include "Logger.h"
using namespace std;
struct RGB {
    int r, g, b;  // RGB颜色分量，取值范围0-255
};
class ColorReader {
    string color_file;

    int colors[COLOR_COUNT]{};

    bool loaded = false;

private:
    struct HSL {
        double h, s, l;
    };

    /**
     * @brief 限制数值在指定范围内（C++11兼容版本）
     * @param val 要限制的数值
     * @param min_val 最小值
     * @param max_val 最大值
     * @return 限制后的数值
     */
   static double clamp(double val, double min_val, double max_val) {
        return std::max(min_val, std::min(max_val, val));
    }


    /**
     * @brief 将RGB结构体转换为十六进制整数格式（0xRRGGBB）
     * @param rgb RGB颜色结构体
     * @return 十六进制整数表示的颜色值
     */
   static int rgb_to_hex(RGB rgb) {
        return ((rgb.r & 0xFF) << 16) | ((rgb.g & 0xFF) << 8) | (rgb.b & 0xFF);
    }

    /**
     * @brief 将RGB颜色空间转换为HSL（色相、饱和度、亮度）
     * @param rgb RGB颜色结构体，各分量取值范围0-255
     * @return HSL结构体，h(色相): 0-1, s(饱和度): 0-1, l(亮度): 0-1
     */
   static HSL rgb_to_hsl(RGB rgb) {
        HSL hsl;
        double r = rgb.r / 255.0;
        double g = rgb.g / 255.0;
        double b = rgb.b / 255.0;

        double max_val = std::max(r, std::max(g, b));
        double min_val = std::min(r, std::min(g, b));
        double delta = max_val - min_val;

        hsl.l = (max_val + min_val) / 2.0;

        if (delta == 0) {
            hsl.h = 0;
            hsl.s = 0;
        } else {
            hsl.s = hsl.l < 0.5 ? delta / (max_val + min_val) : delta / (2 - max_val - min_val);

            if (max_val == r) {
                hsl.h = (g - b) / delta + (g < b ? 6 : 0);
            } else if (max_val == g) {
                hsl.h = (b - r) / delta +2;
            } else {
                hsl.h = (r - g) / delta + 4;
            }
            hsl.h /= 6.0;
        }

        return hsl;
    }

    /**
     * @brief 将HSL颜色空间转换为RGB
     * @param hsl HSL颜色结构体，h(色相): 0-1, s(饱和度): 0-1, l(亮度): 0-1
     * @return RGB结构体，各分量取值范围0-255
     */
   static RGB hsl_to_rgb(HSL hsl) {
        RGB rgb;
        double h = hsl.h;
        double s = hsl.s;
        double l = hsl.l;

        if (s == 0) {
            rgb.r = rgb.g = rgb.b = static_cast<int>(l * 255);
            return rgb;
        }

        auto hue_to_rgb = [](double p, double q, double t) {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0/6.0) return p + (q - p) * 6 * t;
            if (t < 1.0/2.0) return q;
            if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6;
            return p;
        };

        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;

        rgb.r = static_cast<int>(hue_to_rgb(p, q, h + 1.0/3.0) * 255);
        rgb.g = static_cast<int>(hue_to_rgb(p, q, h) * 255);
        rgb.b = static_cast<int>(hue_to_rgb(p, q, h - 1.0/3.0) * 255);

        return rgb;
    }
    /**
     * @brief 从颜色文件中加载16个颜色值到内存数组
     * @details 读取文件中的十六进制颜色值（支持带#前缀），每行一个颜色
     */
    void load_colors() {
       ifstream file(color_file);
       if (!file.is_open())
           return ;

       string line_content;
       int current_line = 0;
       Logger::stdout_line("reading colors");
       while (getline(file, line_content)) {
           // 跳过空行和注释行
           if (line_content.empty() ) continue;

           try {
               // 移除'#'前缀（如果存在）并将十六进制字符串转换为整数
               size_t start_pos = (line_content[0] == '#') ? 1 : 0;
               colors[current_line] = stoi(line_content.substr(start_pos), nullptr, 16);
           } catch (...) {
               Logger::stdout_line("error when reading color source");
               return ;
           }
           current_line++;
       }
       loaded = true;
   }
public:
    /**
     * @brief 设置颜色源文件路径并加载颜色数据
     * @param file_path 颜色文件的路径，文件包含16行十六进制颜色值
     */
    void setColorSource(string &file_path) {
        color_file = file_path;
        load_colors();
    }
    /**
     * @brief 将十六进制颜色值转换为RGB结构体
     * @param hex 十六进制颜色值（格式：0xRRGGBB）
     * @return RGB结构体，包含r、g、b三个分量（0-255）
     */
    static RGB hex_to_rgb(int hex) {
        return {
            (hex >> 16) & 0xFF,
            (hex >> 8) & 0xFF,
            hex & 0xFF
        };
    }
    /**
     * @brief 构造函数，初始化颜色文件路径
     * @param file_path 颜色文件的路径
     */
    ColorReader(const string& file_path) : color_file(file_path) {

    }

    /**
     * @brief 获取指定索引位置的颜色值（懒加载模式）
     * @param line 颜色索引，范围0-15（对应16种颜色）
     * @return 十六进制颜色值，如果索引越界则返回0（黑色）
     */
   int get_color(int line) {
        // Logger::stdout_line("reading color line"+ to_string(line));
        if (!loaded)
            load_colors();
        if (line < 0 || line >= COLOR_COUNT) {
            return 0; // 如果行号超出范围，返回黑色
        }
        return colors[line];
    }


    /**
     * @brief 设置颜色的亮度值（基于HSL色彩空间）
     * @param rgb_hex 原始颜色的十六进制值（0xRRGGBB）
     * @param value 亮度百分比（例如：50表示50%亮度）
     * @return 调整后的十六进制颜色值
     */
    static int set_value(int rgb_hex, int value) {
        // Logger::stdout_line("setting color value");
        RGB rgb = hex_to_rgb(rgb_hex);
        HSL hsl = rgb_to_hsl(rgb);

        // 设置亮度（value参数为百分比，例如：50 = 50%）
        hsl.l = clamp(value / 100.0, 0.0, 1.0);

        RGB new_rgb = hsl_to_rgb(hsl);
        return rgb_to_hex(new_rgb);
    }
    /**
     * @brief 设置颜色的饱和度（基于HSL色彩空间）
     * @param rgb_hex 原始颜色的十六进制值（0xRRGGBB）
     * @param value 饱和度百分比（例如：50表示50%饱和度）
     * @return 调整后的十六进制颜色值
     */
    static int set_saturation(int rgb_hex, int value) {
        // Logger::stdout_line("setting color saturation");
        RGB rgb = hex_to_rgb(rgb_hex);
        HSL hsl = rgb_to_hsl(rgb);

        // 设置饱和度（value参数为百分比，例如：50 = 50%）
        hsl.s = clamp(value / 100.0, 0.0, 1.0);

        RGB new_rgb = hsl_to_rgb(hsl);
        return rgb_to_hex(new_rgb);

    }
};
#endif //DY_CONFIG_LOADER_COLOR_UTILS_H