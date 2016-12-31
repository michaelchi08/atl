#ifndef __AWESOMO_ROS_EXAMPLE_NODE_HPP__
#define __AWESOMO_ROS_EXAMPLE_NODE_HPP__

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/CameraInfo.h>

#include <awesomo_core/awesomo_core.hpp>
#include <awesomo_msgs/AprilTagPose.h>

#include "awesomo_ros/ros_node.hpp"
#include "awesomo_ros/ros_msgs.hpp"

#include "awesomo_ros/camera_node.hpp"


namespace awesomo {

#define APRILTAG_NODE_NAME "apriltag_node"
#define APRILTAG_NODE_RATE 100

#define APRILTAG_POSE_TOPIC "awesomo/apriltag/pose"

class AprilTagNode : public ROSNode {
public:
  SwathmoreDetector detector;

  int configure(const std::string &node_name, int hz);
  void imageCallback(const sensor_msgs::ImageConstPtr &msg);
};

}  // end of awesomo namespace
#endif
