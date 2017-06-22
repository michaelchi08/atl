#ifndef WAVESIM_GAZEBO_CAMERA_CLIENT_HPP
#define WAVESIM_GAZEBO_CAMERA_CLIENT_HPP

#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "wavesim_gazebo/gazebo_node.hpp"

namespace wavesim {
namespace gaz {

// PUBLISH TOPICS
#define IMAGE_TOPIC "~/camera/image"

class CameraGClient : public GazeboNode {
public:
  bool connected;
  cv::Mat image;

  CameraGClient(void);
  ~CameraGClient(void);
  int configure(void);
  virtual void imageCallback(ConstImagePtr &msg);
};

}  // end of gaz namespace
}  // end of wavesim namespace
#endif
