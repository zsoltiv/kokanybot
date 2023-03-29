#include "img.h"

#include "gpio.h"
#include "pwm.h"

#include <iostream>
#include <ctime>
#include <dirent.h>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
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

static void print_mat_details(cv::Mat& m)
{
    std::cout << m.channels() << " channel\n";
}

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
    cv::Vec3i avg((bSum / pixels), (gSum / pixels), (rSum / pixels));
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
        references[i] = cv::imread(filepath, cv::IMREAD_COLOR);
        reference_average_colors[i] = average_color(references[i]);
        print_mat_details(references[i]);
        i++;
    }
    closedir(dir);
}

static int color_distance_percent(cv::Vec3i& avgA, cv::Vec3i& avgB)
{
    cv::Vec3i hundreds(100, 100, 100);
    cv::Vec3i percents = hundreds - cv::Vec3i(abs(avgA[0] - avgB[0]),
                                              abs(avgA[1] - avgB[1]),
                                              abs(avgA[2] - avgB[2]));
    return (percents.val[0] + percents.val[1] + percents.val[2]) / 3;
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
    int ret = thrd_success;
    cv::VideoCapture cap(0, cv::CAP_V4L);
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
    if(!cap.set(cv::CAP_PROP_CONVERT_RGB, true))
        fprintf(stderr, "CAP_PROP_CONVERT_RGB unsupported\n");

    for(int i = 0; i < 50; i++)
        cap >> frame;
    std::cout << "frames retrieved\n";

    while(true) {
        std::cerr << "img_loop\n";
        mtx_lock(&img_mtx);

        while(!img_need_doing)
            cnd_wait(&img_cnd, &img_mtx);

        std::cerr << "RECOGNITION\n";

        cap >> frame;

        ret = cv::imwrite("captured.png", frame);
        std::cout << "wrote image\n";

        cv::Vec3i avg = average_color(frame);
        cv::Vec3i ref_avg(255, 0, 0);
        bool match = false;
        for(int i = 0; i < REFERENCE_COUNT; i++) {
            ref_avg = reference_average_colors[i];
            int color_distance = color_distance_percent(avg, reference_average_colors[i]);
            std::cout << "color_distance: " << color_distance << '\n';
            if(color_distance > 70) {
                std::cout << "matched " << i << '\n';
                match = true;
                break;
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
