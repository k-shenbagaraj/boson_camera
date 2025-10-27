# FLIR Boson ROS 2 Wrapper

A ROS 2 (Humble +) package providing a minimal interface to stream and republish infrared video from the **FLIR Boson 640+** camera.

[![Build Status](https://github.com/k-shenbagaraj/boson_camera/actions/workflows/build.yml/badge.svg)](https://github.com/k-shenbagaraj/boson_camera/actions/workflows/build.yml)


## Overview

This package wraps the FLIR Boson SDK and publishes the camera’s infrared stream as standard ROS 2 image topics using:

- rclcpp  
- sensor_msgs  
- image_transport  
- camera_info_manager  
- cv_bridge  

It provides:

- Real-time infrared image publishing  
- Camera info publishing  
- Compatibility with RViz2 and standard ROS 2 image pipelines  

## Prerequisites

Make sure your system has **ROS 2** installed, then install dependencies:

```bash
sudo apt update
sudo apt install ros-${ROS_DISTRO}-rclcpp ros-${ROS_DISTRO}-image-transport \
                 ros-${ROS_DISTRO}-camera-info-manager ros-${ROS_DISTRO}-cv-bridge \
                 ros-${ROS_DISTRO}-sensor-msgs libopencv-dev
```

## Build Instructions

```bash
# Create and enter a ROS 2 workspace
mkdir -p ~/ws/src
cd ~/ws/src

# Clone this repository
git clone https://github.com/k-shenbagaraj/boson_camera.git

# Build the package
cd ~/ws
colcon build --packages-select boson_camera

# Source the workspace
source install/setup.bash
```

## Connect the Camera

1. Plug in your **FLIR Boson 640** via USB. It should appear as `/dev/video0` or `/dev/v4l/by-id/...`
2. If permission is denied:
   ```bash
   sudo chmod a+rw /dev/video0
   ```
   For serial devices:
   ```bash
   sudo chmod a+rw /dev/ttyACM0
   ```
3. Verify the device path:
   ```bash
   ls /dev/v4l/by-id/
   ```
   Example output:
   ```
   usb-FLIR_Boson_XXXXXXXX-video-index0
   ```

## How to Run

### Run Directly

```bash
ros2 run boson_camera boson_camera_node /dev/video0
```

Publishes to:
```
/boson/image_raw
```

### Run Using the Launch File

```bash
ros2 launch boson_camera boson_camera.launch.py
```

If needed, edit `launch/boson_camera.launch.py` to point to your device path (e.g., `/dev/v4l/by-id/...`).



## Boson SDK Documentation

[FLIR Boson SDK Documentation](https://drive.google.com/open?id=1fuXUIu_wzB4zuVmTPbtUhoiKg0WnqEHm)

