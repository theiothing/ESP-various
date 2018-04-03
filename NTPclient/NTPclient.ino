/*

  Udp NTP Client

  Get the time from a Network Time Protocol (NTP) time server
  Demonstrates use of UDP sendPacket and ReceivePacket
  For more on NTP time servers and the messages needed to communicate with them,
  see http://en.wikipedia.org/wiki/Network_Time_Protocol

  created 4 Sep 2010
  by Michael Margolis
  modified 9 Apr 2012
  by Tom Igoe
  updated for the ESP8266 12 Apr 2015
  by Ivan Grokhotkov
  updated with Timezone lib and easier function Mar 2018
  by Marco P. - www.theiothing.com - git: https://github.com/theiothing/ESP-various
  This code is in the public domain.
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Timezone.h>

char ssid[] = "YOUR WIFI";  //  your network SSID (name)
char pass[] = "YOUR PASSWORD";       // your network password

uint8_t t_sec;
uint8_t t_min;
uint8_t t_hour;


uint32_t lastNtpRequest = 0;
uint32_t lastSecondsUpdate = 0;
uint32_t lastSerialPrint = 0;
uint32_t ntpRequestInterval = 1*60*60*1000; //1h*60min*60s*1000ms

//uint8_t t_weekday;  // 1 -> sunday - no 0
//uint8_t t_day;
//uint8_t t_month;

/************* NTP SETUP *************/
IPAddress timeServerIP;                                      // ip address of the POOL assigned by WiFi.hostByName function
const char* ntpServerName = "0.it.pool.ntp.org";            // italy pool
const int NTP_PACKET_SIZE = 48;                             // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];                        // buffer to hold incoming and outgoing packets
unsigned int localPort = 2390;                              // local port to listen for UDP packets
WiFiUDP udp;                                                // A UDP instance to let us send and receive packets over UDP
bool waitingUdpAnswer = false;

/*************setup timezones and time stuff *************/

// Central European Time (Frankfurt, Paris, Rome)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);                                     // bond summer and standard time

// UTC  //utc in Timezone format - no changes
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule, utcRule);

time_t utc;                                                 //type time_t is waaaay more manageble
time_t centrEUtime;


void startUDP() {
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}


unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;                 // LI, Version, Mode
  packetBuffer[1] = 0;                          // Stratum, or type of clock
  packetBuffer[2] = 6;                          // Polling Interval
  packetBuffer[3] = 0xEC;                       // Peer Clock Precision
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
void sendNtpRequest() {
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);             //bond the dns name and the Ip adrress of the pool
  sendNTPpacket(timeServerIP);                              // send an NTP packet to a time server
  // wait to see if a reply is available
  waitingUdpAnswer = true;
}

void handleIncomingNtpPacket() {
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    waitingUdpAnswer = false;
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE);                // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);  //the timestamp starts at byte 40 of the received packet and is four bytes, or two words, long. First, esxtract the two words
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;             // combine the four bytes (two words) into a long integer this is NTP time (seconds since Jan 1 1900):
    Serial.print("Unix time = ");                                       // now convert NTP time into everyday time:
    const unsigned long seventyYears = 2208988800UL;                    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    unsigned long epoch = secsSince1900 - seventyYears;                 // subtract seventy years:
    Serial.println(epoch);
    utc = epoch;
    Serial.printf("UTC TIME: %i:%i:%i\n", hour(utc), minute(utc), second(utc));
    centrEUtime = CE.toLocal(utc);
    Serial.printf("___________________________\nCentral Europe Time: %i:%i:%i\n", hour(centrEUtime), minute(centrEUtime), second(centrEUtime));
    t_sec = second(centrEUtime);
    t_min = minute(centrEUtime);
    t_hour = hour(centrEUtime);
    lastSecondsUpdate = millis();

  }
}
void updateNTP(uint32_t interval) {
  if ((uint32_t)(millis() - lastNtpRequest) >= interval) {
    sendNtpRequest();
    lastNtpRequest = millis();
    Serial.println("sending request");
  }
  if (waitingUdpAnswer) handleIncomingNtpPacket();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  startUDP();
  sendNtpRequest();
  delay(1000);
}

void updateClock() {
  if ((uint32_t)(millis() - lastSecondsUpdate) >= 1000) {
    t_sec++;
    if (t_sec == 60){
      t_sec = 0;
      t_min++;
      if (t_min == 60){
        t_min = 0;
        t_hour++;
        if (t_hour == 24){
          t_hour = 0;
        }//h
      }//m
    }//s
    lastSecondsUpdate = millis();
  }
}
void serialPrintClock(int freq) {
    if ((uint32_t)(millis() - lastSerialPrint) >= freq) {
       Serial.printf("ora:%i:%i,%i\n",t_hour,t_min,t_sec);
       lastSerialPrint = millis();
    }  
}
void loop() {
  updateNTP(ntpRequestInterval);
  updateClock();
  serialPrintClock(1000);
}

// send an NTP request to the time server at the given address

