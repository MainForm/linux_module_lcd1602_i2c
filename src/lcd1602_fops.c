#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/module.h>

// peripheral headers
#include <linux/i2c.h>

#include "lcd1602.h"

static int lcd1602_open(struct inode * inode, struct file * file){
    int ret = 0;

    pr_info("opened lcd1602 device\n");

    return ret;
}

static int lcd1602_release(struct inode * inode, struct file * file){
    pr_info("released lcd1602 device\n");

    return 0;
}

const struct file_operations lcd1602_fops = {
    .owner = THIS_MODULE,
    .open = lcd1602_open,
    .release = lcd1602_release,
};
