#include "game_state.h"

namespace Dagor {

void executeMove(Move) {}

void GameState::parseFenString(std::string_view fenString) {
  int file = 0;
  int rank = Board::width - 1;
  for (char c : fenString) {
    if ('0' < c && c <= '8') {
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
        throw "unrecognized piece " + c;
      }
    }
  }
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
      << (board.enPassantSquare == Board::Square::no_square ? "-" : "!")
      << ", castling rights: " << static_cast<int>(board.castlingRights);
  out << std::endl;
  return out;
}

}  // namespace Dagor
