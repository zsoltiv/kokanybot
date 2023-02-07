#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <libudev.h>

#include <libinput.h>

struct libinput_interface linterface;
struct libinput *lctx;
struct udev *uctx;

/* libinput_interface-hez callbackek */
static int open_callback(const char *path, int flags, void *user_data)
{
    if(!path)
        return ENOENT;
    int fd = open(path, flags);
    if(fd < 0)
        fprintf(stderr, "libinput nem tudta megnyitni a %s eszkozt\n", path);
    return fd;
}

static void close_callback(int fd, void *user_data)
{
    close(fd);
}

void input_init(void)
{
    linterface.open_restricted = open_callback;
    linterface.close_restricted = close_callback;
    uctx = udev_new();
    if(!uctx)
        fprintf(stderr, "udev_new() NULL-t adott vissza\n");
    lctx = libinput_udev_create_context(&linterface, NULL, uctx);
    if(libinput_udev_assign_seat(lctx, "kokanybot") < 0)
        fprintf(stderr, "libinput_udev_assign_seat() hibaba utkozott\n");
}

void input_close(void)
{
    libinput_unref(lctx);
    udev_unref(uctx);
}
