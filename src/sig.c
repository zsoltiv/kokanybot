#include <signal.h>

#include "gpio.h"
#include "sig.h"

void sig_handler(int signum)
{
    switch(signum) {
        case SIGINT:
        case SIGHUP:
            gpio_cleanup();
            break;
        default:
            break;
    }
}
