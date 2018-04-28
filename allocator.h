#pragma once
#include <memory>
#include <list>
#include <cstddef>
#include "smallfunctions.h"

class RealAllocator {
public:
	RealAllocator();
	~RealAllocator();

	void* allocate(size_t align, size_t size);
private:
	static constexpr size_t _PAGE_SIZE = 4096;

	void new_page(size_t min_size);
	void* alloc_on_current_page(size_t size);

	size_t _current_full_size;
	size_t _current_free_size;
	void* _current_ptr;
	std::list<void*> _pages;
};

template <typename T>
class StackAllocator {
public:
	StackAllocator();
	~StackAllocator() = default;
	template <typename U>
	explicit StackAllocator(const StackAllocator<U>&);

	T* allocate(size_t size);
	void deallocate(T* ptr, size_t size);

	template <typename U>
	void destroy(U* ptr);

	template<typename U, class... Args>
	void construct(U* ptr, Args&&... args);

	template<typename U>
	struct rebind { typedef StackAllocator<U> other; };

	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;

private:
	std::shared_ptr<RealAllocator> _real_allocator;
};

//******************************************************************

template <typename T>
StackAllocator<T>::StackAllocator() {
    _real_allocator = std::make_shared<RealAllocator>();
}

template <typename T>
template <typename U>
StackAllocator<T>::StackAllocator(const StackAllocator<U>&) {
    _real_allocator = std::make_shared<RealAllocator>();
}

template <typename T>
T* StackAllocator<T>::allocate(size_t size) {
    return static_cast<T*>(_real_allocator->allocate(alignof(T), size*sizeof(T)));
}

template <typename T>
void StackAllocator<T>::deallocate(T* ptr, size_t size) {}

template <typename T>
template <typename U>
void StackAllocator<T>::destroy(U *ptr) {
	ptr->~U();
}

template <typename T>
template<typename U, class... Args>
void StackAllocator<T>::construct(U* ptr, Args&&... args ) {
	::new((void *)ptr) U(std::forward<Args>(args)...);
};
