#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <array>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "bitboard.h"
#include "movetables.h"
#include "types.h"

namespace Dagor {

class Move {
 public:
  Square::t start;
  Square::t end;
  Piece::t promotion;
  std::uint8_t flags;

  Move(Square::t start, Square::t end, Piece::t promotion = 0)
      : start{start}, end{end}, promotion{promotion}, flags{0} {}

  explicit Move(std::string const &algebraic);
};

const Move wkCastle{Square::e1, Square::g1};
const Move wqCastle{Square::e1, Square::c1};
const Move bkCastle{Square::e8, Square::g8};
const Move bqCastle{Square::e8, Square::c8};

inline bool operator==(Move const &a, Move const &b) {
  return a.start == b.start && a.end == b.end && a.promotion == b.promotion &&
         a.flags == b.flags;
}

class GameState;

struct UndoInfo {
  Piece::t piece;
  Piece::t capture;
  Square::t start;
  Square::t end;
  Square::t enPassant;
  CastlingRights::t castlingRights;
  std::uint8_t uneventfulHalfMoves;
  /// @brief Flags marking special Moves:
  ///
  /// - `0`: a normal move
  /// - `1, 2, 3, 4`: a castle move
  /// - `5` an en passant capture
  MoveFlags::t flags;

  UndoInfo(const GameState &state, const Move &move);
};

class GameState {
 public:
  static inline const std::string startingPosition =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  std::array<BitBoards::BitBoard, Piece::all.size()> pieces;
  std::array<BitBoards::BitBoard, Color::all.size()> colors;
  std::stack<UndoInfo> undoStack;
  std::uint8_t uneventfulHalfMoves;
  CastlingRights::t castlingRights;
  Square::t enPassantSquare;
  Color::t next;

  GameState()
      : pieces(),
        colors(),
        undoStack(),
        uneventfulHalfMoves{0},
        castlingRights{CastlingRights::none},
        enPassantSquare{Square::noSquare},
        next{Color::white} {
    parseFenString(startingPosition);
  }

  explicit GameState(std::string const &fen)
      : pieces(),
        colors(),
        undoStack(),
        uneventfulHalfMoves{0},
        castlingRights{CastlingRights::none},
        enPassantSquare{Square::noSquare},
        next{Color::white} {
    parseFenString(fen);
  }

  BitBoards::BitBoard bitboardFor(Piece::t piece, Color::t color) const {
    return pieces[piece] & colors[color];
  }

  Piece::t getPiece(Square::t square) const {
    for (Piece::t type : Piece::all) {
      if (pieces[type].isSet(square)) {
        return type;
      }
    }
    return Piece::empty;
  }

  Color::t getColor(Square::t square) const {
    if (colors[Color::black].isSet(square)) return Color::black;
    if (colors[Color::white].isSet(square)) return Color::white;
    return Color::empty;
  }

  BitBoards::BitBoard occupancy() const {
    return colors[Color::white] | colors[Color::black];
  }

  inline Color::t us() { return next; }
  inline Color::t them() { return Color::opponent(us()); }

  BitBoards::BitBoard getMoves(Piece::t piece, Color::t color,
                               Square::t square) const;
  BitBoards::BitBoard getMoves(Piece::t piece, Color::t color, Square::t square,
                               BitBoards::BitBoard occupancy) const;
  BitBoards::BitBoard getAttacks(Square::t square, Color::t color) const;
  BitBoards::BitBoard getAttacks(Square::t square, Color::t color,
                                 BitBoards::BitBoard occupancy) const;

  std::vector<Move> generateLegalMoves() const;

  void executeMove(Move move);
  void undoMove();
  void parseFenString(const std::string &fenString);
};

inline bool operator==(const GameState &a, const GameState &b) {
  return a.pieces == b.pieces && a.colors == b.colors &&
         a.uneventfulHalfMoves == b.uneventfulHalfMoves &&
         a.castlingRights == b.castlingRights &&
         a.enPassantSquare == b.enPassantSquare && a.next == b.next;
}

std::ostream &operator<<(std::ostream &out, const GameState &board);
std::ostream &operator<<(std::ostream &out, const Move &move);

}  // namespace Dagor

#endif
