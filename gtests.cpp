#include <gtest/gtest.h>
#include <vector>
#include "allocator.h"
#include "checker.h"
#include "list.h"
#include "test.h"

using std::vector;

TEST(allocator, alloc) {
    StackAllocator<int> alloc;
    int* x = alloc.allocate(3);
    int* y = alloc.allocate(4);
    x[2] = 4;
    x[1] = (int)1e9;
    x[0] = (int)1e9 + 1;
    y[0] = 0;
    EXPECT_EQ(y[0], 0);
    EXPECT_EQ(x[1], (int)1e9);
}

TEST(allocator, construct) {
    StackAllocator<std::vector<int> > alloc;
    std::vector<int>* v = alloc.allocate(1);
    alloc.construct(v, 3, 3);
    EXPECT_EQ((*v)[2], 3);
    alloc.destroy(v);
}

TEST(allocator, destruct) {
    Checker::events.clear();
    StackAllocator<Checker> alloc;

    Checker* c = alloc.allocate(1);
    alloc.construct(c);
    alloc.destroy(c);

    EXPECT_EQ(Checker::events[0], CONSTRUCT_DEFAULT);
    EXPECT_EQ(Checker::events[1], DESTRUCT);
}

TEST(allocator, big_page) {
    StackAllocator<int> alloc;

    int* x = alloc.allocate(4097);
    int* y = alloc.allocate(3);
    x[1024] = 0;
    x[4096] = 0;
    y[0] = y[1] = y[2] = (int)1e9;
    EXPECT_EQ(0, x[1024]);
    EXPECT_EQ(0, x[4096]);
}

TEST(allocator, complex_construct) {
    StackAllocator<Checker> alloc;
    Checker::events.clear();

    Checker c;
    Checker* ptr = alloc.allocate(1);
    alloc.construct(ptr, c);

    std::vector<CheckerEvent> answer = {CONSTRUCT_DEFAULT, CONSTRUCT_COPY};
    EXPECT_EQ(answer, Checker::events);
}

TEST(allocator, rebind) {
    StackAllocator<int> alloc;
    StackAllocator<int>::rebind<double>::other alloc2;

    int* x = alloc.allocate(3);
    double* y = alloc2.allocate(1);
    *y = 0.0;
    x[2] = (int)2e9;
    EXPECT_EQ(*y, 0.0);
}

//-----------------------------------------------------------------------------

namespace list_test {

    XorList<int> gen_list(size_t size) {
        XorList<int> result;
        for (int i = 0; i < size; ++i) {
            result.push_back(i);
        }
        return result;
    }

    template <typename T>
    void check_list_element(XorList<T>& list, int pos, T value) {
        auto it = list.begin();
        for (int i = 0; i < pos; ++i) {
            ++it;
        }
        EXPECT_EQ(*it, value);
    }

}

TEST(list, push) {
    XorList<int> list;
    list.push_back(4);
    list.push_front(1);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.back(), 4);
    EXPECT_EQ(list.front(), 1);
}

TEST(list, pop) {
    XorList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);

    list.pop_back();
    EXPECT_EQ(list.back(), 3);
    EXPECT_EQ(list.front(), 1);

    list.pop_front();
    EXPECT_EQ(list.back(), 3);
    EXPECT_EQ(list.front(), 2);
}

TEST(list, construct_copy) {
    XorList<int> l1(1, 1);
    XorList<int> l2(l1);
    l1.pop_front();

    EXPECT_EQ(l2.size(), 1);
    EXPECT_EQ(l2.back(), 1);
}

TEST(list, construct_fill) {
    XorList<int> list(5, 4);
    EXPECT_EQ(list.size(), 5);
    for (auto it = list.begin(); it != list.end(); ++it) {
        EXPECT_EQ(*it, 4);
    }
}

TEST(list, stack_alloc) {
    XorList<int, StackAllocator<int> > list(3, 1);
    list.push_back(4);
    list.pop_front();

    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 4);
}

TEST(list, push_copy) {
    Checker::events.clear();
    XorList<Checker> list;

    Checker c;
    list.push_back(c);
    list.push_front(c);

    vector<CheckerEvent> answer = {CONSTRUCT_DEFAULT, CONSTRUCT_COPY, CONSTRUCT_COPY};
    EXPECT_EQ(Checker::events, answer);
}

TEST(list, construct_move) {
    Checker::events.clear();

    XorList<Checker>* ptr = new XorList<Checker>(2);
    XorList<Checker> l(std::move(*ptr));
    delete ptr;

    vector<CheckerEvent> answer = {CONSTRUCT_DEFAULT,
                                   CONSTRUCT_COPY,
                                   CONSTRUCT_COPY,
                                   DESTRUCT};
    EXPECT_EQ(Checker::events, answer);
}

TEST(list, push_move) {
    Checker::events.clear();
    XorList<Checker> list;

    Checker c;
    Checker ch;
    list.push_back(std::move(c));
    list.push_front(std::move(ch));

    vector<CheckerEvent> answer = {CONSTRUCT_DEFAULT,
                                   CONSTRUCT_DEFAULT,
                                   CONSTRUCT_MOVE,
                                   CONSTRUCT_MOVE};
    EXPECT_EQ(Checker::events, answer);
}

TEST(list, destroy) {
    Checker::events.clear();
    XorList<Checker>* lptr = new XorList<Checker>(3);
    delete lptr;

    vector<CheckerEvent> answer = {CONSTRUCT_DEFAULT,
                                   CONSTRUCT_COPY,
                                   CONSTRUCT_COPY,
                                   CONSTRUCT_COPY,
                                   DESTRUCT,
                                   DESTRUCT,
                                   DESTRUCT,
                                   DESTRUCT};
    EXPECT_EQ(Checker::events, answer);
}

TEST(list, insert_before) {
    XorList<int> list(5, 4);
    auto it = list.begin();

    ++it;
    it = list.insert_before(it, 3);
    list.insert_before(it, 10);

    list_test::check_list_element(list, 0, 4);
    list_test::check_list_element(list, 1, 3);
    list_test::check_list_element(list, 2, 10);
    list_test::check_list_element(list, 3, 4);
}

TEST(list, insert_after) {
    XorList<int> list(6, 4);
    auto it = list.begin();
    it = list.insert_after(it, 10);
    ++it;
    list.insert_after(it, 100);

    list_test::check_list_element(list, 0, 4);
    list_test::check_list_element(list, 1, 10);
    list_test::check_list_element(list, 2, 100);
    list_test::check_list_element(list, 3, 4);
}

//------------------------------------------------------------------------

TEST(iterator, begin) {
    XorList<int> list = list_test::gen_list(4);

    EXPECT_EQ(*list.begin(), 0);
}

TEST(iterator, end) {
    XorList<int> list = list_test::gen_list(5);
    auto it = list.end();
    --it;
    EXPECT_EQ(*it, 4);
}

TEST(iterator, next) {
    XorList<int> list = list_test::gen_list(3);
    int i = 0;
    for (int &it : list) {
        EXPECT_EQ(it, i);
        ++i;
    }
    EXPECT_EQ(i, 3);
}

TEST(iterator, prev) {
    XorList<int> list = list_test::gen_list(5);
    auto it = list.end();
    int i = 4;
    for (; i >= 0; --i) {
        --it;
        EXPECT_EQ(*it, i);
    }
}

TEST(iterator, equial) {
    XorList<int> list(4);
    auto it1 = list.begin();
    auto it2 = list.begin();
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);

    it1++;
    EXPECT_TRUE(it1 != it2);
    EXPECT_FALSE(it1 == it2);

    it2++;
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);

    auto it3 = list.end();
    for (int i = 0; i < 3; ++i) --it3;
    EXPECT_TRUE(it1 == it3);
    EXPECT_FALSE(it1 != it3);
}

TEST(auto_tests, check_is_equial) {
    int size = 20;
    int count = 10000;
    for (int i = 0; i < count; ++i) {
        bool ok = check_is_equial<int,
                std::list<int>,
                XorList<int, StackAllocator<int> > >(size);
        EXPECT_TRUE(ok);
    }
}
