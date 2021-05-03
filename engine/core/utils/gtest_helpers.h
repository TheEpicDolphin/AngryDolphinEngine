#pragma once

#include <gtest/gtest.h>

#define ASSERT_CONTAINERS_EQ(CONTAINER_1, SIZE_1, CONTAINER_2, SIZE_2) \
		ASSERT_EQ(SIZE_1, SIZE_2) << "Vectors x and y are of unequal length";						\
		for (std::size_t i = 0; i < SIZE_1; ++i) {                                                  \
			EXPECT_EQ(CONTAINER_1[i], CONTAINER_2[i]) << "Vectors x and y differ at index " << i;   \
		}																							\