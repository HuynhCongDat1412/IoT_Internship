# Hướng dẫn ESP32-S3-DevKitC-1

## 1. Giới thiệu

### Task
- Sử dụng RGB LED tích hợp trên kit.
- Sử dụng module OLED để hiển thị chữ.
- Sử dụng module SX1280 để giao tiếp
truyền nhận dữ liệu không dây bằng công nghệ LoRa.

### Công cụ PlatformIO
1. [Cách tải PlatformIO](https://www.youtube.com/watch?v=1hKQPLwo6-8&ab_channel=GreenTechnology)
2. [Cách tải thư viện có sẵn](https://www.youtube.com/watch?v=unft_WOXkiI&ab_channel=GreenTechnology)
3. Cách build, nạp code

## BÁO CÁO

### Nạp Code Cho Kit
Trước tiên, tìm hiểu về kit ESP32-S3-DevKitC-1. Tham khảo tài liệu chính thức và hướng dẫn chi tiết.

**Lưu Ý Khi Nạp Code:**
- Thực hiện thao tác nạp code mặc định của PlatformIO theo hướng dẫn.
- Để vào chế độ nạp code, giữ nút **Boot**, sau đó nhấn nút **Reset**. Có thể cần thử vài lần để quen với thao tác này.
- **Kết Quả:**
  - [ ] Nạp code thành công
  - [ ] Nạp code không thành công

### Sử Dụng RGB LED Tích Hợp
RGB LED trên ESP32-S3-DevKitC-1 được điều khiển qua **chân số 48**, theo tài liệu của kit.

**Chi Tiết Thực Hiện:**
- **File**: `codeled.cpp`
- **Thư Viện**: Sử dụng thư viện có sẵn để điều khiển LED.
- **Các Bước**:
  1. Khai báo chân cho RGB LED.
  2. Thay đổi giá trị R, G, B để thay đổi màu sắc của LED.

### Hiển Thị Chữ Trên Module OLED
Module OLED được giao tiếp bằng giao thức I2C để hiển thị chữ.

**Chi Tiết Thực Hiện:**
- **File**: `codeoled.cpp`
- **Thư Viện**: `SSD1306Wire.h`
- **Các Bước**:
  1. Khai báo các chân I2C cho module OLED.
  2. Sử dụng các hàm của thư viện `SSD1306Wire` để điều khiển hiển thị trên OLED.

**Mã Tham Khảo**:
```cpp
#include "SSD1306Wire.h"
```

### Giao Tiếp Truyền Nhận Dữ Liệu Không Dây Với Module SX1280 (LoRa)
Module SX1280 được sử dụng để truyền và nhận dữ liệu không dây bằng công nghệ LoRa.

#### Truyền Dữ Liệu
- **File**: `filetransmit.cpp`
- **Thư Viện**: `RadioLib`
- **Khai Báo Chân**: Khai báo các chân theo thứ tự: NSS PIN, DIO1 PIN, NRST PIN, BUSY PIN.
- **Kết Quả**: Gửi thành công một tin nhắn cơ bản.

#### Nhận Dữ Liệu
- **File**: `filereceive.cpp`
- **Thư Viện**: `Arduino.h`, `Wire.h`, `SSD1306Wire.h`, `RadioLib`
- **Khai Báo Chân**: Giống với phần truyền dữ liệu.
- **Kết Quả**: Nhận thành công một tin nhắn cơ bản.