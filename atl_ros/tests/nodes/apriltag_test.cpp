#include <gtest/gtest.h>

#include "atl/ros/nodes/apriltag_node.hpp"
#include "atl/ros/utils/node.hpp"

namespace atl {

// NODE SETTNGS
#define TEST_NODE_NAME "apriltag_test_node"

// PUBLISH TOPICS
#define SHUTDOWN_TOPIC "/atl/apriltag/shutdown"
#define IMAGE_TOPIC "/atl/camera/image_pose_stamped"

// SUBSCRIBE TOPICS
#define TARGET_POSE_TOPIC "/atl/apriltag/target"
#define TARGET_W_TOPIC "/atl/apriltag/target/inertial"
#define TARGET_P_TOPIC "/atl/apriltag/target/body"

// TEST DATA
#define TEST_IMAGE "test_data/image.jpg"

class NodeTest : public ::testing::Test {
protected:
  ros::NodeHandle ros_nh;

  ros::Publisher shutdown_pub;
  image_transport::Publisher image_pub;

  ros::Subscriber pose_sub;
  ros::Subscriber if_sub;
  ros::Subscriber bf_sub;

  atl_msgs::AprilTagPose pose_msg;
  geometry_msgs::Vector3 inertial_msg;
  geometry_msgs::Vector3 body_msg;

  NodeTest() {
    image_transport::ImageTransport it(this->ros_nh);

    // clang-format off
    this->image_pub = it.advertise(IMAGE_TOPIC, 1);
    this->pose_sub = this->ros_nh.subscribe(TARGET_POSE_TOPIC, 1, &NodeTest::poseCallback, this);
    this->if_sub = this->ros_nh.subscribe(TARGET_W_TOPIC, 1, &NodeTest::inertialCallback, this);
    this->bf_sub = this->ros_nh.subscribe(TARGET_P_TOPIC, 1, &NodeTest::bodyPlanarCallback, this);
    // clang-format on

    ros::spinOnce();
    ros::Duration(1.0).sleep();
  }

  virtual void SetUp() {
    sensor_msgs::ImageConstPtr msg;
    cv::Mat image;

    // setup image
    image = cv::imread(TEST_IMAGE);
    image.at<double>(0, 0) = 0.0; // position x
    image.at<double>(0, 1) = 0.0; // position y
    image.at<double>(0, 2) = 3.0; // position z

    image.at<double>(0, 3) = 1.0; // quaternion w
    image.at<double>(0, 4) = 0.0; // quaternion x
    image.at<double>(0, 5) = 0.0; // quaternion y
    image.at<double>(0, 6) = 0.0; // quaternion z

    // publish image
    msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", image).toImageMsg();
    this->image_pub.publish(msg);

    // spin and sleep
    ros::spinOnce();
    ros::Duration(1.0).sleep();
    ros::spinOnce();
  }

  void poseCallback(const atl_msgs::AprilTagPose &msg) { this->pose_msg = msg; }

  void inertialCallback(const geometry_msgs::Vector3 &msg) {
    this->inertial_msg = msg;
  }

  void bodyPlanarCallback(const geometry_msgs::Vector3 &msg) {
    this->body_msg = msg;
  }
};

TEST_F(NodeTest, poseMsg) {
  EXPECT_EQ(1, this->pose_sub.getNumPublishers());

  EXPECT_EQ(0, this->pose_msg.id);
  EXPECT_TRUE(this->pose_msg.detected);
  ASSERT_NEAR(0.0, this->pose_msg.position.x, 0.2);
  ASSERT_NEAR(0.0, this->pose_msg.position.y, 0.2);
  ASSERT_NEAR(3.0, this->pose_msg.position.z, 0.2);
}

TEST_F(NodeTest, inertialMsg) {
  EXPECT_EQ(1, this->if_sub.getNumPublishers());

  ASSERT_NEAR(0.0, this->inertial_msg.x, 0.2);
  ASSERT_NEAR(0.0, this->inertial_msg.y, 0.2);
  ASSERT_NEAR(0.0, this->inertial_msg.z, 0.2);
}

TEST_F(NodeTest, bodyMsg) {
  EXPECT_EQ(1, this->bf_sub.getNumPublishers());

  ASSERT_NEAR(0.0, this->body_msg.x, 0.2);
  ASSERT_NEAR(0.0, this->body_msg.y, 0.2);
  ASSERT_NEAR(-3.0, this->body_msg.z, 0.2);
}

} // namespace atl

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  ros::init(argc, argv, TEST_NODE_NAME);
  return RUN_ALL_TESTS();
}
