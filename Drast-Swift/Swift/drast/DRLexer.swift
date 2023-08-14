//
//  DRLexer.swift
//  drast
//
//  Created by Ashwin Paudel on 2022-11-04.
//  Copyright © 2022 AX. All rights reserved.
//

import Foundation

class DRLexer {
    var contents: String
    var pos = DRLocation(line: 1, column: 0)
    
    private var index = 0
    
    static private var keywordMap: [String: DRTokenType] = [
        "import": .import,
        "class": .class,
        "struct": .struct,
        "func": .func,
        "enum": .enum,
        "let": .let,
        "var": .var,
        "if": .if,
        "else": .else,
        "while": .while,
        "for": .for,
        "in": .in,
        "break": .break,
        "continue": .continue,
        "self": .self,
        "return": .return,
        
        "int": .int,
        "float": .float,
        "string": .string
    ]
    
    init(contents: String) {
        self.contents = contents
    }
    
    public func lex() {
        while index < contents.count - 1 {
            print(getToken())
        }
    }
    
    public func getToken() -> DRToken {
        skipWhitespace()
        let peek = peek()
        
        switch currentCharacterSafe() {
        case "a" ... "z",
            "A" ... "Z":
            return identifier()
        case "0" ... "9":
            return number()
        case "?":
            return retTok(.question)
        case ";":
            return retTok(.ooSemicolon)
        case "(":
            return retTok(.ooOpenBracket)
        case ")":
            return retTok(.ooCloseBracket)
        case "[":
            return retTok(.ooOpenSquare)
        case "]":
            return retTok(.ooCloseSquare)
        case "{":
            return retTok(.ooOpenCurly)
        case "}":
            return retTok(.ooCloseCurly)
        case ",":
            return retTok(.ooComma)
        case ".":
            return retTok(.ooPeriod)
        case "\"":
            return string()
        case "$":
            return retTok(.ooDollar)
        case "#":
            return retTok(.ooHashtag)
        case "@":
            return retTok(.ooAt)
        case "\\":
            return retTok(.ooBackslash)
        case "`":
            return retTok(.ooBacktick)
        case ":" where peek == "=":
            return retDoubleTok(.ooDeclareEqual)
        case ":":
            return retTok(.ooColon)
        case "<" where peek == "=":
            return retDoubleTok(.lessThanEqual)
        case "<":
            return retTok(.lessThan)
        case ">" where peek == "=":
            return retDoubleTok(.greaterThanEqual)
        case ">":
            return retTok(.greaterThan)
        case "=" where peek == "=":
            return retDoubleTok(.equalEqual)
        case "=":
            return retTok(.equal)
        case "!" where peek == "=":
            return retDoubleTok(.notEqual)
        case "!":
            return retTok(.not)
        case "+" where peek == "=":
            return retDoubleTok(.opAddEqual)
        case "+":
            return retTok(.opAdd)
        case "-" where peek == "=":
            return retDoubleTok(.opSubEqual)
        case "-" where peek == ">":
            return retDoubleTok(.opArrow)
        case "-":
            return retTok(.opSub)
        case "*" where peek == "=":
            return retDoubleTok(.opMulEqual)
        case "*":
            return retTok(.opMul)
        case "/" where peek == "=":
            return retDoubleTok(.opDivEqual)
        case "/" where peek == "/":
            skipLineComment()
            return getToken()
        case "/" where peek == "*":
            skipBlockComment()
            return getToken()
        case "/":
            return retTok(.opDiv)
        case "%" where peek == "=":
            return retDoubleTok(.opModEqual)
        case "%":
            return retTok(.opMod)
        case "\0":
            return retTok(.end)
        default:
            // TODO: Implement Error Reader
            fatalError("Invalid Character: `\(currentCharacter())`") /*\(/*pos*/)*/
        }
    }
}

extension DRLexer {
    private func identifier() -> DRToken {
        let start = index
        
        while alphaNumeric() { advance() }
                
        let word = String(contents[start..<index])
        
        let type = DRLexer.keywordMap[word]
        
        return .init(type: type ?? .identifier, value: word, pos: pos)
    }
    
    private func number() -> DRToken {
        let start = index
        
        while numberic() { advance() }
        
        let word = String(contents[start..<index])
        
        // TODO: Add support for float type
        
        return .init(type: .int, value: word, pos: pos)
    }
    
    private func string() -> DRToken {
        let start = index
        advance()
        
        while currentCharacter() != "\"" { advance() }
        advance()
        
        let word = String(contents[start..<index])
        
        return .init(type: .string, value: word, pos: pos)
    }
    
    
    
    private func skipWhitespace() {
        guard !isAtEnd() else { print("sfa"); return }
        while currentCharacterSafe() == " " || currentCharacterSafe() == "\n" {
            advance()
        }
    }
    
    public func isAtEnd() -> Bool {
        return index > contents.count - 1
    }
    
    func skipLineComment() {
        while !isAtEnd() && currentCharacter() != "\n" {
            advance()
        }
    }
    
    func skipBlockComment() {
        var nesting = 1
        
        advance()
        
        while nesting > 0 {
            if peek() == "\0" {
                fatalError("Unterminated block comment")
            }
            
            if currentCharacter() == "/" && peek() == "*" {
                advance()
                nesting += 1
            } else if currentCharacter() == "*" && peek() == "/" {
                advance()
                nesting -= 1
            }
            
            advance()
        }
    }
}

extension DRLexer {
    private func advance() {
        index += 1
    }
    
    private func peek(_ i: Int = 1) -> Character {
        guard !isAtEnd() else { return "\0" }
        return contents[index + i]
    }
    
    func currentCharacter() -> Character {
        return contents[index]
    }
    
    func currentCharacterSafe() -> Character {
        guard !isAtEnd() else { return "\0" }
        return contents[index]
    }
    
    private func retTok(_ t: DRTokenType) -> DRToken {
        let oldPos = pos
        let tok = DRToken(type: t, pos: oldPos)
        advance()
        
        return tok
    }
    
    private func retDoubleTok(_ t: DRTokenType) -> DRToken {
        let oldPos = pos
        let tok = DRToken(type: t, pos: oldPos)
        advance()
        advance()
        
        return tok
    }
}

extension DRLexer {
    private func alphaNumeric() -> Bool {
        return currentCharacter().isLetter || currentCharacter().isNumber || currentCharacter() == "_"
    }
    
    private func numberic() -> Bool {
        return currentCharacter().isNumber
    }
}
