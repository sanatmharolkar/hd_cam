## Grab frame from HD cam and publish on ROS topic:

Use the command "rosrun hd_cam acquire-ros id" where id is the integer ID of the /dev/video* device connected.

## Grab images from stereo camera setup for calibration

Use the command "rosrun hd_cam calib-acquire id1 id2" where id1 and id2 are the integer IDs of the /dev/video* devices connected. If you enter only 1 ID, the images will be saved for only that device (used for single camera calibration). The images are stored in the ros package folder under calib/date, where "date" is the date on which the node is run. The node will capture a total of 30 images, with 4 second intervals in between.

## Grab images from 3 camera setup and stream using Gstreamer

Use the command "rosrun hd_cam acquire id1 id2 id3", where id1, id2 and id3 are the integer IDs of the three /dev/video* devices connected. The IP address to which the concatenated frame is to be sent can be modified in code. The code also contains lines to display the concatenated frame in the main loop, which can be uncommented for use.