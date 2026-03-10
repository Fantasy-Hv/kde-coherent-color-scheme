//
// Created by yian on 2026/3/10.
//

#ifndef DY_CONFIG_LOADER_COLOR_UTILS_H
#define DY_CONFIG_LOADER_COLOR_UTILS_H
#define COLOR_COUNT 16

#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "Logger.h"
using namespace std;
struct RGB {
    int r, g, b;
};
class ColorReader {
private:
   string color_file;
   int colors[COLOR_COUNT]{};
   bool loaded = false;

   struct HSL {
        double h, s, l;
    };

    // Clamp value between min and max (C++11 compatible)
   static double clamp(double val, double min_val, double max_val) {
        return std::max(min_val, std::min(max_val, val));
    }



    // Convert RGB struct to hex
   static int rgb_to_hex(RGB rgb) {
        return ((rgb.r & 0xFF) << 16) | ((rgb.g & 0xFF) << 8) | (rgb.b & 0xFF);
    }

    // Convert RGB to HSL
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

    // Convert HSL to RGB
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
    void load_colors() {
       ifstream file(color_file);
       if (!file.is_open())
           return ;

       string line_content;
       int current_line = 0;
       Logger::stdout_line("reading colors");
       while (getline(file, line_content)) {
           // Skip empty lines and comments
           if (line_content.empty() ) continue;

           try {
               // Remove '#' prefix if present and convert hex to int
               Logger::stdout_line(line_content);
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
    void setColorSource(string &file_path) {
        color_file = file_path;
        load_colors();
    }
    // Convert hex RGB to RGB struct
    static RGB hex_to_rgb(int hex) {
        return {
            (hex >> 16) & 0xFF,
            (hex >> 8) & 0xFF,
            hex & 0xFF
        };
    }
    ColorReader(const string& file_path) : color_file(file_path) {

    }

    // Get color from colors file by line number
   int get_color(int line) {
        Logger::stdout_line("reading color line"+ to_string(line));
        if (!loaded)
            load_colors();
        if (line < 0 || line >= COLOR_COUNT) {
            return 0; // Return black color if line number is out of range
        }
        return colors[line];
    }

    // Add value (brightness/lightness) to hex RGB color
   static int add_value(int rgb_hex, int value) {
        RGB rgb = hex_to_rgb(rgb_hex);
        HSL hsl = rgb_to_hsl(rgb);

        // Increase lightness (value parameter is in percentage, e.g., 10 = +10%)
        hsl.l = clamp(hsl.l + value / 100.0, 0.0, 1.0);

        RGB new_rgb = hsl_to_rgb(hsl);
        return rgb_to_hex(new_rgb);
    }

    // Add saturation to hex RGB color
   static int add_saturation(int rgb_hex, int value) {
        RGB rgb = hex_to_rgb(rgb_hex);
        HSL hsl = rgb_to_hsl(rgb);

        // Increase saturation (value parameter is in percentage, e.g., 10 = +10%)
        hsl.s = clamp(hsl.s + value / 100.0, 0.0, 1.0);

        RGB new_rgb = hsl_to_rgb(hsl);
        return rgb_to_hex(new_rgb);
    }
    static int set_value(int rgb_hex, int value) {
        // Logger::stdout_line("setting color value");
        RGB rgb = hex_to_rgb(rgb_hex);
        HSL hsl = rgb_to_hsl(rgb);

        // Set lightness (value parameter is in percentage, e.g., 50 = 50%)
        hsl.l = clamp(value / 100.0, 0.0, 1.0);

        RGB new_rgb = hsl_to_rgb(hsl);
        return rgb_to_hex(new_rgb);
    }
    static int set_saturation(int rgb_hex, int value) {
        // Logger::stdout_line("setting color saturation");
        RGB rgb = hex_to_rgb(rgb_hex);
        HSL hsl = rgb_to_hsl(rgb);

        // Set saturation (value parameter is in percentage, e.g., 50 = 50%)
        hsl.s = clamp(value / 100.0, 0.0, 1.0);

        RGB new_rgb = hsl_to_rgb(hsl);
        return rgb_to_hex(new_rgb);

    }
};
#endif //DY_CONFIG_LOADER_COLOR_UTILS_H