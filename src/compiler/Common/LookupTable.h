//
// LookupTable.h
// Created by Ashwin Paudel on 2022-04-18.
//
// =============================================================================
//
// Contributed by:
//  - Ashwin Paudel <ashwonixer123@gmail.com>
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the Error class. Which is used by the
/// Token class as a container for the keywords.
///
// =============================================================================
//
// Copyright (c) 2022, Drast Programming Language Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//
// =============================================================================

#ifndef DRAST_LOOKUPTABLE_H
#define DRAST_LOOKUPTABLE_H

#include <algorithm>
#include <utility>
#include <vector>

template <typename Key, typename Value> class LookupTable {
  public:
    using lookup_value = std::pair<Key, Value>;
    using lookup_list = std::vector<lookup_value>;

  private:
    lookup_list _container;

  public:
    using iterator = typename lookup_list::const_iterator;
    using const_iterator = iterator;

    LookupTable(std::initializer_list<lookup_value> init) : _container(init) {
        std::sort(_container.begin(), _container.end());
    }

    LookupTable(lookup_value container) : _container(std::move(container)) {
        std::sort(_container.begin(), _container.end());
    }

    const_iterator begin() const { return _container.begin(); }

    const_iterator end() const { return _container.end(); }

    template <typename K> const_iterator find(const K &key) const {
        const_iterator it = std::lower_bound(
            begin(), end(), key,
            [](const lookup_value &p, const K &key) { return p.first < key; });
        return it != end() && it->first == key ? it : end();
    }

    size_t size() const { return _container.size(); }

    void push_back(const lookup_value &value) {
        _container.push_back(value);
        std::sort(_container.begin(), _container.end());
    }

    void emplace_back(const Key &key, const Value &value) {
        _container.emplace_back(key, value);
        std::sort(_container.begin(), _container.end());
    }

    const lookup_value &operator[](size_t index) const {
        return _container[index];
    }
};

#endif // DRAST_LOOKUPTABLE_H
