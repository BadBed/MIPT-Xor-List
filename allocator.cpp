#include <algorithm>
#include <vector>
#include <memory>
#include "allocator.h"
#include "smallfunctions.h"

//------------------------------------------------------------------------------------

RealAllocator::RealAllocator():
	_current_full_size(0),
	_current_free_size(0),
	_current_ptr(nullptr)
{}

RealAllocator::~RealAllocator() {
	for (auto &_page : _pages) {
		free(_page);
	}
}

void RealAllocator::new_page(size_t min_size) {
	_current_full_size = std::max((size_t)1, div_ceil(min_size, _PAGE_SIZE))*_PAGE_SIZE;
	_current_free_size = _current_full_size;
	_current_ptr = malloc(_current_full_size);
	_pages.push_back(_current_ptr);
}

void* RealAllocator::alloc_on_current_page(size_t size) {
	void* result = _current_ptr;
	_current_free_size -= size;
	_current_ptr = increase_ptr(_current_ptr, (int)size);

	return result;
}

void* RealAllocator::allocate(size_t align, size_t size) {
	std::align(align, size, _current_ptr, _current_free_size);

	if (size > _current_free_size) {
		new_page(size);
	}

	return alloc_on_current_page(size);
}

