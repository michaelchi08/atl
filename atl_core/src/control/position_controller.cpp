#include "atl/control/position_controller.hpp"

namespace atl {

int PositionController::configure(const std::string &config_file) {
  ConfigParser parser;

  // load config
  parser.addParam("roll_controller.k_p", &this->y_controller.k_p);
  parser.addParam("roll_controller.k_i", &this->y_controller.k_i);
  parser.addParam("roll_controller.k_d", &this->y_controller.k_d);
  parser.addParam("roll_controller.min", &this->roll_limit[0]);
  parser.addParam("roll_controller.max", &this->roll_limit[1]);

  parser.addParam("pitch_controller.k_p", &this->x_controller.k_p);
  parser.addParam("pitch_controller.k_i", &this->x_controller.k_i);
  parser.addParam("pitch_controller.k_d", &this->x_controller.k_d);
  parser.addParam("pitch_controller.min", &this->pitch_limit[0]);
  parser.addParam("pitch_controller.max", &this->pitch_limit[1]);

  parser.addParam("throttle_controller.k_p", &this->z_controller.k_p);
  parser.addParam("throttle_controller.k_i", &this->z_controller.k_i);
  parser.addParam("throttle_controller.k_d", &this->z_controller.k_d);
  parser.addParam(
    "throttle_controller.hover_throttle", &this->hover_throttle);

  if (parser.load(config_file) != 0) {
    return -1;
  }

  // convert roll and pitch limits from degrees to radians
  this->roll_limit[0] = deg2rad(this->roll_limit[0]);
  this->roll_limit[1] = deg2rad(this->roll_limit[1]);
  this->pitch_limit[0] = deg2rad(this->pitch_limit[0]);
  this->pitch_limit[1] = deg2rad(this->pitch_limit[1]);

  this->configured = true;
  return 0;
}

Vec4 PositionController::calculate(
  Vec3 setpoints, Pose robot_pose, double yaw, double dt) {
  // check rate
  this->dt += dt;
  if (this->dt < 0.01) {
    return this->outputs;
  }

  // calculate setpoint relative to quadrotor
  Vec3 errors;
  target2bodyplanar(
    setpoints, robot_pose.position, robot_pose.orientation, errors);

  // roll, pitch, yaw and throttle (assuming NWU frame)
  // clang-format off
  double r = -this->y_controller.update(errors(1), this->dt);
  double p = this->x_controller.update(errors(0), this->dt);
  double y = yaw;
  double t = this->hover_throttle;
  t += this->z_controller.update(errors(2), this->dt);
  t /= fabs(cos(r) * cos(p));  // adjust throttle for roll and pitch
  // clang-format o

  // limit roll, pitch and throttle
  r = (r < this->roll_limit[0]) ? this->roll_limit[0] : r;
  r = (r > this->roll_limit[1]) ? this->roll_limit[1] : r;
  p = (p < this->pitch_limit[0]) ? this->pitch_limit[0] : p;
  p = (p > this->pitch_limit[1]) ? this->pitch_limit[1] : p;
  t = (t < 0) ? 0.0 : t;
  t = (t > 1.0) ? 1.0 : t;

  // set outputs
  Vec4 outputs{r, p, y, t};

  // keep track of setpoints and outputs
  this->setpoints = setpoints;
  this->outputs = outputs;
  this->dt = 0.0;

  return outputs;
}

void PositionController::reset() {
  this->x_controller.reset();
  this->y_controller.reset();
  this->z_controller.reset();
}

void PositionController::printOutputs() {
  double r = rad2deg(this->outputs(0));
  double p = rad2deg(this->outputs(1));
  double t = this->outputs(3);

  std::cout << "roll: " << std::setprecision(2) << r << "\t";
  std::cout << "pitch: " << std::setprecision(2) << p << "\t";
  std::cout << "throttle: " << std::setprecision(2) << t << std::endl;
}

void PositionController::printErrors() {
  double p = this->x_controller.error_p;
  double i = this->x_controller.error_i;
  double d = this->x_controller.error_d;

  std::cout << "x_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << d << std::endl;

  p = this->y_controller.error_p;
  i = this->y_controller.error_i;
  d = this->y_controller.error_d;

  std::cout << "y_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << d << std::endl;

  p = this->z_controller.error_p;
  i = this->z_controller.error_i;
  d = this->z_controller.error_d;

  std::cout << "z_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << d << std::endl;
}

}  // namespace atl
