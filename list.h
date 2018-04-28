#pragma once
#include <iterator>
#include <type_traits>
#include "smallfunctions.h"

template <typename T, class Alloc>
class XorListIterator;

template <typename T>
struct XorListNode {
public:
	T value;
	XorListNode* ptr;

	template <typename U>
    explicit XorListNode(U&& val): value(std::forward<U>(val)){}
};

template <typename T, class Alloc = std::allocator<T> >
class XorList {
public:
	explicit XorList(const Alloc& alloc = Alloc());
    explicit XorList(size_t count, const T& value = T(), const Alloc& alloc = Alloc());

	XorList(const XorList<T, Alloc>&);
	XorList(XorList<T, Alloc>&&) noexcept;
	~XorList();

	XorList<T, Alloc>& operator=(const XorList<T, Alloc>&);
	XorList<T, Alloc>& operator=(XorList<T, Alloc>&&) noexcept;

	friend class XorListIterator<T, Alloc>;
	typedef XorListIterator<T, Alloc> iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;


	size_t size() const;

	T& back();
	T& front();

    template <typename U> void push_back(U&&);
    template <typename U> void push_front(U&&);
    template <typename U> iterator insert_before(iterator, U&&);
    template <typename U> iterator insert_after(iterator, U&&);

	void pop_back();
	void pop_front();
	void erase(iterator);

	iterator begin();
	iterator end();

private:
    typedef XorListNode<T> node;

    void delete_nodes();
    void insert_node_before(node*, iterator&);

    //Alloc _alloc;
    typedef typename Alloc::template rebind<node>::other AllocNode;
	AllocNode _alloc;
	XorListNode<T>* _first;
	XorListNode<T>* _last;
	size_t _size;
#if DEBUG
    uint _version;
#endif
};

template <typename T, class Alloc>
class XorListIterator : std::iterator<std::bidirectional_iterator_tag, T> {
public:
    friend class XorList<T, Alloc>;

    XorListIterator<T, Alloc> &operator++();

    const XorListIterator<T, Alloc> operator++(int);
    XorListIterator<T, Alloc> &operator--();
    const XorListIterator<T, Alloc> operator--(int);
    T& operator*();
    T* operator->();

    bool operator==(const XorListIterator<T, Alloc>&) const;
    bool operator!=(const XorListIterator<T, Alloc>&) const;

private:
    XorList<T, Alloc>* _list;
    XorListNode<T>* _node;
    XorListNode<T>* _prev_node;
#if DEBUG
    bool is_valid() const;
    uint _version;
#endif
};

template <typename T>
XorListNode<T>* get_next(XorListNode<T>* first, XorListNode<T>* second);

template <typename T>
XorListNode<T>* get_prev(XorListNode<T>* first, XorListNode<T>* second);

//=======================================================================================
//=======================================================================================

using std::forward;

template<typename T, class Alloc>
XorList<T, Alloc>::XorList(const Alloc& alloc):
        _alloc(alloc),
        _first(nullptr), _last(nullptr),
        _size(0) {
#if DEBUG
    _version = 0;
#endif
}

template<typename T, class Alloc>
XorList<T, Alloc>::XorList(size_t count, const T& value,
                           const Alloc& alloc): XorList() {
    for (int i = 0; i < count; ++i) {
        this->push_back(value);
    }
}

template<typename T, class Alloc>
XorList<T, Alloc>::XorList(const XorList<T, Alloc>& other):XorList() {
    _alloc = other._alloc;
    auto other_ptr = const_cast<XorList<T, Alloc>*>(&other);
    for (auto it = other_ptr->begin(); it != other_ptr->end(); ++it) {
        push_back(*it);
    }
}

template<typename T, class Alloc>
XorList<T, Alloc>::XorList(XorList&& other) noexcept:
        _alloc(other._alloc),
        _first(other._first), _last(other._last),
        _size(other._size) {
#if DEBUG
    _version = 0;
#endif
    other._first = other._last = nullptr;
    other._size = 0;
}

template<typename T, class Alloc>
XorList<T, Alloc>::~XorList() {
    delete_nodes();
}

template <typename T, class Alloc>
void XorList<T, Alloc>::delete_nodes() {
    node* first = nullptr;
    node* second = _first;

    while (second != nullptr) {
        node* next_node = get_next(first, second);
        first = second;
        second = next_node;
        _alloc.destroy(first);
        _alloc.deallocate(first, 1);
    }
}


//----------------------------------------------------------------------

template <typename T, class Alloc>
XorList<T, Alloc>& XorList<T, Alloc>::operator=(const XorList<T, Alloc>& other) {
    delete_nodes();
    _first = _last = nullptr;
    _size = 0;
#if DEBUG
    _version++;
#endif
    auto other_ptr = const_cast<XorList<T, Alloc>*>(&other);
    for (auto it = other_ptr->begin(); it != other_ptr->end(); ++it) {
        push_back(*it);
    }
}

template <typename T, class Alloc>
XorList<T, Alloc>& XorList<T, Alloc>::operator=(XorList && other) noexcept {
    delete_nodes();
    _alloc = other._alloc;
    _first = other._first;
    _last = other._last;
    _size = other._size;
#if DEBUG
    _version++;
#endif

    other._size = 0;
    other._first = other._last = nullptr;
}

//----------------------------------------------------------------------

template <typename T, class Alloc>
typename XorList<T, Alloc>::iterator XorList<T, Alloc>::begin() {
    iterator iter;
    iter._list = this;
    iter._node = _first;
    iter._prev_node = nullptr;
#if DEBUG
    iter._version = _version;
#endif
    return iter;
}

template <typename T, class Alloc>
typename XorList<T, Alloc>::iterator XorList<T, Alloc>::end() {
    iterator iter;
    iter._list = this;
    iter._node = nullptr;
    iter._prev_node = _last;
#if DEBUG
    iter._version = _version;
#endif
    return iter;
}

//----------------------------------------------------------------------

template <typename T, class Alloc>
size_t XorList<T, Alloc>::size() const {
    return _size;
}

//-----------------------------------------------------------------------

template<typename T>
XorListNode<T>* get_next(XorListNode<T> *first, XorListNode<T> *second) {
    return xor_ptr(first, second->ptr);
}

template <typename T>
XorListNode<T>* get_prev(XorListNode<T> *first, XorListNode<T> *second) {
    return xor_ptr(first->ptr, second);
}

//---------------------------------------------------------------------------

template <typename T, class Alloc>
void XorList<T, Alloc>::insert_node_before(XorList<T, Alloc>::node* node_for_insert,
                                           iterator& iter) {
    node_for_insert->ptr = xor_ptr(iter._prev_node, iter._node);

    if (iter._node != nullptr) {
        iter._node->ptr = xor_ptr(iter._node->ptr, node_for_insert, iter._prev_node);
    }
    else {
        iter._list->_last = node_for_insert;
    }

    if (iter._prev_node != nullptr) {
        iter._prev_node->ptr = xor_ptr(iter._prev_node->ptr, node_for_insert, iter._node);
    }
    else {
        iter._list->_first = node_for_insert;
    }
    iter._prev_node = node_for_insert;

    _size++;
#if DEBUG
    _version++;
    iter._version++;
#endif
}

//-----------------------------------------------------------------------------

template<typename T, class Alloc>
template <typename U>
typename XorList<T, Alloc>::iterator XorList<T, Alloc>::insert_before
        (XorList<T, Alloc>::iterator iter, U&& value) {
#ifdef DEBUG
    if (iter._version != this->_version)
        throw YException("XorList: Iterator is invalid because the list has been changed");
    if (iter._list != this)
        throw YException("XorList: trying to use iterator from other list");
#endif

    node* new_node = _alloc.allocate(1);
    _alloc.construct(new_node, std::forward<U>(value));
    insert_node_before(new_node, iter);
    return iter;
}

template<typename T, class Alloc>
template <typename U>
typename XorList<T, Alloc>::iterator XorList<T, Alloc>::insert_after
        (XorList<T, Alloc>::iterator iter, U&& value) {
#ifdef DEBUG
    if (iter == end())
        throw YException("XorList: trying to insert after end iterator");
#endif

    ++iter;
    iter = insert_before(iter, forward<U>(value));
    --iter;
    --iter;
    return iter;
}

template<typename T, class Alloc>
template <typename U>
void XorList<T, Alloc>::push_back(U&& value) {
    auto it = end();
    insert_before(it, forward<U>(value));
}

template<typename T, class Alloc>
template <typename U>
void XorList<T, Alloc>::push_front(U&& value) {
    insert_before(begin(), forward<U>(value));
}

//---------------------------------------------------------------------------------

template<typename T, class Alloc>
void XorList<T, Alloc>::erase(XorList<T, Alloc>::iterator iter) {
#ifdef DEBUG
    if (iter._node == nullptr)
        throw YException("XorList: trying to erase element after last");
    if (iter._version != this->_version)
        throw YException("XorList: Iterator is invalid because the list has been changed");
    if (iter._list != this)
        throw YException("XorList: trying to use iterator from other list");
#endif

    node* next_node = get_next(iter._prev_node, iter._node);

    if (next_node != nullptr) {
        next_node->ptr = xor_ptr(next_node->ptr, iter._node, iter._prev_node);
    }
    else {
        _last = iter._prev_node;
    }

    if (iter._prev_node != nullptr) {
        iter._prev_node->ptr = xor_ptr(iter._prev_node->ptr, iter._node, next_node);
    }
    else {
        _first = next_node;
    }

    _alloc.destroy(iter._node);
    _alloc.deallocate(iter._node, 1);

    --_size;
#if DEBUG
    ++_version;
#endif
}

template<typename T, class Alloc>
void XorList<T, Alloc>::pop_back() {
    auto it = end();
    --it;
    erase(it);
}

template<typename T, class Alloc>
void XorList<T, Alloc>::pop_front() {
    erase(begin());
}

//---------------------------------------------------------------------------------

template<typename T, class Alloc>
T& XorList<T, Alloc>::back() {
    if (_size == 0)
        throw YException("XorList: trying to get elements from empty list");

    return _last->value;
}

template<typename T, class Alloc>
T& XorList<T, Alloc>::front() {
    if (_size == 0)
        throw YException("XorList: trying to get elements from empty list");

    return _first->value;
}

//**********************************************************************************

template <typename T, class Alloc>
XorListIterator<T, Alloc>& XorListIterator<T, Alloc>::operator++() {
#if DEBUG
    if (!is_valid())
        throw YException("XorList iterator: Iterator is invalid because the list has been changed");
#endif

    auto next_node = get_next(_prev_node, _node);
    _prev_node = _node;
    _node = next_node;
    return *this;
}

template <typename T, class Alloc>
XorListIterator<T, Alloc>& XorListIterator<T, Alloc>::operator--() {
#if DEBUG
    if (!is_valid())
        throw YException("XorList iterator: Iterator is invalid because the list has been changed");
#endif

    auto very_prev_node = get_prev(_prev_node, _node);
    _node = _prev_node;
    _prev_node = very_prev_node;
    return *this;
}

template <typename T, class Alloc>
const XorListIterator<T, Alloc> XorListIterator<T, Alloc>::operator++(int) {
    auto result = *this;
    operator++();
    return result;
}

template <typename T, class Alloc>
const XorListIterator<T, Alloc> XorListIterator<T, Alloc>::operator--(int) {
    auto result = *this;
    operator--();
    return result;
}

//----------------------------------------------------------------------------------

template <typename T, class Alloc>
bool XorListIterator<T, Alloc>::operator==
        (const XorListIterator<T, Alloc> &other) const {

    return _list == other._list and _node == other._node;
}

template <typename T, class Alloc>
bool XorListIterator<T, Alloc>::operator!=(const XorListIterator<T, Alloc> &other) const {
    return not (*this == other);
}

//-----------------------------------------------------------------------------

template <typename T, class Alloc>
T& XorListIterator<T, Alloc>::operator*() {
    return _node->value;
}

template <typename T, class Alloc>
T* XorListIterator<T, Alloc>::operator->() {
    return &(_node->value);
}

#if DEBUG
template <typename T, class Alloc>
bool XorListIterator<T, Alloc>::is_valid() const {
    return _version == _list->_version;
}
#endif