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

#include "cmd.h"
#include "input.h"

struct libinput_interface linterface;
struct libinput *lctx;
struct udev *uctx;

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
    int fd = libinput_get_fd(lctx);
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
        .revents = 0,
    };

    poll(&pfd, 1, -1);
    if(libinput_dispatch(lctx) < 0) {
        fprintf(stderr, "libinput_dispatch() failed\n");
        return;
    }

    struct libinput_event *ev = libinput_get_event(lctx);
    if(!ev) {
        fprintf(stderr, "ev is NULL\n");
        goto destroy;
    }
    printf("van event\n");
    if(libinput_event_get_type(ev) == LIBINPUT_EVENT_KEYBOARD_KEY) {
        struct libinput_event_keyboard *kbev = libinput_event_get_keyboard_event(ev);
        uint32_t keycode = libinput_event_keyboard_get_key(kbev);
        int state_idx = get_key_bind_index(keycode);
        if(state_idx > -1) {
            enum libinput_key_state lkey_bind = libinput_event_keyboard_get_key_state(kbev);
            key_binds[state_idx].func(lkey_bind == LIBINPUT_KEY_STATE_PRESSED ? true : false);
        } else {
            fprintf(stderr, "rossz gomb\n");
        }
    }
destroy:
    libinput_event_destroy(ev);
}
