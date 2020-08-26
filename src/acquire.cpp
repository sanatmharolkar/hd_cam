#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <ros/ros.h>
#include <ros/package.h>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <boost/filesystem.hpp>
#include <future>

using namespace cv;
using namespace std;
namespace fs = boost::filesystem;

// Stereo params for three cam setup
string pkgPath = ros::package::getPath("hd_cam");
fs::path calibPath(pkgPath + "/calib/three_cam_setup");
fs::path clParamsPath = calibPath / fs::path("cl_stereoParams.txt");
fs::path crParamsPath = calibPath / fs::path("cr_stereoParams.txt");

// Set parameters for grabbing frames from each camera
int BACKEND = CAP_V4L2;
int CODEC = VideoWriter::fourcc('M', 'J', 'P', 'G');
// int WINSIZE[2] = {1440, 270};
int FRAMESIZE[2] = {1280, 720};
int BUFFERSIZE = 1;

Mat K_c1, D_c1, K_l, D_l, R_lc, T_lc, Rect_c1, Rect_l, P_c1, P_l, Q_lc, K_c2, D_c2, K_r, D_r, R_rc, T_rc, Rect_c2, Rect_r, P_c2, P_r, Q_rc, mapc11, mapc12, mapl1, mapl2, mapc21, mapc22, mapr1, mapr2;

Mat grabber(VideoCapture device);
void sender(VideoWriter streamer, Mat frame);
Mat processor(Mat f1, Mat f2, Mat f3);
Mat vecToMat(vector<double> vec, int rows, int cols);

int main(int argc, char **argv) {
    int device1 = 0;
    int device2 = 0;
    int device3 = 0;
    if (argc < 4) {
        cout << "Please enter 3 IDs for cameras." << endl;
        return -1;
    }
    else if (argc == 4) {
        device1 = atoi(argv[1]);
        device2 = atoi(argv[2]);
        device3 = atoi(argv[3]);
        cout << "Using devices " << device1 << ", " << device2 << " and " << device3 << "." << endl;
    }
    else {
        cout << "Too many device IDs entered, please enter only 3." << endl;
        return -1;
    }

    // Read stereo params for 3 cam setup
    /* cout << "Reading pair-wise stereo params..." << endl;
    string delimiter = ":";

    cout << "Centre-Left Pair: " << endl;
    vector<vector<double>> clParams;
    ifstream clInput(clParamsPath.string());
    for (string line; getline(clInput, line); ) {
        string arr_ = line.substr(line.find(delimiter) + 1);
        vector<double> vec_;
        istringstream iss(arr_);
        copy(istream_iterator<double>(iss), istream_iterator<double>(), back_inserter(vec_));
        clParams.push_back(vec_);
        vec_.clear();
    }
    K_c1 = vecToMat(clParams[0], 3, 3);
    cout << "K_c1: " << endl << K_c1 << endl;
    D_c1 = vecToMat(clParams[1], 1, 5);
    cout << "D_c1: " << endl << D_c1 << endl;
    K_l = vecToMat(clParams[2], 3, 3);
    cout << "K_l: " << endl << K_l << endl;
    D_l = vecToMat(clParams[3], 1, 5);
    cout << "D_l: " << endl << D_l << endl;
    R_lc = vecToMat(clParams[4], 3, 3);
    cout << "R_lc: " << endl << R_lc << endl;
    T_lc = vecToMat(clParams[5], 3, 1);
    cout << "T_lc: " << endl << T_lc << endl;

    cout << "Centre-Right Pair: " << endl;
    vector<vector<double>> crParams;
    ifstream crInput(crParamsPath.string());
    for (string line; getline(crInput, line); ) {
        string arr_ = line.substr(line.find(delimiter) + 1);
        vector<double> vec_;
        istringstream iss(arr_);
        copy(istream_iterator<double>(iss), istream_iterator<double>(), back_inserter(vec_));
        crParams.push_back(vec_);
        vec_.clear();
    }
    K_c2 = vecToMat(crParams[0], 3, 3);
    cout << "K_c2: " << endl << K_c2 << endl;
    D_c2 = vecToMat(crParams[1], 1, 5);
    cout << "D_c2: " << endl << D_c2 << endl;
    K_r = vecToMat(crParams[2], 3, 3);
    cout << "K_r: " << endl << K_r << endl;
    D_r = vecToMat(crParams[3], 1, 5);
    cout << "D_r: " << endl << D_r << endl;
    R_rc = vecToMat(crParams[4], 3, 3);
    cout << "R_rc: " << endl << R_rc << endl;
    T_rc = vecToMat(crParams[5], 3, 1);
    cout << "T_rc: " << endl << T_rc << endl; */

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

    // Video capture object for second camera
    VideoCapture cap3(device3, BACKEND);
    cap3.set(CAP_PROP_FOURCC, CODEC);
    cap3.set(CAP_PROP_FRAME_WIDTH, FRAMESIZE[0]);
    cap3.set(CAP_PROP_FRAME_HEIGHT, FRAMESIZE[1]);
    cap3.set(CAP_PROP_BUFFERSIZE, BUFFERSIZE);

    // Gstreamer pipeline
    VideoWriter streamer;
    streamer.open("appsrc ! queue ! videoconvert ! x264enc tune=zerolatency byte-stream=true bitrate=3072 threads=4 ! queue ! rtph264pay pt=96 ! udpsink host=192.168.1.103 port=5000 sync=false", CODEC, (double)60, Size(3 * FRAMESIZE[0], FRAMESIZE[1]), true);
    if (!streamer.isOpened()) {
        cout << "Could not open streamer." << endl;
        return -1;
    }
    else {
        cout << "Stream opened." << endl;
    }

    // Make sure both cameras are opened
    if (cap1.isOpened() && cap2.isOpened() && cap3.isOpened()) {
        cout << "Devices " << device1 << ", " << device2 << " and " << device3 << " opened." << endl;
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

    // Run loop to grab frames and stream
    while (1) {
        future<Mat> future1 = async(launch::async, grabber, cap1);
        future<Mat> future2 = async(launch::async, grabber, cap2);
        future<Mat> future3 = async(launch::async, grabber, cap3);
        Mat f1 = future1.get();
        Mat f2 = future2.get();
        Mat f3 = future3.get();

        future<Mat> futureF = async(launch::async, processor, f1, f2, f3);
        Mat f = futureF.get();
        
        future<void> futureS = async(launch::async, sender, streamer, f);
        // resize(f, f, Size(WINSIZE[0], WINSIZE[1]));
        /* imshow("stream", f);
        if ((char)waitKey(1) == 27) {
            break;
        } */
    }

    return 0;
}

// Function to read a frame from a VideoCapture object, which is called in the main loop asynchronously
Mat grabber(VideoCapture device) {
    while(1) {
        Mat frame;
        bool ret = device.read(frame);
        if (ret) {
            return frame;
        }
    }
}

// Gstreamer sender for streaming the video
void sender(VideoWriter streamer, Mat frame) {
    streamer << frame;
}

// Frames processor
Mat processor(Mat f1, Mat f2, Mat f3) {
    Mat f_, f;
    hconcat(f1, f2, f_);
    hconcat(f_, f3, f);

    return f;
}

// Vec to Mat
Mat vecToMat(vector<double> vec, int rows, int cols) {
    Mat m = Mat(rows, cols, CV_64FC1);
    memcpy(m.data, vec.data(), vec.size() * sizeof(double));
    cout << m << endl;
    return m;
}
