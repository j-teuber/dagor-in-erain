#include "test.h"

#include <iostream>

#include "bitboard.h"
#include "game_state.h"
#include "board.h"
#include "constants.h"

namespace Dagor::Test {

    static unsigned tests = 0;
    static unsigned failures = 0;

    template <typename T>
    void assertEquals(T actual, T expected, std::string_view name) {
        tests++;
        std::cout << name << "... ";
        if (actual == expected) {
            std::cout << "Check!\n";
        } else {
            std::cout << "Fail!\n";
            std::cout << "expected:\n" << expected << "\nbut got:\n" << actual;
            failures++;
        }
    }

    void pieceMovement() {
        using namespace MoveTables;
        assertEquals(pawnAttacks[Color::white][Board::c8], {},
                     "Pawn in last row cannot move further");
        assertEquals(pawnAttacks[Color::white][Board::c3], {0xa000000},
                     "Pawn in the center can attack left and right");
        assertEquals(pawnAttacks[Color::white][Board::a3], {0x2000000},
                     "Pawn in the left side has only one attack");

        assertEquals(knightMoves[Board::d5], {0x14220022140000},
                     "Knight in the center has the correct moves");
        assertEquals(knightMoves[Board::a1], {0x20400},
                     "Knight in a corner has only two options");

        assertEquals(kingMoves[Board::b2], {0x70507},
                     "King has eight moves");
        assertEquals(kingMoves[Board::a1], {0x302},
                     "King in a corner has only three options");

        assertEquals(bishopHashes[Board::c4].lookUp({}), {0x4020110a000a1120},
                     "Unobstructed Bishop moves");
        assertEquals(bishopHashes[Board::c4].lookUp({0x840010504008018a}), {0x110a000a0100},
                     "Bishop with blocking pieces");

        assertEquals(rookHashes[Board::c4].lookUp({}), {0x4040404fb040404},
                     "Unobstructed Rook moves");
        assertEquals(rookHashes[Board::c4].lookUp({0x2440000940a200}), {0x404040b040404},
                     "Rook with blocking pieces");
    }

    void moveClass() {
        assertEquals(Move{"a1a3"}, Move{Board::a1, Board::a3, 0},
                     "Moves can be constructed from algebraic notation");
        assertEquals(Move{"a2a1r"}, Move{Board::a2, Board::a1, Piece::rook},
                     "Moves can be constructed from algebraic notation with promotion");
    }

    void test() {
        std::cout << "\nRun Test suits ================================================================\n";
        pieceMovement();
        moveClass();

        std::cout << "\nTests: " << tests << " (" << (tests - failures) << " passed, "
                  << failures << " failed)\n";
    }


}