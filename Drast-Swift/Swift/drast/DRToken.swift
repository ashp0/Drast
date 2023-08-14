//
//  DRToken.swift
//  drast
//
//  Created by Ashwin Paudel on 2022-11-04.
//  Copyright © 2022 AX. All rights reserved.
//

import Foundation

enum DRTokenType {
    // Keywords
    case `import`
    case `class`
    case `struct`
    case `func`
    case `enum`
    case `let`
    case `var`
    case `if`
    case `else`
    case `while`
    case `for`
    case `in`
    case `break`
    case `continue`
    case `self`
    case `return`
    
    // Types
    case int
    case float
    case string
    
    // Literal
    case identifier
    case ltInt
    case ltFloat
    case ltString
    
    // Operators
    case question // ?
    case lessThan // <
    case lessThanEqual // <=
    case greaterThan // >
    case greaterThanEqual // >=
    case equal       // =
    case equalEqual // ==
    case not       // !
    case notEqual // !=// Other
    case opAdd       // +case eof
    case opAddEqual // +=
    case opSub       // -
    case opSubEqual // -=
    case opArrow // ->
    case opMul       // *
    case opMulEqual // *=
    case opDiv       // /
    case opDivEqual // /=
    case opMod       // %
    case opModEqual // %=

    // Other Operators
    case ooColon        // :
    case ooDeclareEqual // :=
    case ooSemicolon    // ;
    case ooOpenBracket  // (
    case ooCloseBracket // )
    case ooOpenSquare  // [
    case ooCloseSquare // ]
    case ooOpenCurly  // }
    case ooCloseCurly // {
    case ooComma        // ,
    case ooPeriod       // .
    case ooDollar       // $
    case ooHashtag      // #
    case ooAt           // @
    case ooBackslash    // \
    case ooBacktick // `
    
    // Other
    case end


//        // Bitwise Operators
//        BITWISE_AND           // &
//        BITWISE_AND_EQUAL     // &=
//        BITWISE_AND_AND       // &&
//        BITWISE_AND_AND_EQUAL // &&=
//
//        BITWISE_PIPE            // |
//        BITWISE_PIPE_EQUAL      // |=
//        BITWISE_PIPE_PIPE       // ||
//        BITWISE_PIPE_PIPE_EQUAL // ||=
//
//        BITWISE_SHIFT_LEFT       // <<
//        BITWISE_SHIFT_LEFT_EQUAL // <<=
//
//        BITWISE_SHIFT_RIGHT       // >>
//        BITWISE_SHIFT_RIGHT_EQUAL // >>=
//
//        BITWISE_POWER       // ^
//        BITWISE_POWER_EQUAL // ^=
//
//        BITWISE_NOT // ~
//

//        // Other
//        NEW_LINE // \n
//        TAB      // \t
//        T_EOF    // \0
}

struct DRToken {
    var type: DRTokenType
    var value: String?
    var pos: DRLocation
}
