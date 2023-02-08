#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#include "img.h"
#include "i2c.h"
#include "input.h"

int main(void)
{
    input_init();
    while(1) { // robot loop
        input_receive_input();
    }
}
