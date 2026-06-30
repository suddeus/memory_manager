#include <memory_manager.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <limits>

namespace {

constexpr std::size_t kMmapThreshold = 100 * 1024;

}

TEST(memory_manager_tests, malloc_returns_writable_memory) {
    auto* ptr = static_cast<int*>(s_malloc(sizeof(int)));

    ASSERT_NE(ptr, nullptr);

    *ptr = 5;
    EXPECT_EQ(*ptr, 5);

    *ptr += 1;
    EXPECT_EQ(*ptr, 6);

    s_free(ptr);
}

TEST(memory_manager_tests, independent_allocations_preserve_their_contents) {
    constexpr std::size_t kAllocationCount = 8;
    std::array<int*, kAllocationCount> ptrs{};

    for (std::size_t i = 0; i < ptrs.size(); ++i) {
        ptrs[i] = static_cast<int*>(s_malloc(sizeof(int)));
        ASSERT_NE(ptrs[i], nullptr);
        *ptrs[i] = static_cast<int>(i * 10);
    }

    for (std::size_t i = 0; i < ptrs.size(); ++i) {
        EXPECT_EQ(*ptrs[i], static_cast<int>(i * 10));
    }

    for (auto* ptr : ptrs) {
        s_free(ptr);
    }
}

TEST(memory_manager_tests, freed_block_can_be_reused) {
    auto* first = static_cast<unsigned char*>(s_malloc(128));
    ASSERT_NE(first, nullptr);

    std::memset(first, 0xAB, 128);
    s_free(first);

    auto* second = static_cast<unsigned char*>(s_malloc(128));
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second, first);

    second[0] = 0x12;
    second[127] = 0x34;
    EXPECT_EQ(second[0], 0x12);
    EXPECT_EQ(second[127], 0x34);

    s_free(second);
}

TEST(memory_manager_tests, adjacent_freed_blocks_are_merged) {
    auto* first = static_cast<unsigned char*>(s_malloc(128));
    auto* second = static_cast<unsigned char*>(s_malloc(256));
    auto* guard = static_cast<unsigned char*>(s_malloc(64));
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(guard, nullptr);

    s_free(first);
    s_free(second);

    auto* merged = static_cast<unsigned char*>(s_malloc(320));
    ASSERT_NE(merged, nullptr);
    EXPECT_EQ(merged, first);

    merged[0] = 0xAA;
    merged[319] = 0x55;
    EXPECT_EQ(merged[0], 0xAA);
    EXPECT_EQ(merged[319], 0x55);

    s_free(merged);
    s_free(guard);
}

TEST(memory_manager_tests, free_ignores_null_and_unknown_pointers) {
    int stack_value = 42;

    EXPECT_NO_FATAL_FAILURE(s_free(nullptr));
    EXPECT_NO_FATAL_FAILURE(s_free(&stack_value));

    auto* ptr = static_cast<int*>(s_malloc(sizeof(int)));
    ASSERT_NE(ptr, nullptr);
    *ptr = stack_value;
    EXPECT_EQ(*ptr, stack_value);

    s_free(ptr);
}

TEST(memory_manager_tests, large_malloc_at_threshold_returns_writable_memory) {
    auto* ptr = static_cast<unsigned char*>(s_malloc(kMmapThreshold));
    ASSERT_NE(ptr, nullptr);

    ptr[0] = 0x12;
    ptr[kMmapThreshold / 2] = 0x34;
    ptr[kMmapThreshold - 1] = 0x56;

    EXPECT_EQ(ptr[0], 0x12);
    EXPECT_EQ(ptr[kMmapThreshold / 2], 0x34);
    EXPECT_EQ(ptr[kMmapThreshold - 1], 0x56);
}

TEST(memory_manager_tests, large_malloc_above_threshold_returns_writable_memory) {
    constexpr std::size_t kLargeSize = kMmapThreshold + 4096;

    auto* ptr = static_cast<unsigned char*>(s_malloc(kLargeSize));
    ASSERT_NE(ptr, nullptr);

    ptr[0] = 0xAB;
    ptr[kMmapThreshold] = 0xCD;
    ptr[kLargeSize - 1] = 0xEF;

    EXPECT_EQ(ptr[0], 0xAB);
    EXPECT_EQ(ptr[kMmapThreshold], 0xCD);
    EXPECT_EQ(ptr[kLargeSize - 1], 0xEF);
}

TEST(memory_manager_tests, multiple_large_mallocs_return_independent_writable_memory) {
    constexpr std::array<std::size_t, 3> kSizes{
        kMmapThreshold,
        kMmapThreshold + 1,
        kMmapThreshold + 8192,
    };

    std::array<unsigned char*, kSizes.size()> ptrs{};

    for (std::size_t i = 0; i < ptrs.size(); ++i) {
        ptrs[i] = static_cast<unsigned char*>(s_malloc(kSizes[i]));
        ASSERT_NE(ptrs[i], nullptr);

        ptrs[i][0] = static_cast<unsigned char>(0x10 + i);
        ptrs[i][kSizes[i] / 2] = static_cast<unsigned char>(0x20 + i);
        ptrs[i][kSizes[i] - 1] = static_cast<unsigned char>(0x30 + i);
    }

    for (std::size_t i = 0; i < ptrs.size(); ++i) {
        EXPECT_EQ(ptrs[i][0], static_cast<unsigned char>(0x10 + i));
        EXPECT_EQ(ptrs[i][kSizes[i] / 2], static_cast<unsigned char>(0x20 + i));
        EXPECT_EQ(ptrs[i][kSizes[i] - 1], static_cast<unsigned char>(0x30 + i));
    }
}

TEST(memory_manager_tests, free_accepts_large_allocated_pointer) {
    auto* ptr = static_cast<unsigned char*>(s_malloc(kMmapThreshold));
    ASSERT_NE(ptr, nullptr);

    ptr[0] = 0xA5;
    ptr[kMmapThreshold - 1] = 0x5A;

    EXPECT_NO_FATAL_FAILURE(s_free(ptr));
}

TEST(memory_manager_tests, calloc_returns_zero_initialized_writable_memory) {
    constexpr std::size_t kElementCount = 16;

    auto* ptr = static_cast<int*>(s_calloc(kElementCount, sizeof(int)));
    ASSERT_NE(ptr, nullptr);

    for (std::size_t i = 0; i < kElementCount; ++i) {
        EXPECT_EQ(ptr[i], 0);
    }

    ptr[0] = 17;
    ptr[kElementCount - 1] = 29;
    EXPECT_EQ(ptr[0], 17);
    EXPECT_EQ(ptr[kElementCount - 1], 29);

    s_free(ptr);
}

TEST(memory_manager_tests, calloc_reused_memory_is_cleared_before_returning) {
    constexpr std::size_t kSize = 64;

    auto* original = static_cast<unsigned char*>(s_malloc(kSize));
    ASSERT_NE(original, nullptr);
    std::memset(original, 0xFF, kSize);
    s_free(original);

    auto* cleared = static_cast<unsigned char*>(s_calloc(kSize, sizeof(unsigned char)));
    ASSERT_NE(cleared, nullptr);

    for (std::size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(cleared[i], 0);
    }

    s_free(cleared);
}

TEST(memory_manager_tests, calloc_returns_null_when_total_size_overflows) {
    constexpr std::size_t kOverflowCount = std::numeric_limits<std::size_t>::max();

    EXPECT_EQ(s_calloc(kOverflowCount, 2), nullptr);
}

TEST(memory_manager_tests, large_calloc_returns_zero_initialized_writable_memory) {
    constexpr std::size_t kElementCount = kMmapThreshold / sizeof(int);

    auto* ptr = static_cast<int*>(s_calloc(kElementCount, sizeof(int)));
    ASSERT_NE(ptr, nullptr);

    EXPECT_EQ(ptr[0], 0);
    EXPECT_EQ(ptr[kElementCount / 2], 0);
    EXPECT_EQ(ptr[kElementCount - 1], 0);

    ptr[0] = 11;
    ptr[kElementCount / 2] = 22;
    ptr[kElementCount - 1] = 33;

    EXPECT_EQ(ptr[0], 11);
    EXPECT_EQ(ptr[kElementCount / 2], 22);
    EXPECT_EQ(ptr[kElementCount - 1], 33);
}

TEST(memory_manager_tests, large_calloc_above_threshold_returns_zero_initialized_memory) {
    constexpr std::size_t kElementCount = (kMmapThreshold + 4096) / sizeof(int);

    auto* ptr = static_cast<int*>(s_calloc(kElementCount, sizeof(int)));
    ASSERT_NE(ptr, nullptr);

    EXPECT_EQ(ptr[0], 0);
    EXPECT_EQ(ptr[kElementCount / 2], 0);
    EXPECT_EQ(ptr[kElementCount - 1], 0);

    ptr[0] = 101;
    ptr[kElementCount - 1] = 202;

    EXPECT_EQ(ptr[0], 101);
    EXPECT_EQ(ptr[kElementCount - 1], 202);
}

TEST(memory_manager_tests, realloc_null_pointer_behaves_like_malloc) {
    auto* ptr = static_cast<int*>(s_realloc(nullptr, sizeof(int)));
    ASSERT_NE(ptr, nullptr);

    *ptr = 42;
    EXPECT_EQ(*ptr, 42);

    s_free(ptr);
}

TEST(memory_manager_tests, realloc_null_pointer_with_large_size_behaves_like_malloc) {
    auto* ptr = static_cast<unsigned char*>(s_realloc(nullptr, kMmapThreshold));
    ASSERT_NE(ptr, nullptr);

    ptr[0] = 0x44;
    ptr[kMmapThreshold - 1] = 0x88;

    EXPECT_EQ(ptr[0], 0x44);
    EXPECT_EQ(ptr[kMmapThreshold - 1], 0x88);
}

TEST(memory_manager_tests, realloc_zero_size_frees_pointer_and_returns_null) {
    auto* ptr = static_cast<int*>(s_malloc(sizeof(int)));
    ASSERT_NE(ptr, nullptr);
    *ptr = 42;

    EXPECT_EQ(s_realloc(ptr, 0), nullptr);

    auto* reused = static_cast<int*>(s_malloc(sizeof(int)));
    ASSERT_NE(reused, nullptr);
    EXPECT_EQ(reused, ptr);

    s_free(reused);
}

TEST(memory_manager_tests, realloc_same_size_returns_same_pointer_and_preserves_data) {
    constexpr std::size_t kSize = 32;

    auto* ptr = static_cast<unsigned char*>(s_malloc(kSize));
    ASSERT_NE(ptr, nullptr);

    for (std::size_t i = 0; i < kSize; ++i) {
        ptr[i] = static_cast<unsigned char>(i + 1);
    }

    auto* resized = static_cast<unsigned char*>(s_realloc(ptr, kSize));
    ASSERT_NE(resized, nullptr);
    EXPECT_EQ(resized, ptr);

    for (std::size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(resized[i], static_cast<unsigned char>(i + 1));
    }

    s_free(resized);
}

TEST(memory_manager_tests, realloc_larger_block_preserves_existing_contents) {
    constexpr std::size_t kOldSize = 32;
    constexpr std::size_t kNewSize = 96;

    auto* ptr = static_cast<unsigned char*>(s_malloc(kOldSize));
    ASSERT_NE(ptr, nullptr);

    for (std::size_t i = 0; i < kOldSize; ++i) {
        ptr[i] = static_cast<unsigned char>(0xA0 + i);
    }

    auto* resized = static_cast<unsigned char*>(s_realloc(ptr, kNewSize));
    ASSERT_NE(resized, nullptr);

    for (std::size_t i = 0; i < kOldSize; ++i) {
        EXPECT_EQ(resized[i], static_cast<unsigned char>(0xA0 + i));
    }

    resized[kNewSize - 1] = 0x5A;
    EXPECT_EQ(resized[kNewSize - 1], 0x5A);

    s_free(resized);
}

TEST(memory_manager_tests, realloc_smaller_block_preserves_truncated_contents) {
    constexpr std::size_t kOldSize = 96;
    constexpr std::size_t kNewSize = 32;

    auto* ptr = static_cast<unsigned char*>(s_malloc(kOldSize));
    ASSERT_NE(ptr, nullptr);

    for (std::size_t i = 0; i < kOldSize; ++i) {
        ptr[i] = static_cast<unsigned char>(0x10 + i);
    }

    auto* resized = static_cast<unsigned char*>(s_realloc(ptr, kNewSize));
    ASSERT_NE(resized, nullptr);

    for (std::size_t i = 0; i < kNewSize; ++i) {
        EXPECT_EQ(resized[i], static_cast<unsigned char>(0x10 + i));
    }

    s_free(resized);
}

TEST(memory_manager_tests, realloc_returns_null_for_unimplemented_mmap_sized_allocations) {
    auto* ptr = static_cast<int*>(s_malloc(sizeof(int)));
    ASSERT_NE(ptr, nullptr);

    *ptr = 123;
    EXPECT_EQ(s_realloc(ptr, kMmapThreshold), nullptr);
    EXPECT_EQ(*ptr, 123);

    s_free(ptr);
}
