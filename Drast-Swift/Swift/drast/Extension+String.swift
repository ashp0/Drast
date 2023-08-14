//
//  Extension+String.swift
//  drast
//
//  Created by Ashwin Paudel on 2022-11-04.
//  Copyright © 2022 AX. All rights reserved.
//

import Foundation

extension StringProtocol {
    subscript(offset: Int) -> Character { self[index(startIndex, offsetBy: offset)] }
    subscript(range: Range<Int>) -> SubSequence {
        let startIndex = index(self.startIndex, offsetBy: range.lowerBound)
        return self[startIndex..<index(startIndex, offsetBy: range.count)]
    }
}
