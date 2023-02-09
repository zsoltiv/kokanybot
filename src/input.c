#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <inttypes.h>
#include <libudev.h>

#include <libinput.h>

struct libinput_interface linterface;
struct libinput *lctx;
struct udev *uctx;

/* libinput_interface-hez callbackek */
static int open_callback(const char *path, int flags, void *user_data)
{
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
    if(!lctx)
        fprintf(stderr, "libinput_udev_create_context() nem sikerult\n");
    if(libinput_udev_assign_seat(lctx, "seat0") < 0)
        fprintf(stderr, "libinput_udev_assign_seat() hibaba utkozott\n");
}

void input_close(void)
{
    libinput_unref(lctx);
    udev_unref(uctx);
}

void input_receive_input(void)
{
    struct pollfd pfd = {
        .fd = libinput_get_fd(lctx),
        .events = POLLIN,
        .revents = 0,
    };

    poll(&pfd, 1, -1);
    if(libinput_dispatch(lctx) < 0)
        fprintf(stderr, "libinput_dispatch() failed\n");

    struct libinput_event *ev = libinput_get_event(lctx);
    if(libinput_event_get_type(ev) == LIBINPUT_EVENT_KEYBOARD_KEY) {
        struct libinput_event_keyboard *kbev = libinput_event_get_keyboard_event(ev);
        if(libinput_event_keyboard_get_key_state(kbev) == LIBINPUT_KEY_STATE_PRESSED) {
            printf("%u was pressed\n", libinput_event_keyboard_get_key(kbev));
        }
    }
    libinput_event_destroy(ev);
}
