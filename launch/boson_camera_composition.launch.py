from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode

def generate_launch_description():
    container = ComposableNodeContainer(
        name='boson_camera_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        composable_node_descriptions=[
            ComposableNode(
                package='boson_camera',
                plugin='boson_camera::BosonCameraNode',
                name='boson_camera_node',
                parameters=[{
                    'device_id': '/dev/video0',
                    'frame_id': 'boson_optical_frame',
                    'frame_rate': 10.0,
                    'camera_info_url': 'file://config/calibration/boson640.yaml'
                }]
            ),
        ],
        output='screen',
        emulate_tty=True,
    )
    return LaunchDescription([container])
