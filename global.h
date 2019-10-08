#ifndef GLOBAL_H
#define GLOBAL_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QImage>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/videoio/videoio.hpp"

using namespace std;
using namespace cv;

#define IPADDR      "192.168.1.160"
#define PORT        8200
//#define RTSP_URL    "rtsp://admin:Aa11111111@192.168.1.64:554"
#define RTSP_URL    "rtsp://192.168.1.160:554/main"

#define SWIRIP      "192.168.1.10"
#define SWIRPORT    2001

const int SWIRWIDTH = 1280;
const int SWIRHEIGHT = 1024;
const int FRAME_PMNUM =	16;
const int FRAME_PXCNT = SWIRWIDTH * SWIRHEIGHT;
const int FRAMEBUFSIZE = FRAME_PXCNT * 2 + 64;

void msleep(unsigned int msec);

QImage mat2qimage(const Mat &mat);
Mat qimage2mat(const QImage &qimage);
Mat autoWhiteBalance(const Mat& imgin);

#endif // GLOBAL_H
