#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <poll.h>
#include <inttypes.h>
#include <libudev.h>
#include <linux/input-event-codes.h>

#include <libinput.h>

#include "input.h"

static struct libinput_interface linterface;
static struct libinput *lctx;
static struct udev *uctx;

static int get_key_bind_index(uint32_t key)
{
    for(int i = 0; i < sizeof(key_binds) / sizeof(key_binds[0]); i++) {
        if(key_binds[i].key == key)
            return i;
    }

    return -1;
}

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

void dispatch_pending_events(void)
{
    libinput_dispatch(lctx);
    struct libinput_event *ev = NULL;
    while((ev = libinput_get_event(lctx)))
        libinput_event_destroy(ev);
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

    for(int i = 0; i < sizeof(key_binds) / sizeof(key_binds[0]); i++)
        key_binds[i].prev_state = LIBINPUT_KEY_STATE_RELEASED;

    dispatch_pending_events(); // dispatch LIBINPUT_EVENT_DEVICE_ADDED events
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
    if(libinput_dispatch(lctx) < 0) {
        fprintf(stderr, "libinput_dispatch() failed\n");
        return;
    }

    struct libinput_event *ev = NULL;
    while((ev = libinput_get_event(lctx))) {
        if(libinput_event_get_type(ev) == LIBINPUT_EVENT_KEYBOARD_KEY) {
            struct libinput_event_keyboard *kbev = libinput_event_get_keyboard_event(ev);
            uint32_t keycode = libinput_event_keyboard_get_key(kbev);
            int state_idx = get_key_bind_index(keycode);
            if(state_idx > -1) {
                print_event_header(ev);
                enum libinput_key_state key_state = libinput_event_keyboard_get_key_state(kbev);
                if(key_state != key_binds[state_idx].prev_state) {
                    key_binds[state_idx].func(key_state == LIBINPUT_KEY_STATE_PRESSED ? true : false);
                    key_binds[state_idx].prev_state = key_state;
                }
            } else {
                fprintf(stderr, "rossz gomb\n");
            }
        }
        libinput_event_destroy(ev);
        libinput_dispatch(lctx);
    }
}
