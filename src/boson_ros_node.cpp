// ROS2 wrapper for FLIR Boson camera


#include <rclcpp/rclcpp.hpp>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include "boson_camera.h"
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <camera_info_manager/camera_info_manager.hpp>
#include <image_transport/image_transport.hpp>
#include <cv_bridge/cv_bridge.h>
#include <time.h>

// Compute offset between system and monotonic clock
rclcpp::Duration get_reset_time(rclcpp::Clock & clock) {
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);

    rclcpp::Time now = clock.now();
    rclcpp::Duration epoch_duration(0, 0);
    epoch_duration = now - rclcpp::Time(monotime.tv_sec, monotime.tv_nsec, RCL_SYSTEM_TIME);

    std::cout << "Epoch Time: " << epoch_duration.seconds() << std::endl;
    return epoch_duration;
}

int main(int argc, char * argv[])
{
    // Initialize ROS2
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("boson_camera_node");

    // Parameters
    float frame_rate = 10.0f;
    std::string camera_name = "boson";
    std::string camera_info_url;
    std::string frame_id = "boson_optical_frame";
    node->declare_parameter<std::string>("camera_info_url", "");
    node->declare_parameter<float>("frame_rate", frame_rate);
    node->declare_parameter<std::string>("frame_id", frame_id);
    node->get_parameter("camera_info_url", camera_info_url);
    node->get_parameter("frame_rate", frame_rate);
    node->get_parameter("frame_id", frame_id);

    // Camera info manager
    auto cinfo_mgr = std::make_shared<camera_info_manager::CameraInfoManager>(
        node.get(), camera_name, camera_info_url);

    // Get device path from ROS parameter (default to /dev/video0)
    std::string device_id;
    node->declare_parameter<std::string>("device_id", "/dev/video0");
    node->get_parameter("device_id", device_id);

    // Validate device path
    if (access(device_id.c_str(), F_OK) == -1) {
        RCLCPP_ERROR(node->get_logger(), "Camera device not found or not accessible: %s", device_id.c_str());
        RCLCPP_ERROR(node->get_logger(), "Please check the connection or permissions (e.g., sudo chmod a+rw %s)", device_id.c_str());
        rclcpp::shutdown();
        return 1;
    }

    // Initialize Boson camera
    BosonCamera camera(device_id);
    camera.init();
    camera.allocateBuffer();
    camera.startStream();

    // Use system clock for real hardware timing
    rclcpp::Clock clock(RCL_SYSTEM_TIME);
    rclcpp::Duration epoch_duration = get_reset_time(clock);

    // Publishers (relative topic names for remapping)
    auto image_pub = image_transport::create_publisher(node.get(), "/boson/image_raw");
    // auto compressed_pub = image_transport::create_publisher(node.get(), "/boson/image_raw/compressed");
    auto camera_info_pub = node->create_publisher<sensor_msgs::msg::CameraInfo>("/boson/camera_info", 1);

    RCLCPP_INFO(node->get_logger(), "Streaming with frequency of %.1f Hz", frame_rate);
    rclcpp::Rate loop_rate(frame_rate);
    uint64_t framecount = 0;

    try {
        // Main capture loop
        while (rclcpp::ok()) {
            // Capture raw frame
            cv::Mat img = camera.captureRawFrame();
            framecount++;

            // Build header and timestamp
            std_msgs::msg::Header hdr;
            hdr.stamp = rclcpp::Time(camera.last_ts.tv_sec,
                                     camera.last_ts.tv_usec * 1000,
                                     RCL_SYSTEM_TIME) + epoch_duration;
            hdr.frame_id = frame_id;

            // Convert to ROS image message
            auto cv_image = cv_bridge::CvImage(hdr, "mono16", img);
            auto msg = cv_image.toImageMsg();
            msg->width = camera.width;
            msg->height = camera.height;

            // Publish image
            image_pub.publish(msg);

            // Publish camera info if available
            if (cinfo_mgr->isCalibrated()) {
                auto cam_info = std::make_shared<sensor_msgs::msg::CameraInfo>(
                    cinfo_mgr->getCameraInfo());
                cam_info->header = hdr;
                camera_info_pub->publish(*cam_info);
            } else if (framecount % 100 == 0) {
                RCLCPP_INFO(node->get_logger(), "Boson is not calibrated!");
            }

            rclcpp::spin_some(node);
            loop_rate.sleep();
        }
    } catch (const std::exception &e) {
        RCLCPP_ERROR(node->get_logger(), "Exception caught: %s", e.what());
    }

    // Cleanup
    camera.stopStream();
    camera.closeConnection();
    rclcpp::shutdown();
    return 0;
}
