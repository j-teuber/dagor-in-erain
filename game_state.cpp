#include "game_state.h"

#include <stdexcept>
#include <sstream>
#include <vector>

namespace Dagor {

Move::Move(std::string const &algebraic) : start{0}, end{0}, promotion{0}, flags{0} {
  start = Board::index(algebraic[0] - 'a', algebraic[1] - '1');
  end = Board::index(algebraic[2] - 'a', algebraic[3] - '1');
  if (algebraic.size() > 4) {
    promotion = pieceTypeFromChar(algebraic[4]);
  }
}

std::vector<std::string> splitFenFields(std::string const &fenString) {
  std::istringstream iss(fenString);
  std::vector<std::string> fields;
  std::string field;
  while (iss >> field) {
    fields.push_back(field);
  }
  return fields;
}

void GameState::parseFenString(const std::string &fenString) {
  std::vector<std::string> fields = splitFenFields(fenString);
  int file = 0;
  int rank = Board::width - 1;
  for (char c : fields[0]) {
    if (c == ' ') {
      break;
    } else if ('0' < c && c <= '8') {
      file += static_cast<int>(c - '0');
    } else if (c == '/') {
      file = 0;
      rank--;
    } else {
      unsigned color = pieceColorFromChar(c);
      unsigned type = pieceTypeFromChar(c);
      if (type < Piece::noPieces) {
        pieces[type].set_bit(Board::index(file, rank));
        colors[color].set_bit(Board::index(file, rank));
        file++;
      } else {
        throw std::invalid_argument{"unknown character"};
      }
    }
  }
  isWhiteNext = (fields[1][0] == 'w');
  for (char c : fields[2]) {
    switch (c) {
      case 'K':
        castlingRights |= CastlingRights::whiteKingSide;
        break;
      case 'Q':
        castlingRights |= CastlingRights::whiteQueenSide;
        break;
      case 'k':
        castlingRights |= CastlingRights::blackKingSide;
        break;
      case 'q':
        castlingRights |= CastlingRights::blackQueenSide;
        break;
    }
  }
  if (fields[3][0] == '-') {
    enPassantSquare = Board::Square::no_square;
  } else {
    int file = fields[3][0] - 'a';
    int rank = fields[3][1] - '0';
    enPassantSquare = Board::index(file, rank);
  }
  uneventfulHalfMoves = std::stoi(fields[4]);
  moveCounter = std::stoi(fields[5]);
}

std::ostream &operator<<(std::ostream &out, const GameState &board) {
  for (int rank = Board::width - 1; rank >= 0; rank--) {
    out << (rank + 1) << " | ";
    for (int file = 0; file < Board::width; file++) {
      int index = Board::index(file, rank);
      unsigned piece = board.getPiece(index);
      unsigned color = board.getColor(index);
      out << (color >= Color::noColors ? '.' : piecePrintChar(piece, color));
      out << ' ';
    }
    out << '\n';
  }
  out << "    ";
  for (int file = 0; file < Board::width; file++) out << "--";
  out << "\t moves: " << board.moveCounter
      << ", uneventful: " << static_cast<int>(board.uneventfulHalfMoves)
      << ", next: " << (board.isWhiteNext ? "white" : "black");
  out << "\n    ";
  for (int file = 0; file < Board::width; file++)
    out << Board::file_name(file) << ' ';
  out << "\t en passant: "
      << (board.enPassantSquare == Board::Square::no_square
              ? "-"
              : Board::squareName(board.enPassantSquare))
      << ", castling rights: " << static_cast<int>(board.castlingRights);
  out << std::endl;
  return out;
}

std::ostream &operator<<(std::ostream &out, const Move &move) {
  out << Board::squareName(move.start)
      << Board::squareName(move.end);
  if (move.promotion != 0) {
    out << piecePrintChar(move.promotion, Color::white);
  }
  return out;
}

}  // namespace Dagor
