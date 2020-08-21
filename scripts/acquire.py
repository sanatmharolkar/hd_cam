#!/usr/bin/env python

import sys, argparse
import numpy as np
import cv2
from concurrent.futures import ThreadPoolExecutor

BACKEND = cv2.CAP_V4L2
WIN_SIZE = (1440, 405)
FRAME_SIZE = (1920, 1080)
BUFFERSIZE = 10
CODEC = cv2.VideoWriter_fourcc(*'MJPG')


def grabber(stream):
    while True:
        ret, frame = stream.read()
        if ret is True:
            return frame


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("device_1_id", help="id of first device")
    parser.add_argument("device_2_id", help="id of second device")
    args = parser.parse_args()

    cv2.namedWindow("stream", cv2.WINDOW_AUTOSIZE)

    cap1 = cv2.VideoCapture((int)(args.device_1_id), BACKEND)
    cap1.set(cv2.CAP_PROP_FOURCC, CODEC)
    cap1.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_SIZE[0])
    cap1.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_SIZE[1])
    cap1.set(cv2.CAP_PROP_BUFFERSIZE, BUFFERSIZE)
    # cap1.set(cv2.CAP_PROP_FPS, FPS)
    cap2 = cv2.VideoCapture((int)(args.device_2_id), BACKEND)
    cap2.set(cv2.CAP_PROP_FOURCC, CODEC)
    cap2.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_SIZE[0])
    cap2.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_SIZE[1])
    cap2.set(cv2.CAP_PROP_BUFFERSIZE, BUFFERSIZE)
    # cap2.set(cv2.CAP_PROP_FPS, FPS)

    print("Frame size set to: {0}x{1}".format((int)(cap1.get(cv2.CAP_PROP_FRAME_WIDTH)), (int)(cap1.get(cv2.CAP_PROP_FRAME_HEIGHT))))
    print("Buffer size set to: {0}".format(cap1.get(cv2.CAP_PROP_BUFFERSIZE)))
    print("Frame rate is: {}".format(cap1.get(cv2.CAP_PROP_FPS)))
    print("Codec used: " + "".join([chr(((int)(cap1.get(cv2.CAP_PROP_FOURCC)) >> 8 * i) & 0xFF) for i in range(4)]))

    if (cap1.isOpened() and cap2.isOpened()):
        print("Devices {0} and {1} opened.".format((int)(args.device_1_id), (int)(args.device_2_id)))
    else:
        print("Could not open devices.")
        sys.exit()

    while True:
        with ThreadPoolExecutor() as executor:
            future1 = executor.submit(grabber, cap1)
            future2 = executor.submit(grabber, cap2)
            f1 = future1.result()
            f2 = future2.result()
            f = cv2.hconcat([f1, f2])
            f = cv2.resize(f, WIN_SIZE)
            cv2.imshow("stream", f)
            if (cv2.waitKey(1) == 27):
                break