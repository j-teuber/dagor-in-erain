/// @file main.cpp

#include <cstring>
#include <iostream>

#include "test.h"
#include "uci.h"

using namespace Dagor;

int main(int argc, char *argv[]) {
  if (argc < 2 || strcmp(argv[1], "uci") == 0) {
    UCI::universalChessInterface(std::cin, std::cout);
  } else if (strcmp(argv[1], "test") == 0) {
    Test::test();
  } else if (strcmp(argv[1], "run") == 0) {
    GameState s{"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "};
    // s.executeMove(Move{"e1d2"});
    // s.executeMove(Move{"d8e8"});
    // s.executeMove(Move{"d7c8r"});
    // s.executeMove(Move{"e8c8"});
    Test::divide(s, 5);
  }

  return 0;
}
