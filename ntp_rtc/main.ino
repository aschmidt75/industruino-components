// Industruino Proto/IND.IO + Ethernet module
// 
// connects to Ethernet using DHCP, queries current time from ntp server
// if successful, stores time in RTC
// displays time from RTC, even if ethernet is not connected at boot-up.
//
// Time library: https://github.com/PaulStoffregen/Time
// Industruino RTC: https://github.com/Industruino/MCP7940-RTC-Library

#include <UC1701.h>

#include <SPI.h>
#include <EthernetUdp2.h>
#include <EthernetClient.h>
#include <Dhcp.h>
#include <EthernetServer.h>
#include <Ethernet2.h>
#include <util.h>
#include <Dns.h>

// Display

static UC1701 lcd;

const int PIN_LCD_BACKLIGHT = 26; 

static const byte glyph[] = { B11111111, B11111111, B11111111, B11111111, B11111111 };

// Ethernet

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

int NETWORK_INFO_DELAY_SECS = 4;

bool ETHIsConnected = false;


// NTP
#include <TimeLib.h>

unsigned int localPort = 8888;       
char timeServer[] = "pool.ntp.org"; 
EthernetUDP Udp;

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

const int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

time_t getNtpTime(UDP &udp)
{
  while (udp.parsePacket() > 0) ; // discard any previously received packets
  if (SerialUSB) {
    SerialUSB.println("Transmit NTP Request");
  }
  sendNTPpacket(udp, timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      if (SerialUSB) {
        SerialUSB.println("Receive NTP Response");
      }
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  if (SerialUSB) {
    SerialUSB.println("No NTP Response.");
  }
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(UDP &udp, const char *address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


// RTC
#include <Wire.h>
#include <MCP7940.h>

int rtc[7];

void time_to_rtc(time_t now) {
  // write to rtc
  RTCind.get(rtc,true); 
  RTCind.set(MCP7940_SEC,second(now));
  RTCind.set(MCP7940_MIN,minute(now));
  RTCind.set(MCP7940_HR,hour(now));
  //RTCind.set(MCP7940_DOW,1);
  RTCind.set(MCP7940_DATE,day(now));
  RTCind.set(MCP7940_MTH,month(now));
  RTCind.set(MCP7940_YR,year(now)-2000);
}

tmElements_t time_from_rtc() {
  RTCind.get(rtc,true);
  tmElements_t res;
  res.Second = rtc[0];
  res.Minute = rtc[1];
  res.Hour = rtc[2];
  res.Day = rtc[4];
  res.Month = rtc[5];
  res.Year = rtc[6];
  return res;
}

void setup() {
  SerialUSB.begin(115200);

  lcd.begin();
  analogWrite(PIN_LCD_BACKLIGHT, 64);
  lcd.createChar(0, glyph);
  
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Connecting...");
  
  int r = Ethernet.begin(mac);
  if ( r == 0) {
    ETHIsConnected = false;
    
    lcd.setCursor(0,0); 
    lcd.print("DHCP error, no IP.");  

    lcd.setCursor(0,1); lcd.print("MAC"); 
    for ( int i = 0; i < 6; i++) {
      lcd.setCursor(25+(i*15),1); lcd.print(mac[i],HEX);
      lcd.setCursor(25+(i*15)+10,1); lcd.print(":");
    }
    lcd.setCursor(110,1); lcd.print(" ");
  } else {
    ETHIsConnected = true;
    
    lcd.setCursor(0,0); 
    lcd.clearLine();

    lcd.setCursor(0,0); lcd.print("MAC"); 
    for ( int i = 0; i < 6; i++) {
      lcd.setCursor(25+(i*15),0); lcd.print(mac[i],HEX);
      lcd.setCursor(25+(i*15)+10,0); lcd.print(":");
    }
    lcd.setCursor(110,0); lcd.print(" ");
    
    lcd.setCursor(0,1); lcd.print("IP "); 
    lcd.setCursor(25,1); 
    lcd.print(Ethernet.localIP());

    lcd.setCursor(0,2); lcd.print("SUB"); 
    lcd.setCursor(25,2);
    lcd.print(Ethernet.subnetMask());
  
    lcd.setCursor(0,3); lcd.print("GW"); 
    lcd.setCursor(25,3);
    lcd.print(Ethernet.gatewayIP());
  
    lcd.setCursor(0,4); lcd.print("DNS"); 
    lcd.setCursor(25,4);
    lcd.print(Ethernet.dnsServerIP());
  }  

  // show a simple progress bar
  for ( int i = 0; i < 25; i++) {
    lcd.setCursor(i*5, 7);
    lcd.write(0);
    delay((NETWORK_INFO_DELAY_SECS*1000)/25);
  }

  lcd.clear();

  // use display otherwise

  // NTP
  Wire.begin();
  
  if (ETHIsConnected) {
    SerialUSB.println("fetching time...");
    
    Udp.begin(localPort);
  
    time_t now;
    now = getNtpTime(Udp);
    if ( now != 0) {
      time_to_rtc(now);

      // print to serial
      if ( SerialUSB) {
        char buf[64];
        sprintf(buf,"%0i.%0i.%i, %0i:%0i:%i", day(now), month(now), year(now), hour(now), minute(now), second(now));
        SerialUSB.print("received time: "); 
        SerialUSB.println(buf);
      }  
    }
  }

  // start RTC
  RTCind.start(true);    
}


void loop() {
  // get time from rtc.
  tmElements_t now;
  now = time_from_rtc();

  char buf[64];
  sprintf(buf,"%0i.%0i.%i, %0i:%0i:%i", now.Day, now.Month, now.Year, now.Hour, now.Minute, now.Second);

  lcd.setCursor(0,0);
  lcd.print(buf);

  delay(1000);
}





