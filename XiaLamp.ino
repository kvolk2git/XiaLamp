#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"

const char* CodeName = "Smart Xiaomi Yeelight";
const char* CfgFileName = "XiaConfig.json";

ESP8266WebServer server(80);

#define PowerPin D5
#define BrightnessPin D6
#define ColorTemperaturePin D7

unsigned long previousMillis = 0;
uint8_t DelayPush = 50;

String webPage = "";
String webPageSetup = "";
String webPageReturn = "";
String webPageInfo = "";
String webPageInfoPrepare = "";

String ChipID = "";

String FontColor = "White";

String Str_APssid = "";
String Str_APpassword = "";
String Str_STAssid = "";
String Str_STApassword = "";
String Str_Theme = "";
//String Str_HostName = "";
uint8_t Int_STAEnable = 0;
uint8_t Int_LEDEnable = 1;

void Up_Pin_Init() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(PowerPin, OUTPUT);
  digitalWrite(PowerPin, LOW);
  pinMode(BrightnessPin, OUTPUT);
  digitalWrite(BrightnessPin, LOW);
  pinMode(ColorTemperaturePin, OUTPUT);
  digitalWrite(ColorTemperaturePin, LOW);
  delay(1000);
}

void Up_FScfg_WiFi_Web() {
  //Serial.println(F("Mounting file system..."));
  if (!SPIFFS.begin()) {
    Serial.println(F("Failed to mount file system"));
    return;
  }
  /*  if (!saveConfig()) {
      Serial.println(F("Failed to save config"));
    } else {
      Serial.println(F("Config saved"));
    }*/
  if (!loadConfig()) {
    Serial.println(F("Failed to load config"));
    saveConfig();
    loadConfig();
    //delay(1000);
    //ESP.restart();
  } else {
    Serial.println(F("Config loaded"));
  }
}

void Up_OTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(CodeName);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  ChipID = ESP.getChipId();
  Serial.println(CodeName);
  Serial.println();
  Up_Pin_Init();
  Up_FScfg_WiFi_Web();
  Up_OTA();
  Select_FontColor();



  /****************** webPage ******************/
  webPage += F("<!DOCTYPE html><html>");
  webPage += F("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  webPage += F("<link href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEACABoBQAAFgAAACgAAAAQAAAAIAAAAAEACAAAAAAAAAEAAAAAAAAAAAAAAAEAAAAAAABQmP0ATZr6AOHr/QAlgv0AEnb+AK3O/gBKkf4Ajr/9AMfe/gDl8fsATJX9AP79/QAiff4AnMT+AKnL/gDh7v4AG3r+AMPb/gD6/P0Apcr+AP7+/gAgev0AT5b+ACqH+gArhfsAq8z+AEiX/gAAc/gArs7+ACqC/gCkx/4AJ4L7AKbL/QBLlfsAvtf+APb6/gBQm/0Aqs3+AEeS/gD5+v4A+fj+AJa//gAZdvsASpT+APz+/gBAj/4Autb+AE2Y/gDG3f0AKoP8AP7+/QApf/sA9ff+AKnM/gD4+/4AlMD9AFOY/gCrzP0AKob9AP79/gD9/f0ATpn9ACyG/ACoyf4Ax9z8ACmG+QC/2f4A9/r+AKvP/gBQlfwARpL8ACZ9/QA8jfwAG3z8AB12/gDF2/4Awtv7APz+/QD9/P4Asc/+AE6Y/gBLmvsATJb8AAxw/gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsxFRsQM1MbR0obKgwbGxsrJyAbLjYtG0MiGx4SBhsbUDtPG0w8RhssCBs5MhYbGy8yHBtCLEYbLBEbGU0hGxtRO0QbMBQmG01LGw4UUhsbPU0cGyUPSBsyERs1MgobGwEURBsbGxs4CzAbDiwAGxskFAI3KQcNCU0/GwUyABsbGjROOzwyLCNAAxsTKEUbGxsXPjpBGBgfGxsbSR0EGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=\" rel=\"icon\" type=\"image/x-icon\" />");
  webPage += F("<style>html {text-shadow: 0 2px 2px rgba(0, 0, 0, 0.3); font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  webPage += F(".button {text-shadow: 0 2px 2px rgba(0, 0, 0, 0.3); width: 250px; background-color: #");
  webPage += Str_Theme;
  webPage += F("; border: none; color: ");//white; padding: 16px 40px; margin: 0 10px; border-radius: 25px;");
  webPage += FontColor;
  webPage += F("; padding: 16px 40px; margin: 0 10px; border-radius: 25px;");
  webPage += F("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  webPage += F(".buttonSetup {text-shadow: 0 2px 2px rgba(0, 0, 0, 0.3);background-color: #");
  webPage += Str_Theme;
  webPage += F("; border: none; color: ");
  webPage += FontColor;
  webPage += F("; padding: 8px 20px; text-decoration: none; font-size: 12px; margin: 2px; cursor: pointer;margin: 0 10px;border-radius: 5px;}");
  webPage += F("*::before,*::after{margin:0;padding:0;box-sizing:border-box;}");
  webPage += F("input[type=checkbox] {position: relative; top: 4px; margin: 0 10px; border-radius: 5px; background: #999; outline: 0; -webkit-appearance: none; cursor: pointer;}");
  webPage += F("input[type=checkbox]::after {content: ''; position: absolute; top: 2px; left: 2px; display: block; width: calc((100% - 4px)/2); height: calc(100% - 4px); border-radius: 25%; background: #eee; transition: all .1s linear;}");
  webPage += F("input[type=checkbox]:checked {background: #");
  webPage += Str_Theme;
  webPage += F(";}");
  webPage += F("input[type=checkbox]:checked::after {transform: translateX(100%);}");
  webPage += F("input[type=checkbox]:nth-child(1) {width: 36px; height: 20px; }");
  webPage += F("input[type=text] {border-radius: 5px; background: #");
  webPage += Str_Theme;
  webPage += F(";text-align: center; color: ");
  webPage += FontColor;
  webPage += F(";}");
  webPage += F("input[type = password] {border-radius: 5px;  background: #");
  webPage += Str_Theme;
  webPage += F(";text-align: center; color: ");
  webPage += FontColor;
  webPage += F(";}");
  webPage += F("p.dline {line-height: 0.5;}");
  webPage += F("p.dline {line-height: 0.5;}");
  webPage += F("select.listColor {height:21px; border-radius: 5px; border-style:none; text-shadow: 0 2px 2px rgba(0, 0, 0, 0.3); background: #");
  webPage += Str_Theme;
  webPage += F("; padding:0px; color: ");
  webPage += FontColor;
  webPage += F(";}");
//  webPage += F("select.listColor option.optionCurrent{background-color: #000000; color:");
//  webPage += FontColor;
//  webPage += F(";}");
  webPage += F("select.listColor option.option1{background-color: #000000; color:white;}");
  webPage += F("select.listColor option.option2{background-color: #808080; color:white;}");
  webPage += F("select.listColor option.option3{background-color: #C0C0C0; color:black;}");
  webPage += F("select.listColor option.option4{background-color: #FFFFFF; color:black;}");
  webPage += F("select.listColor option.option5{background-color: #FF00FF; color:black;}");
  webPage += F("select.listColor option.option6{background-color: #800080; color:white;}");
  webPage += F("select.listColor option.option7{background-color: #FF0000; color:white;}");
  webPage += F("select.listColor option.option8{background-color: #800000; color:white;}");
  webPage += F("select.listColor option.option9{background-color: #f87300; color:white;}");
  webPage += F("select.listColor option.option10{background-color: #FFFF00; color:black;}");
  webPage += F("select.listColor option.option11{background-color: #808000; color:white;}");
  webPage += F("select.listColor option.option12{background-color: #00FF00; color:black;}");
  webPage += F("select.listColor option.option13{background-color: #008000; color:white;}");
  webPage += F("select.listColor option.option14{background-color: #00FFFF; color:black;}");
  webPage += F("select.listColor option.option15{background-color: #008080; color:white;}");
  webPage += F("select.listColor option.option16{background-color: #0000FF; color:white;}");
  webPage += F("select.listColor option.option17{background-color: #000080; color:white;}");






  webPage += F("</style></head><body><h1>");
  webPage += CodeName;
  webPage += F("</h1> ");
  /**/
  webPageSetup = webPage;
  webPageInfoPrepare = webPage;
  /**/
  webPage += F("<p><a href = \"Power\"><button class=\"button\">On/Off</button></a></p>");
  webPage += F("<p><a href=\"Brightness\"><button class=\"button\">Brightness</button></a></p>");
  webPage += F("<p><a href=\"Color\"><button class=\"button\">Color</button></a></p>");
  webPage += F("<br><br><br><br><br><br><br><br>");
  webPage += F("<p><a href=\"Setup\"><button class=\"buttonSetup\">Setup</button></a><a href=\"Info\"><button class=\"buttonSetup\">Info</button></a></p></body></html>");

  webPageSetup += F("<form action=\"\" method=\"POST\">");
  webPageSetup += F ("<p>AP Wi-Fi SSID:<br><input type=\"text\" name=\"wwwAPssid\" id=\"idAPssid\" size=\"24\"></p>");
  webPageSetup += F ("<p>AP Wi-Fi Password:<br><input type=\"password\" name=\"wwwAPpassword\" size=\"32\"></p>");
  if (Int_STAEnable == 1) webPageSetup += F ("<p>Connect to existent Wi-Fi? <input type=\"checkbox\" name=\"wwwSTAEnable\" value=\"1\" checked></p>");
  else webPageSetup += F ("<p>Connect to other Wi-Fi? <input type=\"checkbox\" name=\"wwwSTAEnable\" value=\"1\"></p>");
  webPageSetup += F ("<p>Wi-Fi SSID:<br><input type=\"text\" name=\"wwwSTAssid\" id=\"idSTAssid\" size=\"24\"></p>");
  webPageSetup += F ("<p>Wi-Fi Password:<br><input type=\"password\" name=\"wwwSTApassword\" size=\"32\"></p>");
  if (Int_LEDEnable == 1) webPageSetup += F ("<p>Blue LED Enabled? <input type=\"checkbox\" name=\"wwwLEDEnable\" value=\"1\" checked></p>");
  else webPageSetup += F ("<p>Blue LED Enabled? <input type=\"checkbox\" name=\"wwwLEDEnable\" value=\"1\"></p>");

  webPageSetup += F ("<p>Theme:&nbsp;<select name=\"wwwTheme\" class=\"listColor\"><option hidden disabled selected class=\"optionCurrent\">Current</option><option value=\"000000\" class=\"option1\">Black</option><option value=\"808080\" class=\"option2\">Gray</option> <option value=\"C0C0C0\" class=\"option3\">Silver</option><option value=\"FFFFFF\" class=\"option4\">White</option> <option value=\"FF00FF\" class=\"option5\">Fuchsia</option><option value=\"800080\" class=\"option6\">Purple</option><option value=\"FF0000\" class=\"option7\">Red</option><option value=\"800000\" class=\"option8\">Maroon</option><option value=\"f87300\" class=\"option9\">Orange</option><option value=\"FFFF00\" class=\"option10\">Yellow</option><option value=\"808000\" class=\"option11\">Olive</option><option value=\"00FF00\" class=\"option12\">Lime</option><option value=\"008000\" class=\"option13\">Green</option><option value=\"00FFFF\" class=\"option14\">Aqua</option><option value=\"008080\" class=\"option15\">Teal</option><option value=\"0000FF\" class=\"option16\">Blue</option><option value=\"000080\" class=\"option17\">Navy</option></select></p>");

  webPageSetup += F("<br><br><br>");
  webPageSetup += F ("<p><input type=\"submit\" class=\"buttonSetup\" class=\"buttonSetup\" value=\"Save and Restart\"><input type=\"button\" class=\"buttonSetup\" onclick=\"history.back(-2); return false;\" value=\"Back\"/></p>");
  webPageSetup += F ("</form>");
  if (Str_APssid != "") {
    webPageSetup += F ("<script type=\"text/javascript\"> document.getElementById(\"idAPssid\").defaultValue = \"");
    webPageSetup += Str_APssid;
    webPageSetup += F ("\";</script>");
  }
  if (Str_STAssid != "") {
    webPageSetup += F ("<script type=\"text/javascript\"> document.getElementById(\"idSTAssid\").defaultValue = \"");
    webPageSetup += Str_STAssid;
    webPageSetup += F ("\";</script>");

  }
  webPageSetup += F("</body></html>");

  webPageReturn += F("<!DOCTYPE html><html><head><script type=\"text/javascript\">history.go(-2)</script></head></html>");

  //  Serial.print("WiFi.status: ");
  //  Serial.println(WiFi.status());
}

void loop() {
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();
  //Serial.println(currentMillis);
  //Serial.println(previousMillis);
  if (currentMillis - previousMillis >= DelayPush) {
    digitalWrite(PowerPin, LOW);
    digitalWrite(BrightnessPin, LOW);
    digitalWrite(ColorTemperaturePin, LOW);
  }
  server.handleClient();
}

void PressButton(uint8_t Button) {
  previousMillis = millis();
  if (Button == 1) {
    digitalWrite(PowerPin, HIGH);
  } else if (Button == 2) {
    digitalWrite(BrightnessPin, HIGH);
  } else if (Button == 3) {
    digitalWrite(ColorTemperaturePin, HIGH);
  }
}

void handleRoot() {
  server.send(200, "text/html", webPage);
}
void handlePower() {
  server.send(200, "text/html", webPage);
  PressButton(1);
}
void handleBrightness() {
  server.send(200, "text/html", webPage);
  PressButton(2);
}
void handleColor() {
  server.send(200, "text/html", webPage);
  PressButton(3);
}
void handleInfo() {
  ServerInfo();
  server.send(200, "text/html", webPageInfo);
  webPageInfo = "";
}
void handleSetup() {
  if (server.args() > 0 ) server.send(200, "text/html", webPageReturn);
  else server.send(200, "text/html", webPageSetup);
  Serial.print ("Setup.args: ");
  Serial.println (server.args());
  if (server.args() > 0 ) {
    Int_STAEnable = 0;
    Int_LEDEnable = 0;
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      Serial.print(server.argName(i));
      Serial.print(": ");
      Serial.println(server.arg(i));
      if (server.arg(i) != "") {
        if (server.argName(i) == "wwwAPssid") {
          Str_APssid = server.arg(i);
        }
        if (server.argName(i) == "wwwAPpassword") {
          if (server.arg(i).length() > 7) Str_APpassword = server.arg(i);
        }
        if (server.argName(i) == "wwwSTAssid") {
          Str_STAssid = server.arg(i);
        }
        if (server.argName(i) == "wwwSTApassword") {
          if (server.arg(i).length() > 7) Str_STApassword = server.arg(i);
        }
        if (server.argName(i) == "wwwSTAEnable") Int_STAEnable = server.arg(i).toInt();
        if (server.argName(i) == "wwwLEDEnable") Int_LEDEnable = server.arg(i).toInt();
        if (server.argName(i) == "wwwTheme") {
          Str_Theme = server.arg(i);
        }

        /*        Serial.println(F("----------"));
                Serial.println(Str_APssid);
                Serial.println(Str_APpassword);
                Serial.println(Int_STAEnable);
                Serial.println(Str_STAssid);
                Serial.println(Str_STApassword);
                Serial.println(Int_LEDEnable);
        */
      }
    }
    saveConfig_www();
    delay(1000);
    ESP.restart();
  }
}

/*
  json["APssid"] = Str_APssid;
  json["APpassword"] = Str_APpassword;
  json["STAEnable"] = Int_STAEnable;
  json["STAssid"] = Str_STAssid;
  json["STApassword"] = Str_STApassword;
  json["LEDEnable"] = Int_LEDEnable;
  //json["HostName"] = "Xiaomi.local";
*/

bool loadConfig() {
  File configFile = SPIFFS.open(CfgFileName, "r");
  if (!configFile) {
    Serial.println(F("Failed to open config file"));
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println(F("Config file size is too large"));
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println(F("Failed to parse config file"));
    return false;
  }

  const char* APssid = json["APssid"];
  const char* APpassword = json["APpassword"];
  const char* STAssid = json["STAssid"];
  const char* STApassword = json["STApassword"];
  const char* Theme = json["Theme"];
  //const char* HostName = json["HostName"];
  uint8_t STAEnable = json["STAEnable"];
  uint8_t LEDEnable = json["LEDEnable"];

  Str_APssid = APssid;
  Str_APpassword = APpassword;
  Str_STAssid = STAssid;
  Str_STApassword = STApassword;
  Str_Theme = Theme;
  //Str_HostName = "";
  Int_STAEnable = STAEnable;
  Int_LEDEnable = LEDEnable;

  // Real world application would store these values in some variables for
  // later use.

  Serial.print(F("AP SSID: "));
  Serial.println(APssid);
  Serial.print(F("AP Password: "));
  Serial.println(APpassword);
  //  if (STAEnable == 1) {
  if (STAEnable == 1) Serial.print(F("Enabled "));
  else Serial.print(F("Disabled "));
  Serial.println(F("connect to existent Wi-Fi."));
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("Wi-Fi SSID: "));
    Serial.println(STAssid);
    Serial.println(F("Wi-Fi Password: ********"));
  }
  //Serial.println(STApassword);
  //Serial.print(F("Loaded HostName: "));
  //Serial.println(HostName);
  //Serial.println(STAEnable);
  Serial.print(F("Blue LED "));
  if (LEDEnable == 1) Serial.println(F("enabled."));
  else Serial.println(F("disabled."));
  Serial.println();

  if (LEDEnable == 0) digitalWrite(LED_BUILTIN, HIGH);
  if (STAEnable == 1) WiFi.mode(WIFI_AP_STA);
  else WiFi.mode(WIFI_AP);

  Serial.print(F("Configuring access point "));
  Serial.print(APssid);
  WiFi.softAP(APssid, APpassword);
  Serial.print(F (". IP address: "));
  Serial.println(WiFi.softAPIP());

  if (STAEnable == 1) {
    Serial.print(F("Trying to connect to "));
    Serial.print(STAssid);
    WiFi.begin(STAssid, STApassword);
    delay(5000);
    Serial.print(F(". "));
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print(F("IP address: "));
      Serial.println(WiFi.localIP());
    }
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect(false);
      Serial.println(F("Not connected."));
    }
  }

  server.on("/", handleRoot);
  server.on("/Power", handlePower);
  server.on("/Brightness", handleBrightness);
  server.on("/Color", handleColor);
  server.on("/Setup", handleSetup);
  server.on("/Info", handleInfo);
  server.begin();
  Serial.println();
  Serial.println(F("HTTP server started"));
  Serial.print(F("http://"));
  Serial.println(WiFi.softAPIP());
  //  if (STAEnable == 1) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("http://"));
    Serial.println(WiFi.localIP());
  }
  return true;
}

bool saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["APssid"] = "SmartXiaomi-" + ChipID;
  json["APpassword"] = "12345678";
  json["STAEnable"] = "0";
  json["STAssid"] = "";
  json["STApassword"] = "";
  json["LEDEnable"] = "1";
  json["Theme"] = "f87300";
  //json["HostName"] = "Xiaomi.local";

  File configFile = SPIFFS.open( CfgFileName, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  json.printTo(configFile);
  return true;
}

bool saveConfig_www() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["APssid"] = Str_APssid;
  json["APpassword"] = Str_APpassword;
  json["STAEnable"] = Int_STAEnable;
  json["STAssid"] = Str_STAssid;
  json["STApassword"] = Str_STApassword;
  json["LEDEnable"] = Int_LEDEnable;
  json["Theme"] = Str_Theme;
  //json["HostName"] = "Xiaomi.local";

  File configFile = SPIFFS.open( CfgFileName, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  json.printTo(configFile);
  return true;
}

void ServerInfo() {
  webPageInfo += webPageInfoPrepare;
  webPageInfo += F("<p class=\"dline\"><b>AP SSID:</b> ");
  webPageInfo += Str_APssid;
  webPageInfo += F("</p>");
  webPageInfo += F("<p class=\"dline\"><b>IP:</b> ");
  webPageInfo += WiFi.softAPIP().toString();
  webPageInfo += F("</p>");
  webPageInfo += F("<p class=\"dline\"><b>AP Password:</b> ");
  webPageInfo += Str_APpassword;
  webPageInfo += F("</p><br>");
  if (WiFi.status() == WL_CONNECTED) {
    webPageInfo += F("<p class=\"dline\"><b>Wi-Fi SSID:</b> ");
    webPageInfo += Str_STAssid;
    webPageInfo += F("</p>");
    webPageInfo += F("<p class=\"dline\"><b>IP:</b> ");
    webPageInfo += WiFi.localIP().toString();
    webPageInfo += F("</p>");
    webPageInfo += F("<p class=\"dline\"><b>AP Password:</b> ********</p>");
  } else webPageInfo += F("<p class=\"dline\"><b>Wi-Fi:</b> no connection to existing network.</p><br><br>");
  webPageInfo += F("<br><br><br><br><br><br><br><br><br><br><br><br>");
  webPageInfo += F ("<p><input type=\"button\" class=\"buttonSetup\" onclick=\"history.back(-2); return false;\" value=\"Back\"/></p>");
  webPageInfo += F("</body></html>");
}

void Select_FontColor() {
  if ((Str_Theme == F("C0C0C0")) || (Str_Theme == F("FFFFFF")) || (Str_Theme == F("FF00FF")) || (Str_Theme == F("FFFF00")) || (Str_Theme == F("00FF00")) || (Str_Theme == F("00FFFF"))) FontColor = "Black";
}

