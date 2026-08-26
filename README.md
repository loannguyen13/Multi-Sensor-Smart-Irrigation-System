# Multi-Sensor Smart Irrigation System
> **Hệ thống tưới tiêu thông minh đa cảm biến dựa trên giám sát môi trường và điều khiển bơm tự động.**
> Xây dựng trên nền tảng vi điều khiển STM32F103C8T6 (Bare-metal C) bởi **Nhóm 4BONGHOACODON** (Đồ án cuối khóa môn học Embedded Systems).

---

## Table of Contents
* [1. Overview](#1-overview)
* [2. System Architecture](#2-system-architecture)
* [3. Repository Layout](#3-repository-layout)
* [4. Control Logic](#4-control-logic--signal-processing)
* [5. Hardware Connection (Pinout)](#5-hardware-connection-pinout)
* [6. Requirements](#6-requirements)
    * [6.1 Hardware Components](#61-hardware-components)
    * [6.2 Software & Toolchain](#62-software--toolchain)
* [7. Getting Started](#7-getting-started)
* [8. Demo Output & Video](#8-demo-output--video)
* [9. Known Gaps & Roadmap](#9-known-gaps--roadmap)
* [10. Authors & Group Roles](#10-authors--group-roles)

---

## 1. Overview
**Multi-Sensor Smart Irrigation System** là một hệ thống IoT nông nghiệp thông minh tại chỗ (Edge Node), hoạt động hoàn toàn độc lập mà không cần kết nối Internet. Dự án giải quyết triệt để vấn đề lãng phí nước và bảo vệ cây trồng khỏi tình trạng ngập úng hoặc khô héo bằng cách tự động hóa hoàn toàn quy trình tưới tiêu dựa trên dữ liệu thời gian thực.

### Các tính năng cốt lõi:
* **Giám sát môi trường thời gian thực**: Đo độ ẩm đất, cường độ ánh sáng, nhiệt độ và độ ẩm không khí.
* **Điều khiển thông minh (FSM)**: Tự động đưa ra quyết định bật/tắt bơm dựa trên Bộ máy trạng thái hữu hạn, hỗ trợ 2 chế độ **AUTO** và **MANUAL**.
* **Thuật toán xử lý tín hiệu Edge**: Áp dụng bộ lọc trung bình động (Moving Average Filter) trên STM32 để khử nhiễu tín hiệu ADC của cảm biến độ ẩm đất.
* **Hiển thị & Giám sát tại chỗ**: Trình bày trực quan đồng thời 4 thông số môi trường trên màn hình LCD2004 qua giao tiếp I2C.

---

## 2. System Architecture

Hệ thống được thiết kế theo mô hình luồng dữ liệu một chiều khép kín, từ việc đọc cảm biến ngoại vi, xử lý tín hiệu số, chuyển đổi trạng thái điều khiển cho đến việc xuất tín hiệu kích hoạt cơ cấu chấp hành:

```
                      ┌────────────────────────────────────────┐
                      │        STM32F103C8T6 (Blue Pill)       │
                      │                                        │
    [Sensors Input]   │   [Data Processing]     [FSM Logic]    │         [Actuators]
    DHT11 (Air) ─────►│   Moving Average           IDLE        │
    Soil (Moisture)──►│   (10-sample filter)        │          │
    BH1750 (Lux) ────►│          │                  ▼          │ ──────► Relay 5V ──► Water Pump
                      │          ▼                CHECK        │
                      │   [User Interface]          │          │
    3x Push Buttons──►│   LCD2004 (I2C)             ▼          │ ──────► Alert LED & Buzzer
    UART (PC Debug)  ◄┼────────────────────────►WATERING◄──────┼────────► UART Log (115200 bps)
                      └────────────────────────────────────────┘
```

---

## 3. Repository Layout

Mã nguồn dự án được mô-đun hóa cao độ để các thành viên trong nhóm dễ dàng phối hợp phát triển song song thông qua Git:

| Đường dẫn (Path) | Vai trò trong hệ thống (Role) | Phần cứng liên quan | Trạng thái (Status) |
| :--- | :--- | :--- | :--- |
| [`Core/Src/main.c`](Core/Src/main.c) | Luồng chương trình chính, tích hợp FSM và cập nhật hiển thị LCD. | STM32F103C8T6 | **Hoàn thành** |
| [`Core/Src/ADC_Handler.c`](Core/Src/ADC_Handler.c) | Cấu hình bộ ADC1, viết thuật toán lọc nhiễu Moving Average và chuyển đổi sang %. | Cảm biến độ ẩm đất | **Hoàn thành** |
| [`Core/Src/I2C_Handler.c`](Core/Src/I2C_Handler.c) | Driver giao tiếp I2C cho màn hình LCD2004 và cảm biến BH1750. | LCD2004, BH1750 | **Hoàn thành** |
| [`Core/Src/DHT11_Handler.c`](Core/Src/DHT11_Handler.c) | Driver đọc dữ liệu nhiệt độ và độ ẩm không khí theo giao thức 1-wire. | DHT11 | **Hoàn thành** |
| [`Core/Src/GPIO.c`](Core/Src/GPIO.c) | Khởi tạo GPIO, định nghĩa các ngắt ngoài (EXTI) cho nút nhấn vật lý. | 3x Nút nhấn, Relay | **Hoàn thành** |
| [`Core/Src/Tim_Handler.c`](Core/Src/Tim_Handler.c) | Cấu hình Timer 4 tạo trễ micro giây và Timer 2 tạo ngắt lấy mẫu định kỳ. | Toàn hệ thống | **Hoàn thành** |
| [`build_and_flash.bat`](build_and_flash.bat) | File kịch bản tự động hóa biên dịch và nạp chương trình trực tiếp xuống mạch. | ST-Link v2 | **Hoàn thành** |

---

## 4. Control Logic 

### Finite State Machine (FSM)
Bộ điều khiển chính chạy một máy trạng thái hữu hạn (FSM) tuần hoàn với các bước xử lý phi trạng thái chặn (non-blocking):

* **Chế độ AUTO (Tự động)**:
    * `IDLE`: Chờ cờ ngắt từ Timer 2 (bật mỗi 3 giây).
    * `READ`: Kích hoạt ADC đọc độ ẩm đất, giao tiếp I2C đọc BH1750 và 1-wire đọc DHT11.
    * `CHECK`: So sánh giá trị độ ẩm đất trung bình với ngưỡng cài đặt (Mặc định: **Khô < 40%**, **Ướt > 80%**).
    * `WATERING`: Kích hoạt chân GPIO kích mức cao bật Relay mở máy bơm nếu đất khô.
* **Chế độ MANUAL (Thủ công)**:
    * Cho phép người dùng chuyển trạng thái On/Off của máy bơm ngay lập tức bằng nút nhấn vật lý hoặc gửi tín hiệu qua cổng UART mà không phụ thuộc vào giá trị cảm biến.

---

## 5. Hardware Connection (Pinout)

Sơ đồ đấu nối thực tế của các thiết bị ngoại vi với Kit STM32F103C8T6:

| Thiết bị ngoại vi | Chân STM32 | Chức năng kỹ thuật | Lưu ý phần cứng |
| :--- | :--- | :--- | :--- |
| **Soil Moisture Sensor** | `PB0` | ADC1 (Channel 8) | Đọc giá trị analog độ ẩm đất |
| **DHT11 Sensor** | `PA1` | GPIO Input/Output | Đọc nhiệt độ và độ ẩm không khí (1-wire) |
| **BH1750 Lux Sensor** | `PB6` (SCL), `PB7` (SDA) | I2C1 | Đọc cường độ ánh sáng (Dùng chung bus I2C) |
| **LCD2004 Display** | `PB6` (SCL), `PB7` (SDA) | I2C1 (Địa chỉ `0x27`) | Hiển thị thông số thời gian thực |
| **Relay 5V (Pump)** | `PA9` | GPIO Output (Active High) | Đóng ngắt nguồn máy bơm mini 5V |
| **LEDs Cảnh báo** | `PA1`, `PA2` | GPIO Output | Đèn chỉ báo cảnh báo ngưỡng độ ẩm |
| **Button 1 (Mode)** | `PA0` | EXTI0 (Interrupt Falling) | Chuyển đổi chế độ AUTO <-> MANUAL |
| **Button 2 (Manual Target)**| `PA4` | EXTI4 (Interrupt Falling) | Bật/tắt bơm thủ công ở chế độ MANUAL |
| **Button 3 (Manual Status)**| `PA8` | EXTI9_5 (Interrupt Falling) | Đọc nhanh giá trị cảm biến tức thời |

![Pinout Schematic](images/pinout.png)
---

## 6. Requirements

### 6.1 Hardware Components
* 1x Vi điều khiển **STM32F103C8T6 Blue Pill**.
* 1x Cảm biến nhiệt độ, độ ẩm không khí **DHT11**.
* 1x Cảm biến cường độ ánh sáng **BH1750 (GY-302)**.
* 1x Cảm biến độ ẩm đất analog.
* 1x Màn hình **LCD2004** kèm module chuyển đổi I2C PCF8574.
* 1x Module **Relay 5V** cách ly quang.
* 1x Máy bơm chìm mini 5V DC.
* 3x Nút nhấn hành trình, còi chíp Buzzer 12V, LED đơn cảnh báo.
* 1x Nguồn ngoài Adapter 5V/2A để cấp dòng tải cho bơm hoạt động ổn định.

### 6.2 Software & Toolchain
* Trình biên dịch: `arm-none-eabi-gcc` (GNU Arm Embedded Toolchain).
* Hệ thống sinh cấu hình build: **CMake (phiên bản >= 3.16) & Ninja**.
* Phần mềm nạp: **STM32_Programmer_CLI** (được cấu hình nạp tự động qua script `.bat`).
* IDE phát triển: STM32CubeIDE hoặc VS Code.

---

## 7. Getting Started

Dự án đã tích hợp sẵn hệ thống build CMake tự động hóa hoàn toàn. Bạn không cần nạp code thủ công bằng phần mềm GUI.

### Các bước nạp code xuống board:
1. Kết nối mạch STM32 với máy tính thông qua mạch nạp **ST-Link v2**.
2. Đảm bảo bạn đã cài đặt và cấu hình biến môi trường cho `arm-none-eabi-gcc` và `STM32_Programmer_CLI`.
3. Nhấp đúp chuột để chạy file [`build_and_flash.bat`](build_and_flash.bat) ở thư mục gốc của dự án. 
4. Script sẽ tự động dọn dẹp thư mục build cũ, biên dịch mã nguồn bằng Ninja và tiến hành flash trực tiếp xuống vi điều khiển qua cổng CLI.

```bash
# Lệnh thực thi biên dịch thủ công nếu dùng Linux/MacOS:
mkdir build && cd build
cmake -G Ninja ..
ninja
STM32_Programmer_CLI -c port=SWD -w Multi-Sensor-Smart-Irrigation-System.bin 0x08000000 -v -rst
```

---

## 8. Demo Output & Video

🎬 Xem video thực tế hoạt động thử nghiệm của dự án (kiểm thử sự thay đổi trạng thái tự động bật bơm khi đất khô và ngắt bơm khi đất ẩm):

[![Multi-Sensor Smart Irrigation System Demo](https://img.youtube.com/vi/Lm8nARhssnc/0.jpg)](https://youtu.be/Lm8nARhssnc?si=MTAc85anRcH3JVYW)

*Nhấp vào hình ảnh phía trên hoặc truy cập [tại đây](https://youtu.be/Lm8nARhssnc?si=MTAc85anRcH3JVYW) để xem video demo trực tiếp.*

---

## 9. Known Gaps & Roadmap

Do giới hạn về mặt thời gian trong khuôn khổ cuộc thi/môn học, dự án hiện tại vẫn còn một số điểm cần tiếp tục tối ưu hóa trong tương lai:
* 🌐 **Kết nối đám mây (Cloud Integration)**: Chuyển đổi mô hình hoạt động sang IoT thực thụ bằng cách sử dụng module ESP32 kết nối qua UART/SPI của STM32 để gửi dữ liệu lên Cloud Dashboard (Adafruit IO hoặc Blynk).

---

## 10. Authors & Group Roles

Dự án được thực hiện bởi tập thể **Nhóm 4BONGHOACODON**:

* 👩‍💼 **Nguyễn Kim Loan** 
* 👩‍💻 **Lê Trương Tuệ Linh** 
* 👨‍💻 **Trần Thanh Phúc** 
* 👨‍💻 **Phạm Anh Khôi**

---
