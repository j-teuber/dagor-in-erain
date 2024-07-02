/// @file main.cpp

#include <cstring>
#include <iostream>

#include "search.h"
#include "test.h"
#include "uci.h"

using namespace Dagor;

int main(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "uci") == 0) {
    UCI::universalChessInterface(std::cin, std::cout);
  } else if (strcmp(argv[1], "test") == 0) {
    Test::test();
  } else if (strcmp(argv[1], "run") == 0) {
    // GameState s{"2k5/R3P1B1/3P4/3P3P/6Pn/8/2pn4/2K5 w - - 1 44"};
    //  s.executeMove(Move{"e1c1"});
    //  s.executeMove(Move{"a6c4"});
    //  s.executeMove(Move{"c1b1"});
    //  s.executeMove(Move{"c4a2"});
    // std::cerr << s;
    //  Test::divide(s, 5);
    // std::cerr << Search::search(s);
  }

  return 0;
}
