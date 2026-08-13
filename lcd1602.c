#include <linux/module.h>
#include <linux/init.h>

static int __init init_device(void){

    return 0;
}

static void __exit exit_device(void){

}

module_init(init_device);
module_exit(exit_device);

MODULE_LICENSE("MIT");
MODULE_DESCRIPTION("This module is for LCD 1602 display with I2C");
