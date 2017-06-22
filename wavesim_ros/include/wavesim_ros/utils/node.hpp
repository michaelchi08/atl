#ifndef WAVESIM_ROS_UTILS_NODE_HPP
#define WAVESIM_ROS_UTILS_NODE_HPP

#include <functional>

#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/CameraInfo.h>
#include <image_transport/image_transport.h>

namespace wavesim {
namespace ros {

// INFO MESSAGES
#define INFO_CONFIG "Configuring ROS Node [%s]!"

// MACROS
#define ROS_NODE_RUN(NODE_TYPE, NODE_NAME, NODE_RATE)   \
  int main(int argc, char **argv) {                     \
    NODE_TYPE node(argc, argv);                         \
    if (node.configure(NODE_NAME, NODE_RATE) != 0) {    \
      ROS_ERROR("Failed to configure %s!", #NODE_TYPE); \
      return -1;                                        \
    }                                                   \
    node.loop();                                        \
    return 0;                                           \
  }

class ROSNode {
public:
  bool configured;
  bool debug_mode;
  bool sim_mode;

  int argc;
  char **argv;

  std::string ros_node_name;
  long long int ros_seq;
  ::ros::NodeHandle *ros_nh;
  ::ros::Rate *ros_rate;
  ::ros::Time ros_last_updated;

  std::map<std::string, ::ros::Publisher> ros_pubs;
  std::map<std::string, ::ros::Subscriber> ros_subs;
  std::map<std::string, ::ros::ServiceServer> ros_servers;
  std::map<std::string, ::ros::ServiceClient> ros_clients;

  image_transport::Publisher img_pub;
  image_transport::Subscriber img_sub;
  std::function<int(void)> loop_cb;

  ROSNode(int argc, char **argv);
  ~ROSNode(void);
  int configure(const std::string node_name, int hz);
  void shutdownCallback(const std_msgs::Bool &msg);
  int registerShutdown(std::string topic);
  int registerImagePublisher(const std::string &topic);

  template <typename M, typename T>
  int registerImageSubscriber(const std::string &topic,
                              void (T::*fp)(M),
                              T *obj,
                              uint32_t queue_size = 1) {
    // pre-check
    if (this->configured == false) {
      return -1;
    }

    // image transport
    image_transport::ImageTransport it(*this->ros_nh);
    this->img_sub = it.subscribe(topic, queue_size, fp, obj);

    return 0;
  }

  template <typename M>
  int registerPublisher(const std::string &topic,
                        uint32_t queue_size = 100,
                        bool latch = false) {
    ::ros::Publisher publisher;

    // pre-check
    if (this->configured == false) {
      return -1;
    }

    // register publisher
    publisher = this->ros_nh->advertise<M>(topic, queue_size, latch);
    this->ros_pubs[topic] = publisher;

    return 0;
  }

  template <typename M, typename T>
  int registerSubscriber(const std::string &topic,
                         void (T::*fp)(M),
                         T *obj,
                         uint32_t queue_size = 100) {
    ::ros::Subscriber subscriber;

    // pre-check
    if (this->configured == false) {
      return -1;
    }

    // register subscriber
    subscriber = this->ros_nh->subscribe(topic, queue_size, fp, obj);
    this->ros_subs[topic] = subscriber;

    return 0;
  }

  template <class T, class MReq, class MRes>
  int registerServer(const std::string &service_topic,
                     bool (T::*fp)(MReq &, MRes &),
                     T *obj) {
    ::ros::ServiceServer server;

    // pre-check
    if (this->configured == false) {
      return -1;
    }

    // register service server
    server = this->ros_nh->advertiseService(service_topic, fp, obj);
    this->ros_servers[service_topic] = server;

    return 0;
  }

  template <typename M>
  int registerClient(const std::string &service_topic,
                     bool persistent = false) {
    ::ros::ServiceClient client;

    // pre-check
    if (this->configured == false) {
      return -1;
    }

    // register service server
    client = this->ros_nh->serviceClient<M>(service_topic);
    this->ros_clients[service_topic] = client;

    return 0;
  }

  int registerLoopCallback(std::function<int(void)> cb);
  int loop(void);
};

}  // end of ros namespace
}  // end of prototype namespace
#endif