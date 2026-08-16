#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/module.h>

// peripheral headers
#include <linux/i2c.h>

#include "lcd1602.h"
#include "lcd1602_ioctl.h"

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

    return ret;
}

static int lcd1602_release(struct inode * inode, struct file * file){
    struct lcd1602_device *lcd = file->private_data;

    atomic_set(&lcd->opened, 0);
    file->private_data = NULL;

    pr_info("released lcd1602 device\n");

    return 0;
}

static ssize_t lcd1602_write(struct file * file, const char __user * buffer, size_t count, loff_t * offset){
    struct lcd1602_device *lcd = file->private_data;

    size_t cur_x_pos = *offset % LCD1602_WIDTH_SIZE;
    size_t cur_y_pos = *offset / LCD1602_WIDTH_SIZE;

    char lcd_buffer[LCD1602_CHAR_MAX] = {0, };

    if (*offset < 0)
        return -EINVAL;

    if (*offset >= LCD1602_CHAR_MAX)
        return -ENOSPC;

    count = min(LCD1602_CHAR_MAX - (size_t)*offset, count);

    pr_info("writing data to lcd\n");


    if(copy_from_user(lcd_buffer,buffer,count)){
        pr_err("copy_from_user() failed\n");
        return -EFAULT;
    }

    for(size_t i = 0;i < count;i++){
        lcd1602_send_char(lcd, lcd_buffer[i]);

        ++cur_x_pos;
        if(cur_x_pos > 15){
            ++cur_y_pos;
            lcd1602_set_cursor(lcd,0, cur_y_pos);
            cur_x_pos = 0;
        }
    }

    *offset += count;

    return count;
}

static long lcd1602_ioctl(struct file * file, unsigned int command, unsigned long arguments){
    int ret = 0;
    struct lcd1602_device *lcd = file->private_data;
    struct lcd1602_cursor target_cursor;

    switch(command){
        case LCD1602_IOCTL_CLEAR:
            ret = lcd1602_send_command(lcd, LCD1602_CMD_CLEAR_DISPLAY);

            if(ret < 0){
                return ret;
            }

            return 0;
        case LCD1602_IOCTL_SET_CUR:
            if(copy_from_user(&target_cursor,(void *)arguments,sizeof(struct lcd1602_cursor))){
                pr_err("copy_from_user() failed\n");
                return -EFAULT;
            }

            ret = lcd1602_set_cursor(lcd,target_cursor.column,target_cursor.row);
            if(ret < 0){
                return ret;
            }

            return 0;
        default:
            return -EINVAL;
    }

    return -EINVAL;
}

const struct file_operations lcd1602_fops = {
    .owner = THIS_MODULE,
    .open = lcd1602_open,
    .release = lcd1602_release,
    .write = lcd1602_write,
    .unlocked_ioctl = lcd1602_ioctl,
};
