#ifndef __atl_GIMBAL_HPP__
#define __atl_GIMBAL_HPP__

#include <unistd.h>
#include <iostream>
#include <math.h>
#include <map>
#include <yaml-cpp/yaml.h>

#include "atl_core/utils/utils.hpp"
#include "atl_core/vision/gimbal/sbgc.hpp"


namespace atl {

class Gimbal {
public:
  bool configured;

  SBGC sbgc;
  Pose camera_offset;
  double limits[6];
  bool enable_tracking;

  Vec3 setpoints;
  Vec3 target_bpf;

  Vec3 imu_accel;
  Vec3 imu_gyro;
  Vec3 camera_angles;
  Vec3 frame_angles;
  Vec3 rc_angles;
  Vec3 encoder_angles;

  Gimbal(void);
  ~Gimbal(void);
  int configure(std::string config_path);
  int on(void);
  int off(void);
  static Vec3 getTargetInBF(Pose camera_offset, Vec3 target_cf);
  static Vec3 getTargetInBPF(Pose camera_offset,
                             Vec3 target_cf,
                             Quaternion joint_if);
  static Vec3 getTargetInIF(Vec3 target_bpf,
                            Vec3 gimbal_position,
                            Quaternion gimbal_frame_if);
  int getTargetInBPF(Vec3 target_cf, Vec3 &target_bpf);
  int trackTarget(Vec3 target_cf);
  int updateGimbalStates(void);
  int setAngle(double roll, double pitch);
  void printSetpoints(void);
};

}  // end of atl namespace
#endif