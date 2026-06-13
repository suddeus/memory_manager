#include <memory_manager.h>

#include <gtest/gtest.h>

TEST(memory_manager_tests, simple_tests) {
    ASSERT_EQ(0, 0);
    ASSERT_EQ(3, 3);
    ASSERT_EQ(4+4, 8);
}

TEST(memory_manager_tests, sum_tests) {
    ASSERT_EQ(sum(1, 2), 3);
    ASSERT_EQ(sum(3, -3), 0);
    ASSERT_EQ(sum(1, 0), 1);

    ASSERT_NE(sum(1, 2), 0);
}