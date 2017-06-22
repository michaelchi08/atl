#include <gtest/gtest.h>

#include "test_settings.hpp"
#include "wavesim_gazebo/gazebo_test.hpp"
#include "wavesim_gazebo/clients/world_gclient.hpp"

#define TEST_WORLD "tests/worlds/world_test.world"


namespace wavesim {
namespace gaz {

class WorldTest : public ::testing::Test {
protected:
  WorldGClient client;
  std::string test_world;

  WorldTest(void) {
    // setup gazebo world
    this->test_world = "wavesim_gazebo/" + std::string(TEST_WORLD);

    // setup world client
    if (this->client.configure() == -1) {
      std::cout << "Failed to connect to Gazebo Server!" << std::endl;
      exit(-1);
    }
  }

  virtual ~WorldTest(void) {
    this->client.clearWorld();
  }

  virtual void SetUp(void) {
    this->client.clearWorld();
    this->client.resetWorld();
    this->client.loadWorld(this->test_world);
  }

  virtual void TearDown(void) {
    this->client.clearWorld();
    this->client.resetWorld();
  }
};

TEST_F(WorldTest, PauseAndUnPauseWorld) {
  // test pause
  this->client.pauseWorld();
  sleep(1);
  ASSERT_TRUE(this->client.time.paused);

  // test unpause
  this->client.unPauseWorld();
  sleep(1);
  ASSERT_FALSE(this->client.time.paused);
}

TEST_F(WorldTest, Reset) {
  // test reset
  sleep(3);
  this->client.resetWorld();
  sleep(1);
  ASSERT_TRUE(this->client.time.sim_time.sec() < 2);
}

TEST_F(WorldTest, LoadAndRemoveModel) {
  Eigen::Vector3d pos;
  Eigen::Quaterniond quat;

  pos << 0.0, 0.0, 2.0;
  quat.w() = 1.0;
  quat.x() = 0.0;
  quat.y() = 0.0;
  quat.z() = 0.0;

  // test load model
  this->client.loadModel("table", pos, quat);
}

TEST_F(WorldTest, ClearAndLoadWorld) {
  this->client.clearWorld();
  this->client.loadWorld("worlds/quadrotor_test.world");
}

}  // end of gaz namespace
}  // end of wavesim namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
