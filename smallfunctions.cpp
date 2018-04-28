#include "smallfunctions.h"

void* increase_ptr(void* ptr, int delta) {
	return static_cast<void*>(static_cast<char*>(ptr) + delta);
}

int ptr_difference(void* p, void* q) {
    return (int)(static_cast<char*>(p) - static_cast<char*>(q));
}

YException::YException(const std::string& str) noexcept :
        _str(str)
{}

const char* YException::what() const noexcept {
    return _str.c_str();
}
