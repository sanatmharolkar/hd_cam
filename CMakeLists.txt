cmake_minimum_required(VERSION 3.0.2)
project(hd_cam)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  roslib
  OpenCV REQUIRED
  cv_bridge
)

catkin_package(
  INCLUDE_DIRS include
#  LIBRARIES hd_cam
  CATKIN_DEPENDS roscpp rospy std_msgs roslib
#  DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  /usr/include/glib-2.0
  /usr/include/gstreamer-1.0
)

add_executable(acquire src/acquire.cpp)
add_executable(calib-acquire src/calib-acquire.cpp)
add_executable(acquire-ros src/acquire-ros.cpp)
add_executable(grab-single src/grab-single.cpp)

target_link_libraries(acquire
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
)

target_link_libraries(calib-acquire
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
)

target_link_libraries(acquire-ros
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
)

target_link_libraries(grab-single
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
)