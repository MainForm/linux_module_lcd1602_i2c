# LCD1602 I2C Linux Kernel Module

Raspberry Pi에서 I2C 인터페이스가 연결된 LCD1602 문자 디스플레이를 제어하기 위해 작성 중인 Linux 커널 모듈입니다.

Linux 커널 모듈과 디바이스 드라이버의 구조를 학습하는 것을 목적으로 합니다. 현재는 모듈의 초기화 및 종료 함수만 구현되어 있으며, LCD 및 I2C 제어 기능은 아직 구현되지 않았습니다.

## 개발 환경

- Raspberry Pi 4
- Raspberry Pi OS
- LCD1602 문자 디스플레이
- 실행 중인 커널과 일치하는 Linux 커널 헤더
- GCC, GNU Make

다른 Raspberry Pi 모델에서는 아직 테스트하지 않았습니다.

## 하드웨어

- [LCD1602 문자 디스플레이](https://www.devicemart.co.kr/goods/view?no=1384575)
- I2C 인터페이스 모듈

> 연결 전 LCD I2C 모듈의 동작 전압과 Raspberry Pi GPIO의 허용 전압을 반드시 확인하세요.

## 프로젝트 구조

```text
.
├── lcd1602.c    # 커널 모듈 소스 코드
├── Kbuild       # 커널 빌드 대상 설정
├── Makefile     # 외부 커널 모듈 빌드 설정
└── build/       # 빌드 결과물
```

## 빌드

현재 실행 중인 커널의 빌드 디렉터리가 `/lib/modules/$(uname -r)/build`에 준비되어 있어야 합니다.

```bash
make
```

빌드에 성공하면 다음 모듈 파일이 생성됩니다.

```text
build/lcd1602.ko
```

## 모듈 로드

```bash
sudo insmod ./build/lcd1602.ko
```

로드 여부와 커널 메시지는 다음 명령으로 확인할 수 있습니다.

```bash
lsmod | grep lcd1602
sudo dmesg | tail
```

## 모듈 제거

```bash
sudo rmmod lcd1602
```