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
#include "ioutils.h"
#include "DycolorConfigKeys.h"
#include "KdeColorsLoader.h"
using namespace std;
class ConfigLoader {
    private:
    ColorReader *color_reader;
    map<string, map<string, string>> main_config;

    public:

        /**
         * @brief 构造函数，初始化配置加载器
         * */
        ConfigLoader(const string& main_config_file) :color_reader(nullptr) {;
            main_config = parse_ini_config(main_config_file);
            color_reader =new ColorReader(main_config[DYCOLOR_SECTION][WAL_COLOR_PATH_KEY]);
        }
    /**
         * @brief 析构函数，释放动态分配的内存
         * */
    ~ConfigLoader() {
            delete color_reader;
            color_reader = nullptr;
        }


    void override() {
            //注入kde .colors文件;
            KdeColorsLoader kde_colors_loader(&*color_reader
                ,main_config[KDE_SECTION][TARGET_KDE_COLOR_FILE_PATH1_KEY]
                ,main_config[KDE_SECTION][TARGET_KDE_COLOR_FILE_PATH2_KEY]
                ,main_config[KDE_SECTION][KDE_COLOR_MAP_PATH_KEY]);
            kde_colors_loader.overload();
        //todo： 注入 kitty 配置文件
    }
};
#endif //DY_CONFIG_LOADER_CONFIG_LOADER_H