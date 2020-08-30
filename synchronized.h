/*
 * synchronized.h
 *
 *  Created on: 30 Aug 2020
 *      Author: sergeynasekin
 */

#ifndef SYNCHRONIZED_H_
#define SYNCHRONIZED_H_
#pragma once

#include <mutex>
using namespace std;

template<typename T>
class Synchronized {
	/*
	 * A template wrapper class for objects of type T to
	 * guarantee their thread safety. To access the value
	 * of the wrapped object one has to access the field
	 * ref_to_value of the Access class which is guarded
	 * with a mutex
	 *
	 * */
public:
	explicit Synchronized(T initial = T()) :
			value(move(initial)) {
	}

	struct Access {
		T& ref_to_value;
		lock_guard<mutex> guard;
	};

	Access GetAccess() {
		return {value, lock_guard(m)};
	}

private:
	T value;
	mutex m;
};

#endif /* SYNCHRONIZED_H_ */
