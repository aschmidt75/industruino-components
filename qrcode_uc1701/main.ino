#include <Arduino.h>
#include <qrcode.h>

#include <UC1701.h>

// LCD
static UC1701 lcd;
const int PIN_LCD_BACKLIGHT = 26; 

// The structure to manage the QR code
QRCode qrcode;

// Allocate a chunk of memory to store the QR code
uint8_t qrcodeBytes[512];

void qrcode_lcd_small(QRCode& code, UC1701& lcd, int xo, int yo) {
  for (uint8_t y = 0; y < qrcode.size; y += 8) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
        uint8_t v = 0;
        for (uint8_t z = 0; z < 8; z++) {          
          v |= qrcode_getModule(&qrcode, x, y+z) << z;
        }

        if ( (xo+x <= 128) && (yo+y <= 64-8)) {
          lcd.setCursor(xo+x,yo+y/8);
          lcd.drawBitmap(&v, 8, 1);
        }
    }
  }

}

void qrcode_lcd_big(QRCode& qrcode, UC1701& lcd, int xo, int yo) {
  if ( qrcode.size > 3) {
    return;   // too beaucoup
  }
  for (uint8_t y = 0; y < qrcode.size; y += 8) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
        uint8_t v = 0;
        for (uint8_t z = 0; z < 4; z++) {          
          uint8_t v0 = qrcode_getModule(&qrcode, x, y+z);
          v |= v0 << 2*z;
          v |= v0 << 2*z+1;
        }

        lcd.setCursor(xo+2*x,yo+2*y/8);
        lcd.drawBitmap(&v, 8, 1);
        lcd.setCursor(xo+2*x+1,yo+2*y/8);
        lcd.drawBitmap(&v, 8, 1);

        v = 0;
        for (uint8_t z = 4; z < 8; z++) {          
          uint8_t v0 = qrcode_getModule(&qrcode, x, y+z);
          v |= v0 << 2*(z-4);
          v |= v0 << 2*(z-4)+1;
        }

        lcd.setCursor(xo+2*x,yo+1+2*y/8);
        lcd.drawBitmap(&v, 8, 1);
        lcd.setCursor(xo+2*x+1,yo+1+2*y/8);
        lcd.drawBitmap(&v, 8, 1);
    }
  }

}

void setup() {
  SerialUSB.begin(115200);
  delay(2000);

  lcd.begin();
  analogWrite(PIN_LCD_BACKLIGHT, 255);
  
  lcd.clear();
//  lcd.setCursor(0,0); 
//  lcd.print("Industruino PROTO"); 


  memset(qrcodeBytes, 0, sizeof(qrcodeBytes));
  qrcode_initText(&qrcode, qrcodeBytes, 11, ECC_HIGH, "https://thngstruction.online/07EABDAD-914D-4699-BFA7-136CD0D4F878/49D6D6BC-C502-40BC-B210-9FF3D3865378");
  qrcode_lcd_small(qrcode, lcd, 0, 0);

  int x = 75;
  lcd.setCursor(x,0);
  lcd.print("Please");
  lcd.setCursor(x,1);
  lcd.print("scan QR");
  lcd.setCursor(x,2);
  lcd.print("code to");
  lcd.setCursor(x,3);
  lcd.print("activate");
  lcd.setCursor(x,4);
  lcd.print("device.");
}


void loop() {
}
