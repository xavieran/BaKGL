/*
 * Main
 */

//#include <loguru.hpp>


#include "FileBuffer.h"
#include "RequestResource.h"

#include "worldFactory.hpp"

#include <boost/optional.hpp>

#include <fstream>   
#include <iostream>   
#include <vector>

#include <getopt.h>

#define LOG_S(level) (std::cout)

void exit_with_help(char *argv[]) {
    fprintf(stderr, "  nothing\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    //loguru::init(argc, argv);
    
    char opt;
    while ((opt = getopt(argc, argv, "fvs:")) != EOF)
        switch (opt) {
        case 'h':
        case '?':
        default:
            exit_with_help(argv);
        }
    
    std::string path{argv[1]};
    auto fb = FileBufferFactory::CreateFileBuffer(path);
    RequestResource  res{};
    res.Load(&fb);
    std::cout << res;
    
    unsigned zone = 1;
    LOG_S(INFO) << "Loading zone: " << zone;
    
    BAK::WorldFactory::LoadWorld(zone, "1211");
    return 0;
}


