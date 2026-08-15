#ifndef LCD1602_H
#define LCD1602_H

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/atomic.h>

struct lcd1602_device {
    int major;

    atomic_t opened;

    struct class *class;
    struct device *device;

    struct i2c_adapter *adapter;
    struct i2c_client *client;
};

extern const struct file_operations lcd1602_fops;

struct lcd1602_device *lcd1602_get_device(void);

int lcd1602_i2c_init(struct lcd1602_device *lcd, int i2c_nr, int i2c_address);
void lcd1602_i2c_exit(struct lcd1602_device *lcd);

#endif // LCD1602_H
