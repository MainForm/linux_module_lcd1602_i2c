#ifndef LCD1602_H
#define LCD1602_H

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/atomic.h>
#include <linux/types.h>

#define LCD1602_MODE_RS		(0x01 << 0)
#define LCD1602_MODE_RW		(0x01 << 1)
#define LCD1602_MODE_EN		(0x01 << 2)
#define LCD1602_MODE_BL		(0x01 << 3)		// back lgith

#define LCD1602_CMD_CLEAR_DISPLAY       (0x01 << 0)
#define LCD1602_CMD_RETURN_HOME         (0x01 << 1)
#define LCD1602_CMD_ENTRY_MODE_SET      (0x01 << 2)
#define LCD1602_CMD_DISPLAY_CONTROL     (0x01 << 3)
#define LCD1602_CMD_CURSOR_SHIFT        (0x01 << 4)
#define LCD1602_CMD_FUNCTION_SET        (0x01 << 5)
#define LCD1602_CMD_SET_CGRAM_ADDR      (0x01 << 6)
#define LCD1602_CMD_SET_DDRAM_ADDR      (0x01 << 7)

#define LCD1602_CHAR_MAX        32
#define LCD1602_WIDTH_SIZE      16
#define LCD1602_HEIGHT_SIZE     2

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

int lcd1602_set_cursor(struct lcd1602_device *lcd, const size_t col, const size_t row);

#endif // LCD1602_H
