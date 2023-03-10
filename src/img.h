#ifndef IMG_H
#define IMG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC_NO_THREADS__
#error "threads.h not supported"
#endif

#include <stdbool.h>
#include <threads.h>

void do_image_recognition(bool unused);
int img_thread(void *arg);

extern thrd_t img_thrd;
extern cnd_t   img_cnd;
extern mtx_t   img_mtx;
extern bool img_need_doing;

#ifdef __cplusplus
}
#endif

#endif /* IMG_H */
