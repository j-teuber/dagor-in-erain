#include "uci.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "game_state.h"
#include "search.h"

namespace Dagor::UCI {

using namespace std::string_view_literals;

std::vector<std::string> splitOnWhitespace(const std::string &input) {
  std::istringstream stream(input);
  std::vector<std::string> result;
  std::string word;

  while (stream >> word) {
    result.push_back(word);
  }

  return result;
}

void universalChessInterface(std::istream &in, std::ostream &out) {
  GameState state{};
  while (true) {
    std::string line;

    std::cerr << "\n\033[1;34m> \033[0m\n";
    std::getline(in, line);
    std::vector<std::string> parts = splitOnWhitespace(line);

    if (parts[0] == "quit") {
      return;
    } else if (parts[0] == "uci") {
      out << "id name Dagor-in-Erain\n";
      out << "id author Jakob Teuber\n";
      out << "uciok\n";
    } else if (parts[0] == "isready") {
      out << "readyok\n";
    } else if (parts[0] == "ucinewgame") {
      // nothing
    } else if (parts[0] == "position") {
      std::size_t movePos = line.find("moves");
      if (parts[1] == "startpos") {
        state = GameState{};
      } else {
        std::size_t beginFen = line.find("fen") + 4;
        state = GameState{line.substr(beginFen, movePos - beginFen)};
      }
      if (movePos != std::string::npos) {
        auto moves = line.substr(movePos + 5);
        for (auto m : splitOnWhitespace(moves)) {
          state.executeMove(Move{m});
        }
      }
    } else if (parts[0] == "go") {
      auto move = Search::search(state);
      out << "bestmove " << move << "\n";
    } else {
      std::cerr << "discarding unknown command: `" << line << "`\n";
    }
  }
}

}  // namespace Dagor::UCI
