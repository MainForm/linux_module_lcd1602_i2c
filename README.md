# LCD1602 I2C Linux Kernel Module

Raspberry Pi에서 I2C 인터페이스(PCF8574 계열 백팩)가 연결된 LCD1602 문자 디스플레이를 제어하는 Linux 커널 모듈입니다.

모듈을 로드하면 `/dev/lcd1602` 문자 장치가 생성됩니다. 사용자 프로그램은 일반적인 `open(2)`, `write(2)`, `ioctl(2)`, `close(2)` 시스템 호출로 LCD에 문자열을 출력하거나 화면과 커서 위치를 제어할 수 있습니다.

## 주요 기능

- LCD1602 4-bit 모드 초기화
- `/dev/lcd1602`에 쓴 문자열 출력
- 16열 x 2행, 최대 32문자 지원
- 줄 끝에 도달하면 다음 행으로 자동 이동
- `ioctl`을 이용한 화면 지우기 및 커서 위치 변경
- 동시에 하나의 프로세스만 장치를 열 수 있도록 제한

## 개발 환경 및 하드웨어

- Raspberry Pi 4
- Raspberry Pi OS
- LCD1602 문자 디스플레이
- I2C 인터페이스 모듈
- 실행 중인 커널과 일치하는 Linux 커널 헤더
- GCC, GNU Make

다른 Raspberry Pi 모델에서는 아직 테스트하지 않았습니다.

> LCD I2C 모듈을 연결하기 전에 모듈의 동작 전압과 Raspberry Pi GPIO의 허용 전압을 확인하세요.

## I2C 설정

드라이버의 기본 I2C 설정은 다음과 같습니다.

| 항목 | 기본값 |
| --- | --- |
| I2C 버스 | `1` (`/dev/i2c-1`) |
| 7-bit I2C 주소 | `0x27` |

이 값은 `src/lcd1602_main.c`에 정의되어 있습니다.

```c
#define LCD1602_I2C_NR_DEFAULT      1
#define LCD1602_I2C_ADDRESS_DEFAULT 0x27
```

연결된 장치의 주소는 다음 명령으로 확인할 수 있습니다.

```bash
sudo i2cdetect -y 1
```

LCD 백팩의 주소가 `0x27`이 아니면 `LCD1602_I2C_ADDRESS_DEFAULT`를 실제 주소로 변경한 뒤 모듈을 다시 빌드하세요. 다른 I2C 버스를 사용한다면 `LCD1602_I2C_NR_DEFAULT`도 함께 변경해야 합니다.

## 프로젝트 구조

```text
.
├── app/
│   ├── app.c               # 사용자 공간 테스트 프로그램
│   └── Makefile
├── src/
│   ├── Kbuild
│   ├── lcd1602.h
│   ├── lcd1602_ioctl.h     # 사용자 공간과 공유하는 ioctl 인터페이스
│   ├── lcd1602_main.c      # 모듈 및 문자 장치 등록
│   ├── lcd1602_fops.c      # open/write/ioctl/release 구현
│   └── lcd1602_i2c.c       # LCD 및 I2C 제어
├── Makefile
└── build/                  # 커널 모듈 빌드 결과물
```

## 커널 모듈 빌드 및 로드

현재 실행 중인 커널의 빌드 디렉터리가 `/lib/modules/$(uname -r)/build`에 준비되어 있어야 합니다.

```bash
make
sudo insmod ./build/lcd1602.ko
```

모듈이 정상적으로 로드되면 `/dev/lcd1602`가 생성됩니다.

```bash
ls -l /dev/lcd1602
lsmod | grep lcd1602
sudo dmesg | tail
```

권한 오류가 발생하면 테스트 목적으로 `sudo`를 사용하거나 장치 파일의 소유권 및 권한을 적절히 설정하세요.

## 문자열 출력

셸에서 장치 파일에 직접 문자열을 쓸 수 있습니다.

```bash
printf 'Hello, LCD1602!' | sudo tee /dev/lcd1602 > /dev/null
```

C 프로그램에서는 다음과 같이 사용합니다.

```c
#include <fcntl.h>
#include <unistd.h>

int fd = open("/dev/lcd1602", O_RDWR);

if (fd >= 0) {
    const char message[] = "Hello, LCD!";
    write(fd, message, sizeof(message) - 1);
    close(fd);
}
```

한 번 연 장치에서는 최대 32문자까지 쓸 수 있으며, 첫 번째 행의 16문자를 넘으면 두 번째 행으로 이동합니다. 장치가 이미 열려 있으면 추가 `open()` 호출은 `EBUSY`로 실패합니다.

## ioctl 사용법

사용자 프로그램은 `src/lcd1602_ioctl.h`를 include해야 합니다.

```c
#include <sys/ioctl.h>
#include "lcd1602_ioctl.h"
```

### 화면 지우기

`LCD1602_IOCTL_CLEAR`는 별도의 인자를 받지 않습니다.

```c
if (ioctl(fd, LCD1602_IOCTL_CLEAR) == -1)
    perror("LCD1602_IOCTL_CLEAR");
```

### 커서 위치 변경

`LCD1602_IOCTL_SET_CUR`에는 `struct lcd1602_cursor`의 주소를 전달합니다. 행과 열 번호는 모두 0부터 시작합니다.

- `row`: `0` 또는 `1`
- `column`: `0`부터 `15`

```c
struct lcd1602_cursor cursor = {
    .row = 1,
    .column = 0,
};

if (ioctl(fd, LCD1602_IOCTL_SET_CUR, &cursor) == -1)
    perror("LCD1602_IOCTL_SET_CUR");
```

전체 예시는 다음과 같습니다.

```c
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "lcd1602_ioctl.h"

int main(void)
{
    int fd = open("/dev/lcd1602", O_RDWR);
    struct lcd1602_cursor cursor = {
        .row = 1,
        .column = 0,
    };

    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (ioctl(fd, LCD1602_IOCTL_SET_CUR, &cursor) == -1)
        perror("LCD1602_IOCTL_SET_CUR");

    if (write(fd, "Hello, LCD!", 11) == -1)
        perror("write");

    if (ioctl(fd, LCD1602_IOCTL_CLEAR) == -1)
        perror("LCD1602_IOCTL_CLEAR");

    close(fd);
    return 0;
}
```

> 현재 `SET_CUR`은 LCD 하드웨어의 커서만 옮기며, 열린 파일의 내부 offset은 변경하지 않습니다. 따라서 이후 `write()`의 자동 줄바꿈 및 32문자 제한은 해당 파일에서 앞서 쓴 누적 바이트 수를 기준으로 처리됩니다.

## 테스트 프로그램 빌드

`app/Makefile`에는 `../src`가 include 경로로 설정되어 있어 `lcd1602_ioctl.h`를 바로 사용할 수 있습니다.

```bash
make -C app
sudo ./app/lcd1602_test
```

## 모듈 제거 및 정리

```bash
sudo rmmod lcd1602
make clean
make -C app clean
```
