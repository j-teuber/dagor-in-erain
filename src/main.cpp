/// @file main.cpp

#include <iostream>
#include <cstring>

#include "uci.h"
#include "test.h"

using namespace Dagor;

int main(int argc, char *argv[]) {
    if (argc < 2 || strcmp(argv[1], "uci") == 0) {
        UCI::universalChessInterface(std::cin, std::cout);
    } else if (strcmp(argv[1], "test") == 0) {
        Test::test();
    }




  return 0;
}
