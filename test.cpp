#include "test.h"


template <>
int random_value<int> () {
    return rand() % 100;
}

void print(QueryType q) {
    switch (q) {
        case PUSH_BACK:
            cout << "push back ";
            break;
        case PUSH_FRONT:
            cout << "push front ";
            break;
        case POP_FRONT:
            cout << "pop front ";
            break;
        case POP_BACK:
            cout << "pop back ";
            break;
        case FRONT:
            cout << "front ";
            break;
        case BACK:
            cout << "back ";
            break;
        default:
            cout << "We forgot some types";
    }
}