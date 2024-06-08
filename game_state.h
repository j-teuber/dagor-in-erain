#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <array>
#include <iostream>
#include <string>

#include "bitboard.h"
#include "constants.h"
#include "movetables.h"

namespace Dagor {

class Move {
 public:
  std::uint8_t start;
  std::uint8_t end;
  std::uint8_t promotion;
  std::uint8_t flags;

  Move(std::uint8_t start, std::uint8_t end, std::uint8_t promotion)
      : start{start}, end{end}, promotion{promotion}, flags{0} {}

  explicit Move(std::string const& algebraic);
};

inline bool operator==(Move const& a, Move const& b) {
    return a.start == b.start && a.end == b.end && a.promotion == b.promotion && a.flags == b.flags;
}

class GameState {
 public:
  static inline const std::string startingPosition =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  std::array<BitBoards::BitBoard, Piece::noPieces> pieces;
  std::array<BitBoards::BitBoard, Color::noColors> colors;
  std::size_t moveCounter;
  std::uint8_t castlingRights;
  std::uint8_t uneventfulHalfMoves;
  std::uint8_t enPassantSquare;
  bool isWhiteNext;

  GameState()
      : pieces(),
        colors(),
        moveCounter{0},
        castlingRights{0},
        uneventfulHalfMoves{0},
        enPassantSquare{Board::Square::no_square},
        isWhiteNext{true} {
    parseFenString(startingPosition);
  }

  explicit GameState(std::string const& fen)
      : pieces(),
        colors(),
        moveCounter{0},
        castlingRights{0},
        uneventfulHalfMoves{0},
        enPassantSquare{Board::Square::no_square},
        isWhiteNext{true} {
    parseFenString(fen);
  }

  BitBoards::BitBoard bitboardFor(unsigned piece, unsigned color) {
    return pieces[piece] & colors[color];
  }

  unsigned getPiece(int square) const {
    for (unsigned type = 0; type < Piece::noPieces; type++) {
      if (pieces[type].is_set(square)) {
        return type;
      }
    }
    return Piece::noPieces;
  }

  unsigned getColor(int square) const {
    if (colors[Color::black].is_set(square)) return Color::black;
    if (colors[Color::white].is_set(square)) return Color::white;
    return Color::noColors;
  }

  BitBoards::BitBoard getMoves(unsigned piece, unsigned color, unsigned square) {
    auto occupancy = colors[Color::black] & colors[Color::white];
    auto moves = BitBoards::BitBoard();
    switch (piece) {
      case Piece::pawn:
      {int offset = color == Color::white ? Board::CompassOffsets::north
                                           : Board::CompassOffsets::south;
        moves |= BitBoards::BitBoard(1UL << (square + offset)) & ~occupancy;
        moves |= MoveTables::pawnAttacks[color][square] & occupancy;}
        break;
      case Piece::knight:
        moves |= MoveTables::knightMoves[square];
        break;
      case Piece::king:
        moves |= MoveTables::kingMoves[square];
        break;
      case Piece::bishop:
        moves |= MoveTables::bishopHashes[square].lookUp(occupancy);
        break;
      case Piece::rook:
        moves |= MoveTables::rookHashes[square].lookUp(occupancy);
        break;
      case Piece::queen:
        moves |= MoveTables::bishopHashes[square].lookUp(occupancy);
        moves |= MoveTables::rookHashes[square].lookUp(occupancy);
        break;

      default:
          return {};
    }
    return moves & ~colors[color];
  }

  bool isSquareAttacked(int square, int color) {
    auto attackers = BitBoards::BitBoard();
    for (unsigned piece = 0; piece < pieces.size(); piece++) {
      attackers |= getMoves(piece, color, square) & pieces[piece];
    }
    return !attackers.is_empty();
  }

  void executeMove(Move move);
  void parseFenString(const std::string &fenString);
};

std::ostream &operator<<(std::ostream &out, const GameState &board);
std::ostream &operator<<(std::ostream &out, const Move &move);

}  // namespace Dagor

#endif
