#include <gtest/gtest.h>

#include "awesomo_core/quadrotor/hover_mode.hpp"

#define TEST_CONFIG "tests/configs/quadrotor/hover_mode.yaml"


TEST(HoverMode, constructor) {
  awesomo::HoverMode mode;

  ASSERT_FALSE(mode.configured);
  ASSERT_FLOAT_EQ(0.0, mode.hover_height);
  ASSERT_FLOAT_EQ(0.0, mode.hover_position[0]);
  ASSERT_FLOAT_EQ(0.0, mode.hover_position[1]);
  ASSERT_FLOAT_EQ(0.0, mode.hover_position[2]);
}

TEST(HoverMode, configure) {
  awesomo::HoverMode mode;

  mode.configure(TEST_CONFIG);
  ASSERT_TRUE(mode.configured);
  ASSERT_FLOAT_EQ(3.0, mode.hover_height);
  ASSERT_FLOAT_EQ(1.0, mode.hover_position[0]);
  ASSERT_FLOAT_EQ(2.0, mode.hover_position[1]);
  ASSERT_FLOAT_EQ(3.0, mode.hover_position[2]);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}