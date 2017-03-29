#pragma once

#ifndef ASSERT
	#include <assert.h>
	#define ASSERT(x) assert(x)
#endif

template <typename T> 
class singleton
{
    static T* s_instance;

public:
	static T* newInstance() {
	return new T();
	}
	static void deleteInstance() {
		T* instance = s_instance;
		s_instance = 0;
		delete instance;
	}
	static T* instance() {
		ASSERT(s_instance && "the singletion object was not construct to access");
		return (s_instance);
	}

protected:
	
	singleton() {
		ASSERT(!s_instance && "the singleton object repeat construct");
		s_instance = (T*)this;
	}
	virtual ~singleton() {
		s_instance = 0;
	}
private:
	singleton(const singleton& source) {}

};

template<typename T>
	T* singleton<T>::s_instance = 0;

