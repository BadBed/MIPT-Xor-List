#pragma once
#include <string>
#include <exception>

template <typename T>
T div_ceil(T x, T y)  {
    if (x % y) {
        return x / y + 1;
    }
    else {
        return x / y;
    }
}

void* increase_ptr(void* ptr, int delta);

int ptr_difference(void* p, void* q);

class YException : public std::exception {
public:
    const char* what() const noexcept override;
    explicit YException(const std::string& str) noexcept;
private:
    std::string _str;
};

template <typename T>
T* xor_ptr(T* ptr1, T*ptr2) {
    return (T*)((long long)(ptr1) ^ (long long)(ptr2)); // NOLINT
}

template <typename T>
T* xor_ptr(T* ptr1, T* ptr2, T* ptr3) {
    return xor_ptr(ptr1, xor_ptr(ptr2, ptr3));
}
