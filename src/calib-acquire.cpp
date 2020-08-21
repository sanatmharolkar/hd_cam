#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>
#include <boost/filesystem.hpp>

#include <ros/ros.h>
#include <ros/package.h>

using namespace std;
using namespace cv;
namespace fs = boost::filesystem;

Mat grabber(VideoCapture device);
string to_format(const int number);
int stereoCalib(int device1, int device2);
int singleCalib(int device);

string pkgPath = ros::package::getPath("hd_cam");
fs::path calibPath(pkgPath + "/calib");
time_t t = time(0);
tm *now = localtime(&t);
fs::path outputPath(calibPath.string() + "/" + to_string(now->tm_year + 1900) + "_" + to_format(now->tm_mon + 1) + "_" + to_format(now->tm_mday));
fs::path f1Path = outputPath / fs::path("f1");
fs::path f2Path = outputPath / fs::path("f2");
fs::path fPath = outputPath / fs::path("f");

// Set parameters for grabbing frames from each camera
int BACKEND = CAP_V4L2;
int CODEC = VideoWriter::fourcc('M', 'J', 'P', 'G');
int FRAMESIZE[2] = {1280, 720};
int BUFFERSIZE = 1;

int main(int argc, char **argv) {
    int device1 = 0;
    int device2 = 0;
    int device = 0;
    int mode = 1;
    if (argc < 2) {
        cout << "Please enter IDs for cameras." << endl;
        return -1;
    }
    else if (argc == 2) {
        device = atoi(argv[1]);
        cout << "Using only 1 camera: device " << device << "." << endl;
    }
    else if (argc == 3) {
        device1 = atoi(argv[1]);
        device2 = atoi(argv[2]);
        cout << "Using devices " << device1 << " and " << device2 << "." << endl;
        mode = 2;
    }
    else {
        cout << "Too many device IDs entered, please enter only 2." << endl;
        return -1;
    }

    int retval = 0;
    if (mode == 2) {
        retval = stereoCalib(device1, device2);
    }
    else {
        retval = singleCalib(device);
    }

    return retval;
}

string to_format(const int number) {
    stringstream ss;
    ss << setw(2) << setfill('0') << number;
    return ss.str();
}

Mat grabber(VideoCapture device) {
    Mat frame;
    bool ret = device.read(frame);
    if (ret) {
        return frame;
    }
}

int singleCalib(int device) {
    if (!fs::exists(calibPath)) {
        fs::create_directory(calibPath);
    }
    if (!fs::exists(outputPath)) {
        fs::create_directory(outputPath);
    }
    if (!fs::exists(fPath)) {
        fs::create_directory(fPath);
    }

    // Video capture object for camera
    VideoCapture cap(device, BACKEND);
    cap.set(CAP_PROP_FOURCC, CODEC);
    cap.set(CAP_PROP_FRAME_WIDTH, FRAMESIZE[0]);
    cap.set(CAP_PROP_FRAME_HEIGHT, FRAMESIZE[1]);
    cap.set(CAP_PROP_BUFFERSIZE, BUFFERSIZE);

    // Make sure both cameras are opened
    if (cap.isOpened()) {
        cout << "Device " << device << " opened." << endl;
    }
    else {
        cout << "Could not open device." << endl;
        return -1;
    }

    // Print parameters to screen
    cout << "Frame size set to: " << (int)cap.get(CAP_PROP_FRAME_WIDTH) << "x" << (int)cap.get(CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Buffer size set to: " << cap.get(CAP_PROP_BUFFERSIZE) << endl;
    int ex = (int)(cap.get(CAP_PROP_FOURCC));
    char fourcc_c[] = {(char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0};
    cout << "Codec used: " << fourcc_c << endl;

    for (int i = 0; i < 30; i++) {
        cout << "Reading...." << endl;
        this_thread::sleep_for(chrono::seconds(3));

        Mat f = grabber(cap);

        string f_file = fPath.string() + "/" + to_format(i) + ".png";
        imwrite(f_file, f);

        cout << "Image " << to_format(i) << " saved." << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}

int stereoCalib(int device1, int device2) {
    if (!fs::exists(calibPath)) {
        fs::create_directory(calibPath);
    }
    if (!fs::exists(outputPath)) {
        fs::create_directory(outputPath);
    }
    if (!fs::exists(f1Path)) {
        fs::create_directory(f1Path);
    }
    if (!fs::exists(f2Path)) {
        fs::create_directory(f2Path);
    }

    // Video capture object for first camera
    VideoCapture cap1(device1, BACKEND);
    cap1.set(CAP_PROP_FOURCC, CODEC);
    cap1.set(CAP_PROP_FRAME_WIDTH, FRAMESIZE[0]);
    cap1.set(CAP_PROP_FRAME_HEIGHT, FRAMESIZE[1]);
    cap1.set(CAP_PROP_BUFFERSIZE, BUFFERSIZE);

    // Video capture object for second camera
    VideoCapture cap2(device2, BACKEND);
    cap2.set(CAP_PROP_FOURCC, CODEC);
    cap2.set(CAP_PROP_FRAME_WIDTH, FRAMESIZE[0]);
    cap2.set(CAP_PROP_FRAME_HEIGHT, FRAMESIZE[1]);
    cap2.set(CAP_PROP_BUFFERSIZE, BUFFERSIZE);

    // Make sure both cameras are opened
    if (cap1.isOpened() && cap2.isOpened()) {
        cout << "Devices " << device1 << " and " << device2 << " opened." << endl;
    }
    else {
        cout << "Could not open devices." << endl;
        return -1;
    }

    // Print parameters to screen
    cout << "Frame size set to: " << (int)cap1.get(CAP_PROP_FRAME_WIDTH) << "x" << (int)cap1.get(CAP_PROP_FRAME_HEIGHT) << endl;
    cout << "Buffer size set to: " << cap1.get(CAP_PROP_BUFFERSIZE) << endl;
    int ex = (int)(cap1.get(CAP_PROP_FOURCC));
    char fourcc_c[] = {(char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0};
    cout << "Codec used: " << fourcc_c << endl;

    for (int i = 0; i < 30; i++) {
        cout << "Reading...." << endl;
        this_thread::sleep_for(chrono::seconds(3));

        Mat f1 = grabber(cap1);
        Mat f2 = grabber(cap2);

        string f1_file = f1Path.string() + "/" + to_format(i) + ".png";
        imwrite(f1_file, f1);
        string f2_file = f2Path.string() + "/" + to_format(i) + ".png";
        imwrite(f2_file, f2);

        cout << "Image " << to_format(i) << " saved." << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}