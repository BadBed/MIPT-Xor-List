#include <iostream>
#include <list>
#include <gtest/gtest.h>
#include "test.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
int main() {
    cout << time_test<int, XorList<int> >(1000000) << "\n";

    cout << time_test<int, XorList<int, StackAllocator<int> > >(1000000) << "\n";
}
*/
