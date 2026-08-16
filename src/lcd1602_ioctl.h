#ifndef LCD1602_IOCTL_H
#define LCD1602_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

struct lcd1602_cursor {
    __u8 row;
    __u8 column;
};

#define LCD1602_IOCTL_MAGIC 'L'
 
// ioctl 명령어 정의
#define LCD1602_IOCTL_CLEAR     _IO     (LCD1602_IOCTL_MAGIC, 0)            // 인자을 사용하지 않는 명령어
#define LCD1602_IOCTL_SET_CUR   _IOW    (LCD1602_IOCTL_MAGIC, 1, struct lcd1602_cursor)    // 인자를 사용하는 쓰기 명령어

#endif // LCD1602_IOCTL_H