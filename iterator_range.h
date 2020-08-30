/*
 * iterator_range.h
 *
 *  Created on: 30 Aug 2020
 *      Author: sergeynasekin
 */

#ifndef ITERATOR_RANGE_H_
#define ITERATOR_RANGE_H_
#pragma once

#include <algorithm>
using namespace std;

template<typename It>
class IteratorRange {
public:
	IteratorRange(It first, It last) :
			first(first), last(last) {
	}

	It begin() const {
		return first;
	}

	It end() const {
		return last;
	}

	size_t size() const {
		return last - first;
	}

private:
	It first, last;
};

template<typename Container>
auto Head(Container& c, int top) {
	return IteratorRange(begin(c),
			begin(c) + min<size_t>(max(top, 0), c.size()));
}

#endif /* ITERATOR_RANGE_H_ */
