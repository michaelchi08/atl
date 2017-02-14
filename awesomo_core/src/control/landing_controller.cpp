#include "awesomo_core/control/landing_controller.hpp"


namespace awesomo {

// TRAJECTORY
Trajectory::Trajectory(void) {
  this->loaded = false;
  this->pos.clear();
  this->vel.clear();
  this->inputs.clear();
}

int Trajectory::load(std::string filepath) {
  MatX traj_data;
  Vec2 pos, vel, inputs;

  // pre-check
  if (file_exists(filepath) == false) {
    log_err("File not found: %s", filepath.c_str());
    return -1;
  }

  // load trajectory file
  // assumes each column is: x,vx,z,vz,az,theta
  this->reset();
  csv2mat(filepath, true, traj_data);
  if (traj_data.rows() == 0) {
    log_err(ETROWS, filepath.c_str());
    return -2;
  } else if (traj_data.cols() != 6) {
    log_err(ETCOLS, filepath.c_str());
    return -2;
  }

  // set trajectory class
  for (int i = 0; i < traj_data.rows(); i++) {
    pos << traj_data(i, 0), traj_data(i, 2);
    vel << traj_data(i, 1), traj_data(i, 3);
    inputs << traj_data(i, 4), traj_data(i, 5);

    this->pos.push_back(pos);
    this->vel.push_back(vel);
    this->inputs.push_back(inputs);
  }

  this->loaded = true;
  return 0;
}

int Trajectory::update(Vec3 target_pos_bf, Vec2 &wp_pos, Vec2 &wp_vel) {
  int retval;
  Vec2 wp_pos_start, wp_pos_end, wp_pos_last, p;

  // pre-check
  if (this->loaded == false) {
    return -1;
  } else if (this->pos.size() < 2) {
    wp_pos = this->pos.at(0);
    wp_vel = this->vel.at(0);
    return 0;
  }

  // setup
  wp_pos_start = this->pos.at(0);
  wp_pos_end = this->pos.at(1);
  wp_pos_last = this->pos.back();
  p(0) = wp_pos_last(0) - target_pos_bf(0);
  p(1) = target_pos_bf(2) * -1;

  // find next waypoint position and velocity
  retval = closest_point(wp_pos_start, wp_pos_end, p, wp_pos);
  wp_vel = this->vel.at(1);

  // update trajectory waypoints
  if (retval == 2) {
    this->pos.pop_front();
    this->vel.pop_front();
    this->inputs.pop_front();
  }

  wp_pos = wp_pos_last;
  // if (wp_pos(1) < 0.2) {
  //   wp_pos(1) = 0.2;
  // }

  std::cout << "target_pos_bf: " << target_pos_bf.transpose() << std::endl;
  std::cout << "position: " << p.transpose() << std::endl;
  std::cout << "wp_start: " << wp_pos_start.transpose() << std::endl;
  std::cout << "wp_end: " << wp_pos_end.transpose() << std::endl;
  std::cout << "wp_last: " << wp_pos_last.transpose() << std::endl;
  std::cout << "wp_vel: " << wp_vel.transpose() << std::endl;
  std::cout << "wp: " << wp_pos.transpose() << std::endl;
  std::cout << "waypoints: " << this->pos.size() << std::endl;
  std::cout << std::endl;

  return 0;
}

void Trajectory::reset(void) {
  this->pos.clear();
  this->vel.clear();
  this->inputs.clear();
  this->loaded = false;
}


// TRAJECTORY INDEX
TrajectoryIndex::TrajectoryIndex(void) {
  this->loaded = false;

  this->traj_dir = "";
  this->index_data = MatX();
  this->pos_thres = 0.0;
  this->vel_thres = 0.0;
}

int TrajectoryIndex::load(std::string index_file,
                          double pos_thres,
                          double vel_thres) {
  // pre-check
  if (file_exists(index_file) == false) {
    log_err("File not found: %s", index_file.c_str());
    return -1;
  }

  // load trajectory index
  // assumes each column is: (index, p0_x, p0_z, pf_x, pf_z, z)
  csv2mat(index_file, true, this->index_data);
  this->traj_dir = std::string(dirname((char *) index_file.c_str()));
  this->pos_thres = pos_thres;
  this->vel_thres = vel_thres;

  if (this->index_data.rows() == 0) {
    log_err(ETIROWS, index_file.c_str());
    return -2;
  } else if (this->index_data.cols() != 6) {
    log_err(ETICOLS, index_file.c_str());
    return -2;
  }

  this->loaded = true;
  return 0;
}

int TrajectoryIndex::find(Vec2 p0, Vec2 pf, double v, Trajectory &traj) {
  bool p_ok, v_ok;
  std::vector<double> p0_matches;
  std::vector<VecX> matches;
  std::string traj_file;

  // pre-check
  if (this->loaded == false) {
    return -1;
  }

  // NOTE: the following is not the most efficient way of implementing a lookup
  // table, a better way could involve a search tree and traverse it or even a
  // bucket based approach. The following implements a list traversal type
  // search which is approx O(n), ok for small lookups.

  // find rows in the index that have same approx
  // start height (p0_z) and velocity (v)
  for (int i = 0; i < this->index_data.rows(); i++) {
    p_ok = fabs(p0(1) - this->index_data(i, 2)) < this->pos_thres;
    v_ok = fabs(v - this->index_data(i, 5)) < this->vel_thres;

    if (p_ok && v_ok) {
      p0_matches.push_back(i);
    }
  }

  // filter those that have approx end position (pf_x)
  for (int i = 0; i < p0_matches.size(); i++) {
    if (fabs(pf(0) - this->index_data(p0_matches[i], 3)) < this->pos_thres) {
      matches.push_back(this->index_data.row(p0_matches[i]));
    }
  }
  if (matches.size() == 0) {
    return -2;  // found no trajectory
  }

  // load trajectory
  traj_file = this->traj_dir + "/";
  traj_file += std::to_string((int) matches[0](0)) + ".csv";
  if (traj.load(traj_file) != 0) {
    return -3;
  }

  return 0;
}


// LANDING CONTROLLER
LandingController::LandingController(void) {
  this->configured = false;

  this->pctrl_dt = 0.0;
  this->x_controller = PID(0.0, 0.0, 0.0);
  this->y_controller = PID(0.0, 0.0, 0.0);
  this->z_controller = PID(0.0, 0.0, 0.0);
  this->hover_throttle = 0.0;

  this->vctrl_dt = 0.0;
  this->vx_controller = PID(0.0, 0.0, 0.0);
  this->vy_controller = PID(0.0, 0.0, 0.0);
  this->vz_controller = PID(0.0, 0.0, 0.0);

  this->roll_limit[0] = 0.0;
  this->roll_limit[1] = 0.0;
  this->pitch_limit[0] = 0.0;
  this->pitch_limit[1] = 0.0;
  this->throttle_limit[0] = 0.0;
  this->throttle_limit[1] = 0.0;

  this->pctrl_setpoints << 0.0, 0.0, 0.0;
  this->pctrl_outputs << 0.0, 0.0, 0.0, 0.0;
  this->vctrl_setpoints << 0.0, 0.0, 0.0;
  this->vctrl_outputs << 0.0, 0.0, 0.0, 0.0;
  this->att_cmd = AttitudeCommand();
}

int LandingController::configure(std::string config_file) {
  ConfigParser parser;
  std::string config_dir;
  std::string traj_index_file;

  // load config
  // clang-format off
  parser.addParam<double>("roll_controller.k_p", &this->y_controller.k_p);
  parser.addParam<double>("roll_controller.k_i", &this->y_controller.k_i);
  parser.addParam<double>("roll_controller.k_d", &this->y_controller.k_d);

  parser.addParam<double>("pitch_controller.k_p", &this->x_controller.k_p);
  parser.addParam<double>("pitch_controller.k_i", &this->x_controller.k_i);
  parser.addParam<double>("pitch_controller.k_d", &this->x_controller.k_d);

  parser.addParam<double>("throttle_controller.k_p", &this->z_controller.k_p);
  parser.addParam<double>("throttle_controller.k_i", &this->z_controller.k_i);
  parser.addParam<double>("throttle_controller.k_d", &this->z_controller.k_d);
  parser.addParam<double>("throttle_controller.hover_throttle", &this->hover_throttle);

  parser.addParam<double>("vx_controller.k_p", &this->vx_controller.k_p);
  parser.addParam<double>("vx_controller.k_i", &this->vx_controller.k_i);
  parser.addParam<double>("vx_controller.k_d", &this->vx_controller.k_d);

  parser.addParam<double>("vy_controller.k_p", &this->vy_controller.k_p);
  parser.addParam<double>("vy_controller.k_i", &this->vy_controller.k_i);
  parser.addParam<double>("vy_controller.k_d", &this->vy_controller.k_d);

  parser.addParam<double>("vz_controller.k_p", &this->vz_controller.k_p);
  parser.addParam<double>("vz_controller.k_i", &this->vz_controller.k_i);
  parser.addParam<double>("vz_controller.k_d", &this->vz_controller.k_d);

  parser.addParam<double>("roll_limit.min", &this->roll_limit[0]);
  parser.addParam<double>("roll_limit.max", &this->roll_limit[1]);

  parser.addParam<double>("pitch_limit.min", &this->pitch_limit[0]);
  parser.addParam<double>("pitch_limit.max", &this->pitch_limit[1]);

  parser.addParam<double>("throttle_limit.min", &this->throttle_limit[0]);
  parser.addParam<double>("throttle_limit.max", &this->throttle_limit[1]);

  parser.addParam<std::string>("trajectory_index", &traj_index_file);
  // clang-format on
  if (parser.load(config_file) != 0) {
    return -1;
  }

  // load trajectory index
  config_dir = std::string(dirname((char *) config_file.c_str()));
  paths_combine(config_dir, traj_index_file, traj_index_file);
  if (this->traj_index.load(traj_index_file) != 0) {
    return -2;
  }

  // convert roll and pitch limits from degrees to radians
  this->roll_limit[0] = deg2rad(this->roll_limit[0]);
  this->roll_limit[1] = deg2rad(this->roll_limit[1]);
  this->pitch_limit[0] = deg2rad(this->pitch_limit[0]);
  this->pitch_limit[1] = deg2rad(this->pitch_limit[1]);

  // initialize setpoints to zero
  this->pctrl_setpoints << 0.0, 0.0, 0.0;
  this->pctrl_outputs << 0.0, 0.0, 0.0, 0.0;
  this->vctrl_setpoints << 0.0, 0.0, 0.0;
  this->vctrl_outputs << 0.0, 0.0, 0.0, 0.0;

  this->configured = true;
  return 0;
}

int LandingController::loadTrajectory(Vec3 pos,
                                      Vec3 target_pos_bf,
                                      double v) {
  Vec2 quad, target;
  int retval;

  quad << 0.0, pos(2);
  target << target_pos_bf(0), pos(2) + target_pos_bf(2);
  retval = this->traj_index.find(quad, target, v, this->trajectory);

  if (retval == -2) {
    log_err(ETIFAIL, quad(0), quad(1), target(0), target(1), v);
    return -1;
  } else if (retval == -3) {
    log_err(ETLOAD);
    return -1;
  } else {
    log_info(TLOAD, quad(0), quad(1), target(0), target(1), v);
  }

  return 0;
}

Vec4 LandingController::calculatePositionErrors(Vec3 errors,
                                                double yaw,
                                                double dt) {
  double r, p, y, t;
  Vec3 euler;
  Mat3 R;

  // check rate
  this->pctrl_dt += dt;
  if (this->pctrl_dt < 0.01) {
    return this->pctrl_outputs;
  }

  // roll, pitch, yaw and throttle (assuming NWU frame)
  // clang-format off
  r = -this->y_controller.calculate(errors(1), 0.0, this->pctrl_dt);
  p = this->x_controller.calculate(errors(0), 0.0, this->pctrl_dt);
  y = yaw;
  t = this->hover_throttle + this->z_controller.calculate(errors(2), 0.0, this->pctrl_dt);
  t /= fabs(cos(r) * cos(p));  // adjust throttle for roll and pitch
  // clang-format o

  // limit roll, pitch
  r = (r < this->roll_limit[0]) ? this->roll_limit[0] : r;
  r = (r > this->roll_limit[1]) ? this->roll_limit[1] : r;
  p = (p < this->pitch_limit[0]) ? this->pitch_limit[0] : p;
  p = (p > this->pitch_limit[1]) ? this->pitch_limit[1] : p;

  // limit throttle
  t = (t < 0) ? 0.0 : t;
  t = (t > 1.0) ? 1.0 : t;

  // keep track of setpoints and outputs
  this->pctrl_setpoints = errors;
  this->pctrl_outputs << r, p, y, t;
  this->pctrl_dt = 0.0;

  return pctrl_outputs;
}

Vec4 LandingController::calculateVelocityErrors(Vec3 errors,
                                                double yaw,
                                                double dt) {
  double r, p, y, t;
  Vec3 euler;
  Mat3 R;

  // check rate
  this->vctrl_dt += dt;
  if (this->vctrl_dt < 0.01) {
    return this->vctrl_outputs;
  }

  // roll, pitch, yaw and throttle (assuming NWU frame)
  // clang-format off
  r = -this->vy_controller.calculate(errors(1), 0.0, this->vctrl_dt);
  p = this->vx_controller.calculate(errors(0), 0.0, this->vctrl_dt);
  y = 0.0;
  t = this->vz_controller.calculate(errors(2), 0.0, this->vctrl_dt);
  t /= fabs(cos(r) * cos(p));  // adjust throttle for roll and pitch
  // clang-format on

  // limit roll, pitch, throttle
  r = (r < this->roll_limit[0]) ? this->roll_limit[0] : r;
  r = (r > this->roll_limit[1]) ? this->roll_limit[1] : r;
  p = (p < this->pitch_limit[0]) ? this->pitch_limit[0] : p;
  p = (p > this->pitch_limit[1]) ? this->pitch_limit[1] : p;
  t = (t < this->throttle_limit[0]) ? this->throttle_limit[0] : t;
  t = (t > this->throttle_limit[1]) ? this->throttle_limit[1] : t;

  // keep track of setpoints and outputs
  this->vctrl_setpoints = errors;
  this->vctrl_outputs << r, p, y, t;
  this->vctrl_dt = 0.0;

  return this->vctrl_outputs;
}

AttitudeCommand LandingController::calculate(Vec3 pos_errors,
                                             Vec3 vel_errors,
                                             double yaw,
                                             double dt) {
  this->calculatePositionErrors(pos_errors, yaw, dt);
  this->calculateVelocityErrors(vel_errors, yaw, dt);
  this->att_cmd = AttitudeCommand(this->pctrl_outputs + this->vctrl_outputs);
  return this->att_cmd;
}

AttitudeCommand LandingController::calculate(Vec3 target_pos_bf,
                                             Vec3 target_vel_bf,
                                             Vec3 pos,
                                             Vec3 pos_prev,
                                             double yaw,
                                             double dt) {
  int retval;
  Vec3 perrors, verrors;
  Vec2 wp_pos, wp_vel;
  double vz;

  // obtain position and velocity waypoints
  retval = this->trajectory.update(target_pos_bf, wp_pos, wp_vel);

  // calculate position and velocity errors
  // perrors(0) = target_pos_bf(0);
  // perrors(1) = target_pos_bf(1);
  // perrors(2) = wp_pos(1) - pos(2);

  vz = (pos(2) - pos_prev(2)) / dt;
  verrors(0) = wp_vel(0);
  verrors(1) = target_vel_bf(1);
  verrors(2) = wp_vel(1) - vz;

  // control
  return this->calculate(perrors, verrors, yaw, dt);
}

void LandingController::reset(void) {
  this->x_controller.reset();
  this->y_controller.reset();
  this->z_controller.reset();

  this->vx_controller.reset();
  this->vy_controller.reset();
  this->vz_controller.reset();
}

void LandingController::printOutputs(void) {
  double r, p, t;

  r = rad2deg(this->pctrl_outputs(0));
  p = rad2deg(this->pctrl_outputs(1));
  t = this->pctrl_outputs(3);

  std::cout << "roll: " << std::setprecision(2) << r << "\t";
  std::cout << "pitch: " << std::setprecision(2) << p << "\t";
  std::cout << "throttle: " << std::setprecision(2) << t << std::endl;
}

void LandingController::printErrors(void) {
  double p, i, d;

  p = this->x_controller.error_p;
  i = this->x_controller.error_i;
  d = this->x_controller.error_d;

  std::cout << "x_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << i << std::endl;

  p = this->y_controller.error_p;
  i = this->y_controller.error_i;
  d = this->y_controller.error_d;

  std::cout << "y_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << i << std::endl;

  p = this->z_controller.error_p;
  i = this->z_controller.error_i;
  d = this->z_controller.error_d;

  std::cout << "z_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << i << std::endl;
}

}  // end of awesomo namespace
