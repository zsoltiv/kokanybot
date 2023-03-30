#include "img.h"

#include "gpio.h"
#include "pwm.h"
#include "init.h"

#include <iostream>
#include <ctime>
#include <dirent.h>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <threads.h>

/* `ls -1 hazmat | wc -l` */
#define REFERENCE_COUNT 6
#define REFERENCE_DIR "/home/pi/kokanybot/hazmat"

extern "C" {
    thrd_t img_thrd;
    cnd_t   img_cnd;
    mtx_t   img_mtx;
    bool img_need_doing = false;
}

struct reference {
    cv::Mat image;
    cv::Vec3i average_color;
};

reference references[REFERENCE_COUNT];

static cv::Vec3i average_color(cv::Mat& img)
{
    int64_t pixels = img.rows * img.cols;
    int64_t bSum = 0, gSum = 0, rSum = 0;
    for(int row = 0; row < img.rows; row++) {
        uchar* col = img.ptr(row);
        for(int nCol = 0; nCol < img.cols; nCol++) {
            bSum += *(col    );
            gSum += *(col + 1);
            rSum += *(col + 2);
            ++col;
        }
    }
    cv::Vec3i avg((int) (bSum / pixels), (int) (gSum / pixels), (int) (rSum / pixels));
    std::cout << "avg " << avg << std::endl;

    return avg;
}

static void load_references(void)
{
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
        references[i].image = cv::imread(filepath, cv::IMREAD_COLOR);
        references[i].average_color = average_color(references[i].image);
        i++;
        if(i == REFERENCE_COUNT)
            break;
    }
    closedir(dir);
}

static int color_distance_percent(cv::Vec3i& avgA, cv::Vec3i& avgB)
{
    cv::Vec3i hundreds(100, 100, 100);
    cv::Vec3i percents = hundreds - cv::Vec3i(cv::pow(abs(avgA[0] - avgB[0]), 2),
                                              cv::pow(abs(avgA[1] - avgB[1]), 2),
                                              cv::pow(abs(avgA[2] - avgB[2]), 2));
    return cv::sqrt(abs(percents[0]) + abs(percents[1]) + abs(percents[2]));
}

static void rgb_reference_average_color(cv::Vec3i& ref_avg)
{
    pwm_set_duty_cycle(r, ref_avg[0]);
    pwm_set_duty_cycle(g, ref_avg[1]);
    pwm_set_duty_cycle(b, ref_avg[2]);
}

extern "C" void do_image_recognition(bool should_do)
{
    mtx_lock(&img_mtx);
    img_need_doing = should_do;
    cnd_signal(&img_cnd);
    mtx_unlock(&img_mtx);
}

extern "C" int img_thread(void *arg)
{
    mtx_lock(&init_mtx);
    int ret = thrd_success;
    cv::Vec3i red(0, 255, 0);
    cv::VideoCapture cap(0, cv::CAP_V4L);
    cv::Mat frame_rgb, frame;

    if((ret = mtx_init(&img_mtx, mtx_plain)) != thrd_success) {
        std::cerr << "mtx_init bukott\n";
        mtx_unlock(&init_mtx);
        goto finish;
    }
    if((ret = cnd_init(&img_cnd)) != thrd_success) {
        std::cerr << "cnd_init bukott\n";
        mtx_unlock(&init_mtx);
        goto finish;
    }
    if(!cap.isOpened()) {
        std::cerr << "/dev/video0 megnyitasa bukott\n";
        ret = thrd_error;
        mtx_unlock(&init_mtx);
        goto finish;
    }
    load_references();
    if(!cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280))
        fprintf(stderr, "CAP_PROP_FRAME_WIDTH unsupported\n");
    if(!cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720))
        fprintf(stderr, "CAP_PROP_FRAME_HEIGHT unsupported\n");
    if(!cap.set(cv::CAP_PROP_CONVERT_RGB, true))
        fprintf(stderr, "CAP_PROP_CONVERT_RGB unsupported\n");

    std::cout << "frames retrieved\n";
    rgb_reference_average_color(red);
    mtx_unlock(&init_mtx);

    while(true) {
        std::cerr << "img_loop\n";
        mtx_lock(&img_mtx);

        while(!img_need_doing)
            cnd_wait(&img_cnd, &img_mtx);

        std::cerr << "RECOGNITION\n";

        cap >> frame_rgb;
        cv::cvtColor(frame_rgb, frame, cv::COLOR_RGB2BGR);

        ret = cv::imwrite("/home/pi/captured.png", frame_rgb);
        std::cout << "wrote image\n";

        cv::Vec3i avg = average_color(frame);
        cv::Vec3i ref_avg(255, 0, 0);
        int best = 0, best_distance = 0;
        for(int i = 0; i < REFERENCE_COUNT; i++) {
            int color_distance = color_distance_percent(avg, references[i].average_color);
            std::cout << "color_distance: " << color_distance << '\n';
            if(color_distance > best_distance) {
                ref_avg = references[i].average_color;
                best = i;
                best_distance = color_distance;
            }
        }
        // (255, 0, 0) if there was no match
        rgb_reference_average_color(ref_avg);
        timespec one_sec;
        one_sec.tv_sec = 1;
        thrd_sleep(&one_sec, NULL);

        img_need_doing = false;
        mtx_unlock(&img_mtx);
    }

finish:
    std::cerr << "img_thread kilep\n";
    cap.release();
    return ret;
}
