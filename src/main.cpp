/*
 * Main
 */

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

#include <boost/optional.hpp>

#include <boost/archive/binary_iarchive.hpp>                                                    
#include <boost/serialization/string.hpp>                                                       

#include <fstream>   
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
    boost::archive::binary_iarchive iarchive(in);
    //std::string s;
    double s;
    iarchive >> s;
    std::cout << s << '\n';

    LOG_F(INFO, "Done");

    return 0;
}


