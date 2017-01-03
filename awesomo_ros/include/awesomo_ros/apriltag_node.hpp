#ifndef __AWESOMO_ROS_EXAMPLE_NODE_HPP__
#define __AWESOMO_ROS_EXAMPLE_NODE_HPP__

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/CameraInfo.h>

#include <awesomo_core/awesomo_core.hpp>
#include <awesomo_msgs/AprilTagPose.h>

#include "awesomo_ros/ros_node.hpp"
#include "awesomo_ros/ros_msgs.hpp"


namespace awesomo {

#define APRILTAG_NODE_NAME "awesomo_apriltag"
#define APRILTAG_NODE_RATE 100

class AprilTagNode : public ROSNode {
public:
  SwathmoreDetector detector;

  int configure(const std::string &node_name, int hz);
  void imageCallback(const sensor_msgs::ImageConstPtr &msg);
};

}  // end of awesomo namespace
#endif
