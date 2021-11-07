#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

// time between switches on/off in millisecond
#define DELAY 1000

// get a handle on the device-node with the name led0 in the devicetree
// more information on how to access the device tree from C is available at
// https://docs.zephyrproject.org/latest/guides/dts/api-usage.html
#define LED_RED_NODE DT_NODELABEL(led0)
#define LED_GREEN_NODE DT_NODELABEL(led1)
#define LED_BLUE_NODE DT_NODELABEL(led2)

// check if the device is set up properly (led0, the red one)
#if DT_NODE_HAS_STATUS(LED_RED_NODE, okay)
    // get the label in the device tree of the device node
    #define LED0    DT_GPIO_LABEL(LED_RED_NODE, gpios)
    // get the PIN-property of the device node
    #define PIN0    DT_GPIO_PIN(LED_RED_NODE, gpios)
    // get the FLAGS-property of the gpio device node with index 0
    #define FLAGS0    DT_GPIO_FLAGS(LED_RED_NODE, gpios)
#else
    // in case the node is not "okay", i.e., the board does not support led
    #error "The alias in the devicetree is not defined"
    #define LED0    ""
    #define PIN0    0
    #define FLAGS0  0
#endif

// check if the device is set up properly (led1, the green one)
#if DT_NODE_HAS_STATUS(LED_GREEN_NODE, okay)
    // get the label in the device tree of the device node
    #define LED1    DT_GPIO_LABEL(LED_GREEN_NODE, gpios)
    // get the PIN-property of the device node
    #define PIN1    DT_GPIO_PIN(LED_GREEN_NODE, gpios)
    // you can also explicitly provide the index (here 0)
    #define FLAGS1 DT_GPIO_FLAGS_BY_IDX(LED_GREEN_NODE, gpios, 0)
#else
    // in case the node is not "okay", i.e., the board does not support led
    #error "The node in the devicetree is not defined"
    #define LED1    ""
    #define PIN1    0
    #define FLAGS1  0
#endif


// check if the device is set up properly (led2, the blue one)
#if DT_NODE_HAS_STATUS(LED_BLUE_NODE, okay)
    // get the label in the device tree of the device node
    #define LED2    DT_GPIO_LABEL(LED_BLUE_NODE, gpios)
    // get the PIN-property of the device node
    #define PIN2    DT_GPIO_PIN(LED_BLUE_NODE, gpios)
    // you can also explicitly provide the index (here 0)
    #define FLAGS2 DT_GPIO_FLAGS_BY_IDX(LED_BLUE_NODE, gpios, 0)
#else
    // in case the node is not "okay", i.e., the board does not support led
    #error "The node in the devicetree is not defined"
    #define LED2    ""
    #define PIN2    0
    #define FLAGS2  0
#endif

void main(void)
{
    // store potential error values
    int ret;

    // get binding for the led device based on the device tree label
    const struct device *red_led_device = device_get_binding(LED0);
    const struct device *green_led_device = device_get_binding(LED1);
    const struct device *blue_led_device = device_get_binding(LED2);
    if (red_led_device == NULL || green_led_device == NULL || blue_led_device == NULL) {
          // something went wrong with getting the device struct from the
          // device tree
        return;
    }

    // configure the led_device at pin PIN as output and initialize it to a
    // logical 1
    // all gpio-flags are described at https://docs.zephyrproject.org/latest/reference/peripherals/gpio.html
    ret = gpio_pin_configure(red_led_device, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS0);
    if (ret < 0) {
        return;
    }
    // also configure the green led device at PIN1 for output and
    // initialize it to a logical 0
    ret = gpio_pin_configure(green_led_device, PIN1, GPIO_OUTPUT_INACTIVE | FLAGS1);
    if (ret < 0) {
        return;
    }
    // also configure the blue led device at PIN2 for output and
    // initialize it to a logical 0
    ret = gpio_pin_configure(blue_led_device, PIN2, GPIO_OUTPUT_INACTIVE | FLAGS2);
    if (ret < 0) {
        return;
    }

    // make them blink periodically
    while (1) {
        // iterate through rgb colours
        for(uint8_t i=0; i<8; i++)
        { 
            gpio_pin_set(red_led_device, PIN0, i&1);
            gpio_pin_set(green_led_device, PIN1, (i>>1)&1);
            gpio_pin_set(blue_led_device, PIN2, (i>>2)&1);
            k_msleep(DELAY);
        }
    }
}
