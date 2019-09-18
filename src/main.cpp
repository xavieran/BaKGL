/*
 * Main
 */

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

//#include <FileManager.h>
#include <FileBuffer.h>

#include <boost/optional.hpp>

#include <fstream>   
#include <iostream>   
#include <vector>

#include <getopt.h>

void exit_with_help(char *argv[]) {
    fprintf(stderr, "  -s=order : Encode single frame with order given\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    
    char opt;
    while ((opt = getopt(argc, argv, "fvs:")) != EOF)
        switch (opt) {
        case 'h':
        case '?':
        default:
            exit_with_help(argv);
        }
    
    std::ifstream in("krondor.001", std::ios::binary);
    assert(in.good());

    FileBuffer fb{0};
    fb.Load(in);

    LOG_F(INFO, "Done");

    return 0;
}


