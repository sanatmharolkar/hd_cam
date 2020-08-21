#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include <gst/gst.h>

#include <stdio.h>
#include <iostream>
#include <thread>
#include <future>

using namespace cv;
using namespace std;

// Set parameters for grabbing frames from each camera
int BACKEND = CAP_V4L2;
int CODEC = VideoWriter::fourcc('M', 'J', 'P', 'G');
int WINSIZE[2] = {1440, 270};
int FRAMESIZE[2] = {1280, 720};
int BUFFERSIZE = 1;

Mat grabber(VideoCapture device);
void sender(VideoWriter streamer, Mat frame);
Mat processor(Mat f1, Mat f2, Mat f3);

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

    // Create display window
    // namedWindow("stream", CV_WINDOW_AUTOSIZE);

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
    streamer.open("appsrc ! videoconvert ! x264enc tune=zerolatency byte-stream=true threads=4 ! queue ! rtph264pay pt=96 ! udpsink host=192.168.0.172 port=5000", CODEC, (double)60, Size(3 * FRAMESIZE[0], FRAMESIZE[1]), true);
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
        /* resize(f, f, Size(WINSIZE[0], WINSIZE[1]));
        imshow("stream", f);
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