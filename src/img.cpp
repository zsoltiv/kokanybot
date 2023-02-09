#include "img.h"

#include <iostream>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/highgui.hpp>

static cv::Vec3i average_color(cv::Mat& img)
{
    int pixels = img.rows * img.cols;
    int bSum = 0, gSum = 0, rSum = 0;
    for(int row = 0; row < img.rows; row++) {
        uchar* col = img.ptr(row);
        for(int nCol = 0; nCol < img.cols; nCol++) {
            bSum += *(col    );
            gSum += *(col + 1);
            rSum += *(col + 2);
            ++col;
        }
    }
    std::cout << bSum << " " << gSum << " " << rSum << std::endl;
    cv::Vec3i avg((bSum / pixels), (gSum / pixels), (rSum / pixels));

    return avg;
}

static int color_distance_percent(cv::Vec3i& avgA, cv::Vec3i& avgB)
{
    cv::Vec3i hundreds(100, 100, 100);
    cv::Vec3i percents = hundreds - (avgA - avgB);
    return (percents.val[0] + percents.val[1] + percents.val[2]) / 3;
}

extern "C" void do_image_recognition(void)
{
    cv::VideoCapture cap("electricity_sample.webp");
    cv::Mat sample;
    cap.read(sample);

    std::cout << average_color(sample) << std::endl;

    cap.open("electricity_sample.webp");
    cv::Mat testData;
    cap.read(testData);

    cap.release();

    cv::Vec3i avg1 = average_color(sample);
    cv::Vec3i avg2 = average_color(testData);
    std::cout << color_distance_percent(avg1, avg2) << std::endl;
}

