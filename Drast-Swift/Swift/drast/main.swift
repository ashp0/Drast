//
//  main.swift
//  drast
//
//  Created by Ashwin Paudel on 2022-11-04.
//  Copyright © 2022 AX. All rights reserved.
//

import Foundation

let arguments = CommandLine.arguments

//if arguments.count < 2 {
//    print("Invalid Argument Count")
//    exit(EXIT_FAILURE)
//}

// let file = CommandLine.arguments[1]
let file = "/Users/ashwinpaudel/Desktop/AX/drast_xc/drast/example.drast"

var contents = ""

    do {
//        contents = try String(contentsOf: fileURL, encoding: .utf8)
        contents = try String(contentsOfFile: file, encoding: .utf8)
    }
    catch {
        print(error.localizedDescription)
    }


var filer: String = "class?50 <=world this is my programming language? <"

let lexer = DRLexer(contents: contents)
lexer.lex()
