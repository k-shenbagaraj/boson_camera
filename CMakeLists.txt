cmake_minimum_required(VERSION 3.8)
project(boson_camera)

# Ensure modern C++ version

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies

find_package(ament_cmake REQUIRED)          
find_package(rclcpp REQUIRED)                
find_package(rclcpp_components REQUIRED)     
find_package(sensor_msgs REQUIRED)           
find_package(image_transport REQUIRED)      
find_package(camera_info_manager REQUIRED)   
find_package(cv_bridge REQUIRED)            
find_package(OpenCV REQUIRED)               

# Include directories

include_directories(include)

# Build the library (for component)

add_library(${PROJECT_NAME} SHARED
  src/boson_ros_node.cpp 
  src/boson_camera.cpp    
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  rclcpp_components
  sensor_msgs
  image_transport
  camera_info_manager
  cv_bridge
  OpenCV
)

# Register component

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "boson_camera::BosonCameraNode"
  EXECUTABLE boson_camera_exec
)

# Install targets

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib    
  LIBRARY DESTINATION lib    
  RUNTIME DESTINATION bin    
)

# Install launch files
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)


# Install include headers

install(DIRECTORY include/
  DESTINATION include        
)

# Export targets and dependencies for downstream packages

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  rclcpp
  rclcpp_components
  sensor_msgs
  image_transport
  camera_info_manager
  cv_bridge
  OpenCV
)

ament_package()
