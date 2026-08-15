#ifndef LCD1602_H
#define LCD1602_H

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/atomic.h>
#include <linux/types.h>

struct lcd1602_device {
    int major;

    atomic_t opened;

    struct class *class;
    struct device *device;

    struct i2c_adapter *adapter;
    struct i2c_client *client;
};

extern const struct file_operations lcd1602_fops;

// lcd1602_main.c에 정의 되어 있음
struct lcd1602_device *lcd1602_get_device(void);

// lcd1602_i2c.c에 정의 되어 있음
int lcd1602_i2c_init(struct lcd1602_device *lcd, int i2c_nr, int i2c_address);
void lcd1602_i2c_exit(struct lcd1602_device *lcd);

void lcd1602_send_data(struct lcd1602_device *lcd, const u8 data, const u8 mode);
void lcd1602_send_command(struct lcd1602_device *lcd, const u8 command);
void lcd1602_send_char(struct lcd1602_device *lcd, const char character);
void lcd1602_send_string(struct lcd1602_device *lcd, const char* str);

void lcd1602_init_device(struct lcd1602_device *lcd);

#endif // LCD1602_H
