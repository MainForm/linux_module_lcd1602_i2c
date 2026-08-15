#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/module.h>

// peripheral headers
#include <linux/i2c.h>

#include "lcd1602.h"

static int lcd1602_open(struct inode * inode, struct file * file){
    int ret = 0;

    struct lcd1602_device * lcd_dev = lcd1602_get_device();

    if(atomic_cmpxchg(&lcd_dev->opened, 0, 1) != 0){
        pr_warn("lcd1602 device is already open\n");

        return -EBUSY;
    }

    file->private_data = (void *)lcd_dev;

    pr_info("opened lcd1602 device\n");

    lcd1602_init_device(lcd_dev);

    pr_info("initialized lcd1602 device\n");

    // 테스트 용으로 "Hello World" 출력
    lcd1602_send_string(lcd_dev,"Hello World");

    return ret;
}

static int lcd1602_release(struct inode * inode, struct file * file){
    struct lcd1602_device *lcd = file->private_data;

    atomic_set(&lcd->opened, 0);
    file->private_data = NULL;

    pr_info("released lcd1602 device\n");

    return 0;
}

const struct file_operations lcd1602_fops = {
    .owner = THIS_MODULE,
    .open = lcd1602_open,
    .release = lcd1602_release,
};
