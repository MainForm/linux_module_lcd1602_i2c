#define pr_fmt(fmt) "[" KBUILD_MODNAME "][%s]: " fmt, __func__

#include <linux/delay.h>

#include "lcd1602.h"

#define I2C_DEV_TYPE       	"lcd1602_i2c"

static const u8 LCD1602_ROW_OFFSET[2] = {
    0x00,
    0x40
};

int lcd1602_i2c_init(struct lcd1602_device *lcd, int i2c_nr, int i2c_address){
    int ret = 0;

    struct i2c_board_info board_info = {
		I2C_BOARD_INFO(I2C_DEV_TYPE, i2c_address)
	};

    pr_info("getting i2c(nr : %d) adapter\n", i2c_nr);
    lcd->adapter = i2c_get_adapter(i2c_nr);
	if (!lcd->adapter) {
		pr_err("I2C adapter(nr : %d)not found\n", i2c_nr);
		return -ENODEV;
	}

	pr_info("checking for plain I2C transfer support\n");
	if (!i2c_check_functionality(lcd->adapter, I2C_FUNC_I2C)) {
		pr_err("I2C adapter(nr : %d) does not support plain I2C transfers\n", i2c_nr);
		ret = -EOPNOTSUPP;

		goto err_i2c_put_adapter;
	}

    pr_info("successed to get i2c adapter\n");

    pr_info("getting i2c(address : 0x%02x) client\n",i2c_address);
	lcd->client = i2c_new_client_device(lcd->adapter, &board_info);
	if (IS_ERR(lcd->client)) {
		pr_err("failed to register I2C device: %d\n", ret);
		lcd->client = NULL;
		ret = PTR_ERR(lcd->client);

        goto err_i2c_put_adapter;
	}

    return 0;


err_i2c_put_adapter:
	if (lcd->adapter){
		i2c_put_adapter(lcd->adapter);
        lcd->adapter = NULL;
    }

	return ret;
}

void lcd1602_i2c_exit(struct lcd1602_device *lcd){
	if (lcd->client) {
		i2c_unregister_device(lcd->client);
        lcd->client = NULL;
	}

	if (lcd->adapter){
		i2c_put_adapter(lcd->adapter);
        lcd->adapter = NULL;
    }
}

int lcd1602_send_data(struct lcd1602_device *lcd, const u8 data, const u8 mode){
	u8 value = ((data & 0x0f) << 4) | mode;

	u8 buffers[2] = {
		value | LCD1602_MODE_EN,
		value & ~LCD1602_MODE_EN,
	};

	return i2c_master_send(lcd->client, buffers, sizeof(buffers) / sizeof(u8));
}

int lcd1602_send_command(struct lcd1602_device *lcd, const u8 command){
    int ret = 0;
    u8 mode = LCD1602_MODE_BL;

    ret = lcd1602_send_data(lcd, command >> 4, mode);
    if(ret < 0){
        return ret;
    }

    ret = lcd1602_send_data(lcd, command & 0x0f, mode);
    if(ret < 0){
        return ret;
    }

    if (command == 0x01 || command == 0x02 || command == 0x03)
        fsleep(2000);
    else
        fsleep(50);

    return 0;
}

int lcd1602_send_char(struct lcd1602_device *lcd, const char character)
{
    int ret = 0;
    u8 mode = LCD1602_MODE_BL | LCD1602_MODE_RS;

    ret = lcd1602_send_data(lcd, character >> 4, mode);
    if(ret < 0){
        return ret;
    }

    ret = lcd1602_send_data(lcd, character & 0x0f, mode);
    if(ret < 0){
        return ret;
    }

	fsleep(50);

    return 0;
}

int lcd1602_send_string(struct lcd1602_device *lcd, const char* str){
    int ret = 0;
    int char_count = 0;

    while(*str){
        ret = lcd1602_send_char(lcd,*str);
        if(ret < 0){
            return ret;
        }
        char_count++;
        str++;
    }

    return char_count;
}

void lcd1602_init_device(struct lcd1602_device *lcd){

    /* 전원 안정화 */
    fsleep(50000);

    /* 아직 8비트 초기 상태이므로 니블만 전송 */
    lcd1602_send_data(lcd, 0x03, LCD1602_MODE_BL);
    fsleep(5000);

    lcd1602_send_data(lcd, 0x03, LCD1602_MODE_BL);
    fsleep(200);

    lcd1602_send_data(lcd, 0x03, LCD1602_MODE_BL);
    fsleep(200);

    /* 4비트 모드로 전환 */
    lcd1602_send_data(lcd, 0x02, LCD1602_MODE_BL);
    fsleep(200);

    /* 4비트, 2줄, 5x8 폰트 */
    lcd1602_send_command(lcd, LCD1602_CMD_FUNCTION_SET | 0x08);

    /* Display off */
    lcd1602_send_command(lcd, LCD1602_CMD_DISPLAY_CONTROL);

    /* Clear display */
    lcd1602_send_command(lcd, LCD1602_CMD_CLEAR_DISPLAY);

    /* Entry mode: 커서 오른쪽 이동 */
    lcd1602_send_command(lcd, LCD1602_CMD_ENTRY_MODE_SET | 0x02);

    /* Display on, cursor off, blink off */
    lcd1602_send_command(lcd, LCD1602_CMD_DISPLAY_CONTROL | 0x04);
}

int lcd1602_set_cursor(struct lcd1602_device *lcd, const size_t col, const size_t row){
    if(col > 15 || row > 1){
        return -EFAULT;
    }

    return lcd1602_send_command(lcd,LCD1602_CMD_SET_DDRAM_ADDR | (LCD1602_ROW_OFFSET[row] + col));
}