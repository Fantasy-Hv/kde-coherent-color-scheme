#include <iostream>
#include "ColorReader.h"
#include "ConfigLoader.h"

using namespace std;
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <source_config> <target_config> " << endl;
        return 1;
    }
    Logger::stdout_line("write config to " + string(argv[2]) ) ;
    ConfigLoader config_loader(argv[1],argv[2]);
    config_loader.override();

    return 0;
}