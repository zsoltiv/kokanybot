#include "img.h"

#include "gpio.h"

#include <iostream>
#include <ctime>
#include <dirent.h>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <threads.h>

/* `ls -1 hazmat | wc -l` */
#define REFERENCE_COUNT 15
#define REFERENCE_DIR "/home/pi/kokanybot/hazmat"

extern "C" {
    thrd_t img_thrd;
    cnd_t   img_cnd;
    mtx_t   img_mtx;
    bool img_need_doing = false;
}

cv::Mat references[REFERENCE_COUNT];
cv::Vec3i reference_average_colors[REFERENCE_COUNT];

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

static void load_references(void)
{
    cv::VideoCapture cap;
    DIR *dir = opendir(REFERENCE_DIR);
    if(!dir)
        perror("opendir");
    struct dirent *e = NULL;
    int i = 0;
    while((e = readdir(dir))) {
        if(e->d_type != DT_REG)
            continue;
        std::string filepath(REFERENCE_DIR);
        filepath += "/";
        filepath += e->d_name;
        cap.open(filepath);
        cap.retrieve(references[i]);
        reference_average_colors[i] = average_color(references[i]);
        cap.release();
        i++;
    }
    closedir(dir);
}

static int color_distance_percent(cv::Vec3i& avgA, cv::Vec3i& avgB)
{
    cv::Vec3i hundreds(100, 100, 100);
    cv::Vec3i percents = hundreds - (avgA - avgB);
    return (percents.val[0] + percents.val[1] + percents.val[2]) / 3;
}

extern "C" void do_image_recognition(bool unused)
{
    mtx_lock(&img_mtx);
    img_need_doing = true;
    cnd_signal(&img_cnd);
    mtx_unlock(&img_mtx);
}

extern "C" int img_thread(void *arg)
{
    int ret = thrd_success;
    //cv::VideoCapture cap("/dev/video0", cv::CAP_FFMPEG);
    cv::VideoCapture cap(0);
    cv::Mat frame;

    if((ret = mtx_init(&img_mtx, mtx_plain)) != thrd_success) {
        std::cerr << "mtx_init bukott\n";
        goto finish;
    }
    if((ret = cnd_init(&img_cnd)) != thrd_success) {
        std::cerr << "cnd_init bukott\n";
        goto finish;
    }
    if(!cap.isOpened()) {
        std::cerr << "/dev/video0 megnyitasa bukott\n";
        ret = thrd_error;
        goto finish;
    }
    load_references();
    if(!cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280))
        fprintf(stderr, "CAP_PROP_FRAME_WIDTH unsupported\n");
    if(!cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720))
        fprintf(stderr, "CAP_PROP_FRAME_HEIGHT unsupported\n");


    while(true) {
        std::cerr << "img_loop\n";
        mtx_lock(&img_mtx);

        while(!img_need_doing)
            cnd_wait(&img_cnd, &img_mtx);

        std::cerr << "RECOGNITION\n";

        bool ret = cap.read(frame);
        std::cerr << "fotozasok: " << ret << '\n';

        cv::Vec3i avg = average_color(frame);
        bool match = false;
        for(int i = 0; i < REFERENCE_COUNT; i++) {
            int color_distance = color_distance_percent(avg, reference_average_colors[i]);
            std::cout << color_distance << '\n';
            if(color_distance > 70) {
                match = true;
                break;
            }
        }
        if(match)
            led_green();
        else
            led_red();
        timespec one_sec;
        one_sec.tv_sec = 1;
        thrd_sleep(&one_sec, NULL);

        led_blue(); // WARN ehhez kell mutex
        img_need_doing = false;
        mtx_unlock(&img_mtx);
    }

finish:
    std::cerr << "img_thread kilep\n";
    cap.release();
    return ret;
}
