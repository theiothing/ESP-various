#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <WEMOS_SHT3X.h>
SHT3X sht30(0x45);
#include "init.h"
#include "web_page.h"

float temp;
float hum;

void XMLcontent()
{
  String XML = String(XML_Template); // copy template to String for to be able to modify dynamic values
  XML.replace("#value_001#", String(hum)+ "%");
  XML.replace("#value_002#", String(temp) + "°C");
  server.send(200, "text/xml", XML);
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("let's begin");
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);}
  WiFi.mode(WIFI_STA);
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Station IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", WebsiteContent);
  server.on("/xml", XMLcontent);
  server.begin();
}

void loop()
{
  if(sht30.get()==0){
    Serial.print("Temperature in Celsius : ");
    temp = sht30.cTemp;
    Serial.println(temp);
    hum= sht30.humidity;
    Serial.print("Relative Humidity : ");
    Serial.println(hum);
    Serial.println();
  }
  else
  {
    Serial.println("Error!");

  }
  server.handleClient();

}

