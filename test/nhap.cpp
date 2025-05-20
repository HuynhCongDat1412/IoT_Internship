    // // Nếu là số (counter), hiển thị ở dòng 2, còn lại là tin nhắn ở dòng 1
    // bool isNumber = true;
    // for (size_t i = 0; i < str.length(); i++) {
    //   if (!isDigit(str[i])) {
    //     isNumber = false;
    //     break;
    //   }
    // }
    // if (isNumber) {
    //   // Hiển thị tin nhắn trước đó ở dòng 1, counter ở dòng 2
    //   display.drawString(0, 20, lastMsg);
    //   display.drawString(0, 40, str);
    // } else {
    //   // Lưu lại tin nhắn, hiển thị ở dòng 1
    //   lastMsg = str;
    //   display.drawString(0, 20, str);
    //   // Gửi ACK lại cho transmitter
    //   int ackState = radio.transmit("ACK");
    //   if (ackState == RADIOLIB_ERR_NONE) {
    //     Serial.println("[RX] Sent ACK!");
    //   } else {
    //     Serial.print("[RX] Failed to send ACK, code ");
    //     Serial.println(ackState);
    //   }
    // }