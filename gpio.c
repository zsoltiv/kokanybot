#include <gpiod.h>

#define GPIO_CHIP_PATH "/dev/gpiochip0"
#define GPIO_CONSUMER "kokanybot"

struct gpiod_chip *chip;

int main(void)
{
    chip = gpiod_chip_open(GPIO_CHIP_PATH);

    struct gpiod_line_request_config line_cfg = {
        .consumer = "kokanybot",
        .flags = 0,
    };

    gpiod_chip_close(chip);
}
