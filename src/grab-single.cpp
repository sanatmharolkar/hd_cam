#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <iostream>
#include <thread>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace cv;

Mat grabber(VideoCapture device);
string to_format(const int number);

// Set parameters for grabbing frames from each camera
int BACKEND = CAP_V4L2;
int CODEC = VideoWriter::fourcc('M', 'J', 'P', 'G');
int FRAMESIZE[2] = {1280, 720};
int BUFFERSIZE = 1;

int main(int argc, char **argv) {
    int device = 0;
    device = atoi(argv[1]);
    int imgid = atoi(argv[2]);
    cout << "Using device " << device << "." << endl;

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

    for (int i = 0; i < 3; i++) {
        cout << "Reading...." << endl;
        this_thread::sleep_for(chrono::seconds(3));

        Mat f = grabber(cap);
        string imgName = "img_" + to_format(imgid) + ".png";
        imwrite(imgName, f);
    }
    
    return 0;
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
