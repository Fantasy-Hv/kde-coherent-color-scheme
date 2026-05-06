#include <iostream>
#include "ColorReader.h"
#include "ConfigLoader.h"
using namespace std;
/**
 * @brief 程序主入口 -
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码，0表示成功，1表示参数错误
 * @details 用法: ./program <source_config> <target_config>
 *          - source_config: 源配置文件路径（包含颜色映射规则）
 *          - target_config: 目标配置文件路径（将被覆盖的KDE配色文件）
 */
//todo : 需要修改参数，只需要给本程序一个主配置文件，其他信息都从主配置文件中读取
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <main_config> " << endl;
        return 1;
    }
    ConfigLoader config_loader(argv[1]);
    config_loader.override();

    return 0;
}