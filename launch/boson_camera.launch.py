from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Launch arguments
    respawn = LaunchConfiguration('respawn')
    debug = LaunchConfiguration('debug')
    device_id = LaunchConfiguration('device_id')
    boson_name = LaunchConfiguration('boson_name')
    boson_config_file = LaunchConfiguration('boson_config_file')

    return LaunchDescription([
        # Common args
        DeclareLaunchArgument('respawn', default_value='false'),
        DeclareLaunchArgument('debug', default_value='false'),
        DeclareLaunchArgument('device_id',
                              default_value='/dev/v4l/by-id/usb-FLIR_Boson_431561-video-index'),
        DeclareLaunchArgument('boson_name', default_value='boson'),
        DeclareLaunchArgument('boson_config_file',
                              default_value='config/boson640_config.yaml'),

        # Boson camera node
        Node(
            package='boson_camera',
            executable='boson_camera_node',
            name=boson_name,
            output='screen',
            respawn=respawn,
            prefix=[('gdb -ex run --args ' if debug == 'true' else '')],
            arguments=[device_id],
            parameters=[{
                'camera_info_url': 'file://config/calibration/boson640.yaml',
                'frame_id': boson_optical_frame,
            }],
        ),
    ])
