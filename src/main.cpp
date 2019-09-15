/*
 *
 *
 *
 */

#define LOGURU_WITH_STREAMS 1
#include <loguru.hpp>

#include <getopt.h>

#include <vector>

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

    LOG_F(INFO, "Done");
}


