#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <Adafruit_MCP23008.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_MCP9808.h"
#define OLED_RESET 0

Adafruit_SSD1306 display(OLED_RESET);

class ccc
{
  public:
  ccc()
  {
    
  }
  
  const char* ssid     = "errans";
  const char* password = "zamb0rah";
  const char* host = "192.168.1.143";
  const int httpPort = 80;
  int failCount = 0;
  bool connected = 0;
  bool started = 0;
  
  bool connect() {
    if (started == 0) {
      Serial.print("Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);
      started == 1;
      unsigned long connectMillis = millis();
       while (WiFi.status() != WL_CONNECTED) {
        if (connectMillis + 30000 <= millis()) {
          //we failed--reset and try again next time.
          //no loop here because we need to clock to update.
          WiFi.disconnect();
          connected = 0;
          return 1;
        };
        delay(500);
        Serial.print(".");
        };
        connected = 1;
    } else {
      if (connected == 0) {
        unsigned long connectMillis = millis();
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED) {
          if (connectMillis + 30000 <= millis()) {
            //we failed--reset and try again next time.
            //no loop here because we need to clock to update.
            WiFi.disconnect();
            connected = 0;
            return 1;
          };
          delay(500);
          Serial.print(".");
        };
        
        Serial.println("");
        Serial.println("WiFi connected");  
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connected = 1;
        return 0;
      };
    };
  };
  
  int fetchAlarm() {
    int result = 0;
    if (connected == 1) {
      WiFiClient client;
      if (!client.connect(host, httpPort)) {
        Serial.println("fetchAlarm() connection failed...");
        failCount++;
        Serial.println(failCount);
        delay(1000);
        return 1;
      };
      String url = "/api.php?alarm=true";
      Serial.print("Requesting URL: ");
      Serial.println(url);
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
      delay(1000);
      int response = 0;
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
        if (response == 1) {
          Serial.println();
          Serial.print("Alarm is set to ");
          Serial.println(line);
          int alarm = line.toInt();
          line = client.readStringUntil('\r');
          bool alarmSet = line.toInt();
          return (alarm * 10) + alarmSet;
        };
        if (line == "\nALARM") {
          response = 1;
        };
      };
    };
  };
  
  int saveAlarm(int alarm) {
    if (connected == 1) {
      WiFiClient client;
        if (!client.connect(host, httpPort)) {
          Serial.println("saveAlarm() connection failed...");
          failCount++;
          Serial.println(failCount);
          delay(1000);
          return 1;
      };
      String url = "/api.php?alarm=true&subbing=";
      url = url + alarm;
      Serial.print("Requesting URL: ");
      Serial.println(url);
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
      delay(1000);
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
      };
      return 0;
    };
  };
  
  int alarmSwitch(bool setting) {
    if (connected == 1) {
      WiFiClient client;
      if (!client.connect(host, httpPort)) {
        Serial.println("alarmSwitch() connection failed...");
        failCount++;
        Serial.println(failCount);
        delay(1000);
        return 1;
      };
      String url = "/api.php?alarm=true&switch=";
      
      url = url + setting;
      Serial.print("Requesting URL: ");
      Serial.println(url);
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
      delay(1000);
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
      };
      return 0;
    };
  };
  
  int serverTemp(float temp){
    //This tells us which thermostat we are. 1 is master bedroom, 2 is office
    int thermId = 1;
    if (connected == 1) {
      WiFiClient client;
      if (!client.connect(host, httpPort)) {
        Serial.println("serverTemp() connection failed...");
        failCount++;
        Serial.println(failCount);
        delay(1000);
        return 1;
      };
      //only if we connected do we perform the rest--or we can freeze the clock!
      String url = "/thermostat_api.php?inSub=true&temp=";
      url = url + temp;
      url = url + "&id=";
      url = url + thermId;
      Serial.print("Requesting URL: ");
      Serial.println(url);
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
      delay(2000);
      int response = 0;
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
        if (line.startsWith("\nset:")) {
          line.remove(0,5);
          Serial.println();
          Serial.print("returning: ");
          Serial.println(line.toInt());
          return line.toInt();
        };
      };
    };
  };
  
  int setServerOverride(int serverOverride) {
    //This tells us which thermostat we are. 1 is master bedroom
    if (connected == 1) {
      int thermId = 1;
      WiFiClient client;
      if (!client.connect(host, httpPort)) {
        Serial.println("setServerOverride() connection failed...");
        failCount++;
        Serial.println(failCount);
        delay(1000);
        return 1;
      };
      
      String url = "/thermostat_api.php?inSub=true&override=";
      url = url + serverOverride;
      url = url + "&id=";
      url = url + thermId;
      Serial.print("Requesting URL: ");
      Serial.println(url);
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
      delay(1000);
      int response = 0;
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
        if (line.startsWith("\nset:")) {
          line.remove(0,5);
          Serial.print(line);
          Serial.println();
        };
      };
      return 0;
    };
  };
};

class output
{
  Adafruit_7segment matrix = Adafruit_7segment();
  Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
  Adafruit_MCP23008 mcp;
  
  RTC_PCF8523 rtc;
  ccc comms = ccc();
  
  private:
  bool starting = 1;
  int alarm = 0;
  bool alarmSet = 0;
  int setting = 65;
  int newSetting = setting;
  bool listening = 1;
  int cursor = 0;
  bool pm = 0;
  int h1 = 0;
  int h2 = 0;
  int m1 = 0;
  int m2 = 0;
  unsigned long lcdMillis = 0;
  unsigned long redMillis = 0;
  unsigned long displayModeMillis = 0;
  unsigned long serverTempMillis = 0;
  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  public:
  int lcdMode = 0;
  bool alarming = 0;
  bool alarmed = 0;
  output()
  {

  }
  
  void begin()
  {
    mcp.begin();
    for (uint8_t i=0; i<6; i++) {
      mcp.pinMode(i, INPUT);
      mcp.pullUp(i, HIGH);
    };
    
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while(1){
        
      };
    };
    
    if (!tempsensor.begin()) {
      Serial.println("Couldn't find MCP9808!");
      while(1) {
        
      };
    };
    matrix.begin(0x70);
    matrix.setBrightness(0);
    matrix.clear();
    matrix.drawColon(true);
    matrix.writeDisplay();
    
    DateTime now = rtc.now();
    int hour = now.hour();
    
    if (hour >= 12) {
      pinMode(12,OUTPUT);
      //analogWrite(12,50);
      if (hour > 12) {
        hour = hour - 12;
      };
    } else {
      if (hour == 0) {
        hour = 12;
      };
      //analogWrite(12,0);
    };
    
    if (hour < 10) {
      matrix.writeDigitRaw(0, 0x0);
      matrix.writeDigitNum(1, hour);
    }

    else {
     matrix.writeDigitNum(0, (hour / 10));
     matrix.writeDigitNum(1, (hour % 10));
    }

    if (now.minute() < 10) {
      matrix.writeDigitNum(3,0);
      matrix.writeDigitNum(4, now.minute());
    }

    else {
     matrix.writeDigitNum(3, (now.minute() / 10));
     matrix.writeDigitNum(4, (now.minute() % 10));
    };

    matrix.writeDisplay();
    
    comms.connect();

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setCursor(5,0);
    display.print("Loading...");
    display.display();
    display.clearDisplay();
    float c = tempsensor.readTempC();
    float f = c * 9.0 / 5.0 + 32;

    display.setCursor(5,0);
    display.print(daysOfTheWeek[now.dayOfTheWeek()]);
    display.setCursor(5,16);
    display.print(now.month(), DEC);
    display.print('/');
    display.print(now.day(), DEC);
    display.print('/');
    display.print(now.year(), DEC);
    display.setCursor(5,32);
    display.print("Now: ");
    display.print(f, 1);
    Serial.println("Refreshed LCD");
    
    int alarmServer = comms.fetchAlarm();
    if (alarmServer == 1) {
      //deal with a lack of a server response:
      comms.connected = 0;
      alarmServer = 00000;
    };
    alarmSet = alarmServer % 10;
    alarm = alarmServer / 10;
    if (alarmSet == 1)
      analogWrite(13, 50);
    else
      analogWrite(13,0);
    setting = comms.serverTemp(f);
    if (setting == 1) {
      //deal with a lack of a server response:
      comms.connected = 0;
      setting = 0;
    };
    
    if (setting <= 60) {
      setting = 65;
    };
    display.setCursor(5,48);
    display.print("Set: ");
    display.print(setting);
    lcdMode = 1;
  };
  
  void refreshLcdTemp()
  {
    unsigned long currentLCDMillis = millis();
    
    if (currentLCDMillis >= lcdMillis + 1000) {
      float c = tempsensor.readTempC();
      float f = c * 9.0 / 5.0 + 32;
      if (lcdMode < 2) {
        display.clearDisplay();
        DateTime now = rtc.now();
        display.setCursor(5,0);
        display.print(daysOfTheWeek[now.dayOfTheWeek()]);
        display.setCursor(5,16);
        display.print(now.month(), DEC);
        display.print('/');
        display.print(now.day(), DEC);
        display.print('/');
        display.print(now.year(), DEC);
        display.setCursor(5,32);
        display.print("Now: ");
        display.print(f, 1);
        display.setCursor(5,48);
        if (comms.connected == 1) {
          display.print("Set: ");
          display.print(setting);
        } else{
          display.print("No Connect!");
        };
        //display.display();
        //Serial.println("Refreshed LCD");
        lcdMillis = millis();
      };
    };
  };
  
  //this method is where we reconnect to the server
  //if we have had any connection errors from any calls to the ccc class
  void refreshFromServer()
  {
    unsigned long currentTempMillis = millis();
    if (currentTempMillis >= serverTempMillis + 60000) {
      float c = tempsensor.readTempC();
      float f = c * 9.0 / 5.0 + 32;
      int potentialSetting = comms.serverTemp(f);
      if (potentialSetting > 60){
        setting = potentialSetting;
      } else {
        if (potentialSetting == 1) comms.connected = 0;
      };
      
      int alarmServer = comms.fetchAlarm();
      if (alarmServer != 1) {
        alarmSet = alarmServer % 10;
        alarm = alarmServer / 10;
        Serial.print("alarmSet ");
        Serial.println(alarmSet);
        serverTempMillis = millis();
      } else {
       comms.connected = 0; 
      };
    };
    if (comms.connected == 0) {
      if (comms.connect() == 1) {
        Serial.println("did not connect.");
      };
    };
  };
  
  void refreshRedTime()
  {
    unsigned long currentMillis = millis();
    
    if (currentMillis >= redMillis + 100) {
      DateTime now = rtc.now();
      int hour = now.hour();
      
      if (hour >= 12) {
        analogWrite(12,50);
        if (hour > 12) {
          hour = hour - 12;
        };
      } else {
        if (hour == 0) {
          hour = 12;
        };
        analogWrite(12,0);
      };
      
      if (hour < 10) {
        matrix.writeDigitRaw(0, 0x0);
        matrix.writeDigitNum(1, hour);
      }

      else {
       matrix.writeDigitNum(0, (hour / 10));
       matrix.writeDigitNum(1, (hour % 10));
      }

      if (now.minute() < 10) {
        matrix.writeDigitNum(3,0);
        matrix.writeDigitNum(4, now.minute());
      }

      else {
       matrix.writeDigitNum(3, (now.minute() / 10));
       matrix.writeDigitNum(4, (now.minute() % 10));
      };

      matrix.writeDisplay();
      
      //check to see if we can set off the alarm:
      int checkAlarm = (now.hour() * 100) + now.minute();
      if (alarmSet == 1)
        analogWrite(13, 50);
      else
        analogWrite(13,0);
      if (checkAlarm == alarm && alarmed == 0 && alarmSet == 1) {
        alarming = 1;
        Serial.println("Set alarming = 1");
      } else if (checkAlarm == alarm && alarmed == 1) {
        //do nothing, wait
      } else if (checkAlarm != alarm && alarmed == 1) {
        alarmed = 0;
      };
    };
  };
  
  void displayMode()
  {
    //refreshLcdTemp();
    
    if (buttonsRead() == 6) {
      listening = 1;
    };
    
    //change the alarm if the alarm set button is pressed
    if (buttonsRead() == 5 && listening == 1) {
      listening = 0;
      if (alarmSet == 0) {
        alarmSet = 1;
        analogWrite(13,50);
      } else {
        alarmSet =0;
        analogWrite(13,0);
      };
      
      if (comms.alarmSwitch(alarmSet) == 1) {
        display.clearDisplay();
        display.setCursor(5,0);
        display.print("FAILED");
        display.display();
        delay(5000);
        display.clearDisplay();
        display.display();
      };
    };
    
    //base mode is 0.
    if (lcdMode == 0 && buttonsRead() == 4 && listening == 1) {
      lcdMode = 1;
      displayModeMillis = millis();
      Serial.println("LCD Mode = 1.");
      listening = 0;
    };
    
    //DISPLAY THE DATE AND TEMP IF THE SELECT BUTTON IS PRESSED
    if (lcdMode == 1) {
      refreshLcdTemp();
      lcdMillis = millis();
      display.display();
      if (buttonsRead() == 4 && listening == 1) {
        lcdMode = 2;
        listening = 0;
        displayModeMillis = millis();
        Serial.println("LCD Mode = 2.");
      };
      
      if (buttonsRead() == 1 && listening == 1) {
        lcdMode = 4;
        listening = 0;
        displayModeMillis = millis();
        Serial.println("LCD Mode = 4");
      };
      
      if (buttonsRead() == 3 && listening == 1) {
        lcdMode = 4;
        listening = 0;
        displayModeMillis = millis();
        Serial.println("LCD Mode = 4");
      };
      
      if (millis() >= displayModeMillis + 5000) {
        lcdMode = 0;
        display.clearDisplay();
        display.display();
        Serial.println("LCD Mode = 0.");
      };
    };
    
    //IF SELECT BUTTON IS PRESSED WHILE DISPLAYING,
    //INIT ALARM SCREEN
    if (lcdMode == 2) {
      display.clearDisplay();
      display.setCursor(5,0);
      display.print("Loading...");
      display.display();
      if (int alarmServer = comms.fetchAlarm() == 1) {
        display.clearDisplay();
        display.setCursor(5,0);
        display.print("FAILED");
        display.display();
        delay(5000);
        display.clearDisplay();
        display.display();
        lcdMode = 1;
      } else {
        alarmSet = alarmServer % 10;
        alarm = alarmServer / 10;
        display.clearDisplay();
        display.setCursor(5,0);
        display.print("Alarm Set:");
        display.setCursor(5,16);
        
        if (alarm < 100) {
          alarm = alarm + 1200;
          pm = false;
        } else {
          if (alarm >= 1200) {
            pm = true;
          } else {
            pm = false;
          };
          if (alarm >= 1300) {
            alarm = alarm - 1200;
          };
        };
        
        h1 = alarm / 1000;
        h2 = alarm / 100 % 10;
        m1 = alarm % 100 / 10;
        m2 = alarm % 100 % 10;
        
        display.setTextSize(3);
        display.print(h1);
        display.print(h2);
        display.print(":");
        display.print(m1);
        display.print(m2);

        if (pm == false) {
          display.print("A");
        } else {
          display.print("P");
        };
        lcdMode = 3;
        display.setCursor(5,32);
        display.write(24);
        display.display();
      };
    };
    
    //SET THE ALARM AND SAVE TO THE SERVER
    if (lcdMode == 3 && listening == 1) {
      if (buttonsRead() == 0) {
        listening = 0;
        Serial.println("got left");
        if (cursor == 0){
          cursor = 5;
        }
        else{
          cursor --;
        };
        display.clearDisplay();
        display.setCursor(5,0);
        display.setTextSize(2);
        display.print("Alarm Set:");
        display.setTextSize(3);
        display.setCursor(5,16);
        display.print(h1);
        display.print(h2);
        display.print(":");
        display.print(m1);
        display.print(m2);
        if (pm == 0)
          display.print("A");
        else
          display.print("P");
        display.setCursor(5 + (cursor * 18),32);
        display.write(24);
        display.display();
        Serial.println(cursor);
      };
      if (buttonsRead() == 2) {
        listening = 0;
        Serial.println("got right");
        if (cursor == 5){
          cursor = 0;
        }
        else{
          cursor ++;
        };
        display.clearDisplay();
        display.setCursor(5,0);
        display.setTextSize(2);
        display.print("Alarm Set:");
        display.setTextSize(3);
        display.setCursor(5,16);
        display.print(h1);
        display.print(h2);
        display.print(":");
        display.print(m1);
        display.print(m2);
        if (pm == 0)
          display.print("A");
        else
          display.print("P");
        display.setCursor(5 + (cursor * 18),32);
        display.write(24);
        display.display();
        Serial.println(cursor);
      };
      if (buttonsRead() == 1) {
        listening = 0;
        switch(cursor) {
          case 0: if (h1 == 0) {
              h1 = 1;
            } else {
                h1 = 0;
            };
            //display.print(h1);
            break;
          case 1: if (h1 == 0) {
              if (h2 == 9)
                h2 = 0;
              else
                h2++;
            }
            else {
              if (h2 >= 2)
                h2 = 0;
              else
                h2++;
            };
            //display.print(h2);
            break;
          case 2: break;
          case 3: if (m1 == 5) {
              m1 = 0;
            } else {
                m1++;
            };
            //display.print(m1);
            break;
          case 4: if (m2 == 9) {
              m2 = 0;
            } else {
                m2++;
            };
            //display.print(m2);
            break;
          case 5: if (pm == 1) {
              Serial.println("pm / changing to am...");
              //display.print("A");
              pm = 0;
              Serial.println(pm);
              break;
            }
            else{
              Serial.println("am / changing to pm...");
              //display.print("P");
              pm = 1;
              Serial.println(pm);
              break;
            };
        };
        display.clearDisplay();
        display.setCursor(5,0);
        display.setTextSize(2);
        display.print("Alarm Set:");
        display.setTextSize(3);
        display.setCursor(5,16);
        display.print(h1);
        display.print(h2);
        display.print(":");
        display.print(m1);
        display.print(m2);
        if (pm == 0)
          display.print("A");
        else
          display.print("P");
        display.setCursor(5 + (cursor * 18),32);
        display.write(24);
        display.display();
        Serial.println(cursor);
      };
      if (buttonsRead() == 3) {
        listening = 0;
        switch(cursor) {
          case 0: if (h1 == 1) {
              h1 = 0;
            } else {
                h1 = 1;
            };
            //display.print(h1);
            break;
          case 1: if (h1 == 0) {
              if (h2 == 0)
                h2 = 9;
              else
                h2--;
            }
            else {
              if (h2 > 2)
                h2 = 2;
              else {
                if (h2 == 0)
                  h2 = 2;
                else
                  h2--;
              };
            };
            //display.print(h2);
            break;
          case 2: break;
          case 3: if (m1 == 0) {
              m1 = 5;
            } else {
                m1--;
            };
            //display.print(m1);
            break;
          case 4: if (m2 == 0) {
              m2 = 9;
            } else {
                m2--;
            };
            //display.print(m2);
            break;
          case 5: if (pm == 0){
              pm = 1;
              display.print("P");
            }
            else{
              pm = 0;
              //display.print("A");
            };
        };
        display.clearDisplay();
        display.setCursor(5,0);
        display.setTextSize(2);
        display.print("Alarm Set:");
        display.setTextSize(3);
        display.setCursor(5,16);
        display.print(h1);
        display.print(h2);
        display.print(":");
        display.print(m1);
        display.print(m2);
        if (pm == 0)
          display.print("A");
        else
          display.print("P");
        display.setCursor(5 + (cursor * 18),32);
        display.write(24);
        display.display();
        Serial.println(cursor);
      };
      if (buttonsRead() == 4 && listening == 1) {
        listening = 0;
        display.setTextSize(2);
        display.clearDisplay();
        display.setCursor(5,0);
        display.print("saving...");
        display.display();
        int alarmSend = (h1*1000) + (h2 * 100) + (m1 * 10) + m2;
        if (pm == 1){
          if ((alarmSend / 100) != 12){
            alarmSend = alarmSend + 1200;
          };
        } else {
          if ((alarmSend / 100) == 12) {
            alarmSend = alarmSend - 1200;
          };
        };
        Serial.print("Sending ");
        Serial.println(alarmSend);
        if (comms.saveAlarm(alarmSend) == 1) {
          display.clearDisplay();
          display.setCursor(5,0);
          display.print("FAILED");
          display.display();
          delay(5000);
          display.clearDisplay();
          display.display();
        } else {
          alarm = alarmSend;
          cursor = 0;
          displayModeMillis = millis();
        };
        lcdMode = 1;
      };
    };
    
    //INIT THERMOSTAT SETTING SCREEN
    if (lcdMode == 4) {
      newSetting = setting;
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(5,0);
      display.print("Set temp:");
      display.setCursor(5,16);
      display.setTextSize(3);
      display.print(newSetting);
      display.display();
      lcdMode = 5;
    };

    //SET THE THERMOSTAT AND SEND TO SERVER
    if (lcdMode == 5 && listening == 1){
      if (buttonsRead() == 1 && listening == 1) {
        listening = 0;
        newSetting++;
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(5,0);
        display.print("Set temp:");
        display.setCursor(5,16);
        display.setTextSize(3);
        display.print(newSetting);
        display.display();
      } else if (buttonsRead() == 3 && listening == 1) {
        listening = 0;
        newSetting--;
        display.setCursor(0,1);
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(5,0);
        display.print("Set temp:");
        display.setCursor(5,16);
        display.setTextSize(3);
        display.print(newSetting);
        display.display();
      } else if (buttonsRead() == 4 && listening == 1) {
        listening == 0;
        lcdMode = 1;
        display.clearDisplay();
        display.setCursor(5,0);
        display.setTextSize(2);
        display.print("Saving...");
        display.display();
        setting = newSetting;
        if (comms.setServerOverride(newSetting) == 1) {
          display.clearDisplay();
          display.setCursor(5,0);
          display.print("FAILED");
          display.display();
          delay(5000);
          display.clearDisplay();
          display.display();
        };
      };
    };
  };
  
  int buttonsRead()
  {
    bool pressed = 1;
    if(!mcp.digitalRead(0)){ //select
      return 4;
    } else
      pressed = 0;
    
    if (!mcp.digitalRead(1)){ //left
      return 0;
    } else
      pressed = 0;
    
    if (!mcp.digitalRead(2)){ //right
      return 2;
    } else
      pressed = 0;
    
    if (!mcp.digitalRead(3)){ //up
      return 1;
    } else
      pressed = 0;
    
    if (!mcp.digitalRead(4)){ //down
      return 3;
    } else
      pressed = 0;
    
    if (!mcp.digitalRead(5)){ //alarm set on/ off
      return 5;
    } else
      pressed = 0;
    
    if (pressed == 0) {
      return 6;
    };
  };
};

output Overseer = output();

void setup() {
  Serial.begin(9600);
  Serial.println();
  Overseer.begin();
  pinMode(14,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  Serial.println("Setup complete.");
}

void loop() {
  Overseer.displayMode();
  Overseer.refreshRedTime();
  
  //Careful--this halts all execution, including updating the clock until 
  //we get a response from the server, or timeout
  Overseer.refreshFromServer();
  
  //here we want to halt all execution if there is an alarm.
  if (Overseer.alarming == 1) {
    Serial.println("Entered Alarming main loop");
    Overseer.alarming = 0;
    Overseer.alarmed = 1;
    bool stillAlarming = 1;
    bool on = 1;
    while (stillAlarming == 1) {
      if (Overseer.buttonsRead() == 4) {
        stillAlarming = 0;
      };
      Serial.println(stillAlarming);
      if (on == 1){
        on = 0;
        digitalWrite(14,HIGH);
      } else {
        on = 1;
        digitalWrite(14,LOW);
      };
      delay(200);
    };
  } else {
    analogWrite(14,0);
  };
}
