#include "awesomo_ros/nodes/apriltag_node.hpp"

namespace awesomo {

int AprilTagNode::configure(const std::string &node_name, int hz) {
  std::string apriltag_config;

  // ros node
  if (ROSNode::configure(node_name, hz) != 0) {
    return -1;
  }

  // detector
  this->ros_nh->getParam("/apriltag_config", apriltag_config);
  if (this->detector.configure(apriltag_config) != 0) {
    ROS_ERROR("Failed to configure AprilTag Detector!");
    return -2;
  };

  // subscribers and publishers
  // clang-format off
  this->registerPublisher<awesomo_msgs::AprilTagPose>(TARGET_POSE_TOPIC);
  this->registerPublisher<geometry_msgs::Vector3>(TARGET_IF_TOPIC);
  this->registerPublisher<geometry_msgs::Vector3>(TARGET_BPF_TOPIC);
  this->registerSubscriber(GIMBAL_FRAME_POSE_TOPIC, &AprilTagNode::gimbalFrameCallback, this);
  this->registerSubscriber(GIMBAL_JOINT_ORIENTATION_TOPIC, &AprilTagNode::gimbalJointCallback, this);
  this->registerImageSubscriber(CAMERA_IMAGE_TOPIC, &AprilTagNode::imageCallback, this);
  this->registerShutdown(SHUTDOWN);
  // clang-format on

  // downward facing camera (gimbal is NWU frame)
  // NWU frame: (x - forward, y - left, z - up)
  this->camera_offset = Pose(0.0, deg2rad(90.0), 0.0, 0.0, 0.0, 0.0);

  return 0;
}

void AprilTagNode::publishTagPoseMsg(TagPose tag) {
  awesomo_msgs::AprilTagPose msg;
  buildAprilTagPoseMsg(tag, msg);
  this->ros_pubs[TARGET_POSE_TOPIC].publish(msg);
}

void AprilTagNode::publishTargetBodyPositionMsg(void) {
  geometry_msgs::Vector3 msg;
  buildVector3Msg(this->target_bpf, msg);
  this->ros_pubs[TARGET_BPF_TOPIC].publish(msg);
}

void AprilTagNode::publishTargetInertialPositionMsg(Vec3 gimbal_position) {
  geometry_msgs::Vector3 msg;
  Vec3 target_enu, target_if;

  // convert target body planar frame from NWU to ENU
  nwu2enu(this->target_bpf, target_enu);

  // transform target from body to inertial frame
  target_if = gimbal_position + target_enu;

  // build and publish msg
  buildVector3Msg(target_if, msg);
  this->ros_pubs[TARGET_IF_TOPIC].publish(msg);
}

void AprilTagNode::publishGimbalSetpointAttitudeMsg(Vec3 setpoints) {
  geometry_msgs::Vector3 msg;
  buildVector3Msg(setpoints, msg);
  this->ros_pubs[GIMBAL_SETPOINT_ATTITUDE_TOPIC].publish(msg);
}

void AprilTagNode::gimbalFrameCallback(const geometry_msgs::Pose &msg) {
  this->gimbal_position(0) = msg.position.x;
  this->gimbal_position(1) = msg.position.y;
  this->gimbal_position(2) = msg.position.z;
}

void AprilTagNode::gimbalJointCallback(const geometry_msgs::Quaternion &msg) {
  this->gimbal_joint.w() = msg.w;
  this->gimbal_joint.x() = msg.x;
  this->gimbal_joint.y() = msg.y;
  this->gimbal_joint.z() = msg.z;
}

void AprilTagNode::imageCallback(const sensor_msgs::ImageConstPtr &msg) {
  Vec3 target_cf, gimbal_position;
  Quaternion joint_if;
  cv_bridge::CvImagePtr image_ptr;
  std::vector<TagPose> tags;

  // parse msg
  image_ptr = cv_bridge::toCvCopy(msg);

  // debug
  if (this->debug_mode) {
    cv::imshow("AprilTagNode Image", image_ptr->image);
    cv::waitKey(1);
  }

  // detect tags
  tags = this->detector.extractTags(image_ptr->image);
  if (tags.size() == 0) {
    return;
  }

  // transform tag in camera frame to body planar frame
  target_cf << tags[0].position(0), tags[0].position(1), tags[0].position(2);
  gimbal_position(0) = image_ptr->image.at<double>(0, 0);
  gimbal_position(1) = image_ptr->image.at<double>(0, 1);
  gimbal_position(2) = image_ptr->image.at<double>(0, 2);
  joint_if.w() = image_ptr->image.at<double>(0, 3);
  joint_if.x() = image_ptr->image.at<double>(0, 4);
  joint_if.y() = image_ptr->image.at<double>(0, 5);
  joint_if.z() = image_ptr->image.at<double>(0, 6);
  this->target_bpf =
    this->getTargetPositionInBodyPlanarFrame(target_cf, joint_if);

  // Vec3 euler_stamped, euler_actual;
  // quat2euler(joint_if, 321, euler_stamped);
  // quat2euler(this->gimbal_joint, 321, euler_actual);
  // std::cout << fabs(euler_stamped(0) - euler_actual(0)) << std::endl;
  // std::cout << fabs(euler_stamped(1) - euler_actual(1)) << std::endl;
  // std::cout << fabs(euler_stamped(2) - euler_actual(2)) << std::endl;
  // std::cout << std::endl;
  //
  // if (fabs(euler_stamped(0) - euler_actual(0)) > 0.02) {
  //   return;
  // } else if (fabs(euler_stamped(1) - euler_actual(1)) > 0.02) {
  //   return;
  // }

  // publish tag pose
  this->publishTagPoseMsg(tags[0]);
  this->publishTargetBodyPositionMsg();
  this->publishTargetInertialPositionMsg(gimbal_position);
  // this->trackTarget();
}

Vec3 AprilTagNode::getTargetPositionInBodyFrame(Vec3 target_cf) {
  Vec3 target_nwu;
  Mat3 R;
  Vec3 t;

  // transform camera frame to NWU frame
  // camera frame:  (z - forward, x - right, y - down)
  // NWU frame:  (x - forward, y - left, z - up)
  target_nwu(0) = target_cf(2);
  target_nwu(1) = -target_cf(0);
  target_nwu(2) = -target_cf(1);

  // camera mount offset
  R = this->camera_offset.rotationMatrix();
  t = this->camera_offset.position;

  // transform target from camera frame to body frame
  return (R * target_nwu + t);
}

Vec3 AprilTagNode::getTargetPositionInBodyPlanarFrame(Vec3 target_cf,
                                                      Quaternion joint_if) {
  Vec3 p;
  Mat3 R;
  Vec3 target_bpf;

  // joints are assumed to be NWU frame
  R = joint_if.toRotationMatrix();

  // transform target in camera frame to body frame
  p = this->getTargetPositionInBodyFrame(target_cf);

  // transform target in body frame to body planar frame
  target_bpf = R * p;

  return target_bpf;
}

void AprilTagNode::trackTarget(void) {
  double dist;
  Vec3 setpoints;

  // calculate roll pitch yaw setpoints
  dist = this->target_bpf.norm();
  setpoints(0) = asin(this->target_bpf(1) / dist);   // roll
  setpoints(1) = -asin(this->target_bpf(0) / dist);  // pitch
  setpoints(2) = 0.0;                                // yaw

  this->publishGimbalSetpointAttitudeMsg(setpoints);
}

}  // end of awesomo namespace

int main(int argc, char **argv) {
  awesomo::AprilTagNode node(argc, argv);

  if (node.configure(NODE_NAME, NODE_RATE) != 0) {
    ROS_ERROR("Failed to configure AprilTag Node!");
    return -1;
  }
  node.loop();

  return 0;
}
