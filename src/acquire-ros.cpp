#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <cv_bridge/cv_bridge.h>

#include <stdio.h>
#include <iostream>
#include <thread>
#include <future>

using namespace cv;
using namespace std;
using namespace ros;

// Set parameters for grabbing frames from each camera
int BACKEND = CAP_V4L2;
int CODEC = VideoWriter::fourcc('M', 'J', 'P', 'G');
int FRAMESIZE[2] = {1280, 720};
int BUFFERSIZE = 5;

Mat grabber(VideoCapture device);

int main(int argc, char **argv) {
    int device = 0;
    if (argc < 2) {
        cout << "Please enter 1 ID for camera." << endl;
        return -1;
    }
    else if (argc == 2) {
        device = atoi(argv[1]);
        cout << "Using device " << device << "." << endl;
    }
    else {
        cout << "Too many device IDs entered, please enter only 1." << endl;
        return -1;
    }

    init(argc, argv, "hd_cam_node");
    NodeHandle nh;
    string camTopic = "hd_cam_" + to_string(device);
    Publisher imgPub = nh.advertise<sensor_msgs::Image>(camTopic, 1);

    // Video capture object for first camera
    VideoCapture cap(device, BACKEND);
    cap.set(CAP_PROP_FOURCC, CODEC);
    cap.set(CAP_PROP_FRAME_WIDTH, FRAMESIZE[0]);
    cap.set(CAP_PROP_FRAME_HEIGHT, FRAMESIZE[1]);
    cap.set(CAP_PROP_BUFFERSIZE, BUFFERSIZE);

    // Make sure camera is opened
    if (cap.isOpened()) {
        cout << "Device " << device << " opened." << endl;
    }
    else {
        cout << "Could not open devices." << endl;
        return -1;
    }

    // Print parameters to screen
    cout << "Frame size set to: " << (int)cap.get(CAP_PROP_FRAME_WIDTH) << "x" << (int)cap.get(CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Buffer size set to: " << cap.get(CAP_PROP_BUFFERSIZE) << endl;
    int ex = (int)(cap.get(CAP_PROP_FOURCC));
    char fourcc_c[] = {(char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0};
    cout << "Codec used: " << fourcc_c << endl;

    // Run loop to grab frames and stream
    while (ok()) {
        future<Mat> futureF = async(launch::async, grabber, cap);
        Mat f = futureF.get();
        cv_bridge::CvImage rosImg;
        rosImg.header.frame_id = "hd_cam";
        rosImg.header.stamp = Time::now();
        rosImg.encoding = sensor_msgs::image_encodings::BGR8;
        rosImg.image = f;
        imgPub.publish(rosImg.toImageMsg());
        spinOnce();
    }

    return 0;
}

// Function to read a frame from a VideoCapture object, which is called in the main loop asynchronously
Mat grabber(VideoCapture device) {
    Mat frame;
    bool ret = device.read(frame);
    if (ret) {
        return frame;
    }
}