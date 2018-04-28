#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include <cstdio>
#include <ctime>
#include "allocator.h"
#include "list.h"

using std::shared_ptr;
using std::vector;
using std::cout;

enum QueryType {PUSH_BACK = 0, POP_BACK, PUSH_FRONT, POP_FRONT, BACK,
    FRONT, COUNT_OF_QUERY_TYPES};

//-------------------------------------------------------------------

template <typename T>
struct QueryInputAdd {
    T value;
};

template <typename T>
struct QueryInputDelete {};

template <typename T>
struct QueryInputGet {};

template <typename T>
struct QueryInput {
    QueryType type;
    union {
        QueryInputAdd<T> add;
        QueryInputDelete<T> del;
        QueryInputGet<T> get;
    };
};

void print(QueryType);

template <typename T>
void print(QueryInput<T> query) {
    print(query.type);
    if (query.type == PUSH_BACK or query.type == PUSH_FRONT) {
        cout << query.add.value;
    }
    cout << "\n";
}

//--------------------------------------------------------------------

template <typename T>
struct QueryOutputAdd {};

template <typename T>
struct QueryOutputDelete {};

template <typename T>
struct QueryOutputGet {
    T result;
};

template <typename T>
struct QueryOutput {
    QueryType type;
    union {
        QueryOutputAdd<T> add;
        QueryOutputDelete<T> del;
        QueryOutputGet<T> get;
    };

    bool operator==(const QueryOutput<T>& other) const;
};

template <typename T>
void print(QueryOutput<T> query) {
    print(query.type);
    if (query.type == BACK or query.type == FRONT) {
        cout << query.get.result;
    }
    cout << "\n";
}

//--------------------------------------------------------

struct ModelList {
    size_t size;

    ModelList() : size(0) {}
};

//********************************************************************

template <typename T>
bool QueryOutput<T>::operator==(const QueryOutput<T> &other) const {
    if (type != other.type) {
        return false;
    }
    else if (type == BACK or type == FRONT) {
        return get.result == other.get.result;
    }
    else {
        return true;
    }
}

template <typename T>
bool model_query(ModelList& model, QueryInput<T> query) {
    if (query.type == PUSH_BACK or query.type == PUSH_FRONT) {
        model.size++;
    }
    else if (query.type == POP_BACK or query.type == POP_FRONT) {
        if (model.size == 0)
            return false;
        model.size--;
    }
    else if (query.type == BACK or query.type == FRONT) {
        if (model.size == 0)
            return false;
    }
    else {
        throw YException("model_query bug: unknown query types");
    }
    return true;
};

//------------------------------------------------------------------------

template <typename T>
T random_value();

template <typename T>
QueryInput<T> random_simple_query() {
    auto type = static_cast<QueryType >(rand() % COUNT_OF_QUERY_TYPES);
    if (type == POP_FRONT or type == POP_BACK or type == BACK or type == FRONT) {
        QueryInput<T> result;
        result.type = type;
        return result;
    }
    else if (type == PUSH_FRONT or type == PUSH_BACK){
        QueryInput<T> result;
        result.type = type;
        result.add.value = random_value<T>();
        return result;
    }
    else {
        throw YException("From random_simple_query: unknown type of query\n");
    }
}

template <typename T>
std::vector<QueryInput<T>> gen_simple_queries(size_t size) {
    vector<QueryInput<T>> result;
    ModelList mlist;

    while (result.size() < size) {
        auto query = random_simple_query<T>();
        if (model_query(mlist, query)) {
            result.push_back(query);
        }
    }
    return result;
}

//--------------------------------------------------------------------

template <typename T, class List>
QueryOutput<T> do_query(List& list, QueryInput<T> query) {
    QueryOutput<T> result;
    result.type = query.type;

    if (query.type == POP_FRONT) {
        list.pop_front();
    }
    else if (query.type == POP_BACK) {
        list.pop_back();
    }
    else if (query.type == PUSH_FRONT) {
        list.push_front(query.add.value);
    }
    else if (query.type == PUSH_BACK) {
        list.push_front(query.add.value);
    }
    else if (query.type == BACK) {
        result.get.result = list.back();
    }
    else if (query.type == FRONT) {
        result.get.result = list.front();
    }
    else {
        throw YException("From do_query: unknown type of query\n");
    }
    return result;
}

template <typename T, class List>
vector<QueryOutput<T> > get_answers (const vector<QueryInput<T> >& queries) {
    List list;
    vector<QueryOutput<T> > result;

    for (int i = 0; i < queries.size(); ++i) {
        try {
            result.push_back(do_query(list, queries[i]));
        }
        catch (...){
            for (int j = 0; j <= i; ++j)
                print(queries[j]);

            throw;
        }
    }

    return result;
};

//----------------------------------------------------------------------

template <typename T, class List>
double time_test (size_t count) {
    auto queries = gen_simple_queries<T>(count);

    clock_t start = clock();
    List list;
    for (int i = 0; i < count; ++i) {
        do_query(list, queries[i]);
    }
    clock_t finish = clock();

    return (double)(finish - start)/CLOCKS_PER_SEC;
};

template <typename T, class List1, class List2>
bool check_is_equial(size_t count) {
    auto queries = gen_simple_queries<T>(count);
    auto res1 = get_answers<T, List1>(queries);
    auto res2 = get_answers<T, List2>(queries);
    return res1 == res2;
}
