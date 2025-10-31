// ROS2 wrapper for FLIR Boson camera
#include <rclcpp/rclcpp.hpp>
#include <image_transport/image_transport.hpp>
#include <camera_info_manager/camera_info_manager.hpp>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <opencv2/opencv.hpp>
#include <unistd.h>  
#include <memory>
#include "boson_camera.h"

namespace boson_camera
{

class BosonCameraNode : public rclcpp::Node
{
public:
  explicit BosonCameraNode(const rclcpp::NodeOptions &options)
  : Node("boson_camera_node", options)
  {
    // Declare parameters
    frame_rate_ = this->declare_parameter<float>("frame_rate", 10.0);
    frame_id_   = this->declare_parameter<std::string>("frame_id", "boson_optical_frame");
    device_id_  = this->declare_parameter<std::string>("device_id", "/dev/video0");
    std::string camera_info_url = this->declare_parameter<std::string>("camera_info_url", "");

    // Validate device accessibility
    if (access(device_id_.c_str(), F_OK) == -1) {
      RCLCPP_FATAL(this->get_logger(), "Camera device not found or inaccessible: %s", device_id_.c_str());
      RCLCPP_FATAL(this->get_logger(), "Try: sudo chmod a+rw %s", device_id_.c_str());
      throw std::runtime_error("Camera device not accessible");
    }

    // Initialize Boson camera
    camera_ = std::make_unique<BosonCamera>(device_id_);
    camera_->init();
    camera_->allocateBuffer();
    camera_->startStream();

    // Camera info manager
    cinfo_mgr_ = std::make_shared<camera_info_manager::CameraInfoManager>(this, "boson", camera_info_url);

    // Publishers
    image_pub_ = image_transport::create_publisher(this, "/boson/image_raw");
    camera_info_pub_ = this->create_publisher<sensor_msgs::msg::CameraInfo>("/boson/camera_info", 10);

    // Timer for capture loop
    auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / frame_rate_));
    timer_ = this->create_wall_timer(period, std::bind(&BosonCameraNode::captureLoop, this));

    RCLCPP_INFO(this->get_logger(), " Boson camera node started at %.2f Hz using device %s",
                frame_rate_, device_id_.c_str());
  }

  ~BosonCameraNode() override
  {
    try {
      camera_->stopStream();
      camera_->closeConnection();
      RCLCPP_INFO(this->get_logger(), "Boson camera stopped and cleaned up.");
    } catch (const std::exception &e) {
      RCLCPP_WARN(this->get_logger(), "Exception during cleanup: %s", e.what());
    }
  }

private:
  void captureLoop()
  {
    try {
      cv::Mat img = camera_->captureRawFrame();

      if (img.empty()) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "Captured empty frame — check camera connection.");
        return;
      }

      std_msgs::msg::Header header;
      header.stamp = this->get_clock()->now();
      header.frame_id = frame_id_;

      // Convert OpenCV frame → ROS Image
      auto msg = cv_bridge::CvImage(header, "mono16", img).toImageMsg();
      image_pub_.publish(msg);

      // Publish camera info
      if (cinfo_mgr_->isCalibrated()) {
        auto cam_info = cinfo_mgr_->getCameraInfo();
        cam_info.header = header;
        camera_info_pub_->publish(cam_info);
      }

    } catch (const std::exception &e) {
      RCLCPP_ERROR(this->get_logger(), "Frame capture failed: %s", e.what());
    }
  }

  // Members
  std::shared_ptr<camera_info_manager::CameraInfoManager> cinfo_mgr_;
  image_transport::Publisher image_pub_;
  rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::unique_ptr<BosonCamera> camera_;
  std::string frame_id_;
  std::string device_id_;
  float frame_rate_;
};

}  // namespace boson_camera

// Register this as a ROS 2 component
#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(boson_camera::BosonCameraNode)
