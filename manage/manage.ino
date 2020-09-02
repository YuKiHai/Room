#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include <string.h>

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

IPAddress local_IP(192, 168, 31, 116);
IPAddress gateway(192, 168, 31, 0);
IPAddress subnet(255, 255, 255, 0);

// 中断允许标志位
#define interruptPin D3
/*
  串口通信约定格式:
    ^Order#parameter$
*/

void setup() {
  // put your setup code here, to run once:
  pinMode(interruptPin, OUTPUT);

  Serial.begin(9600);

  wifiMulti.addAP("Xiaomi_782C", "wanghai1015");    // add wifi
  WiFi.config(local_IP, gateway, subnet);   // set WiFi 

  // wifi connection test
  Serial.print("Connecting");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
    Serial.println("");
  }
  // output base info
  Serial.print("Connected to"); Serial.println(WiFi.SSID());
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  // start flash system
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start");
  }



  // web server program run start
  server.onNotFound(handleUserRequest);
  server.on("/info", allInfoJson);
  server.begin();
  

}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}

// page not found callback this function
void handleUserRequest() {
  String reqResource = server.uri();  // get user requests Resource name
  // Serial.print("reqResource:"); Serial.println(reqResource);

  if (! handleFileRead(reqResource)) {
    server.send(404, "text/plain", "404 Not Found!!");
  }
}

bool handleFileRead(String resouce) {
  if (resouce.endsWith("/")) {
    resouce = "/index.html";
  }

  String contentType = getContentType(resouce);

  if (SPIFFS.exists(resouce)) {
    File file = SPIFFS.open(resouce, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename) {
  if      (filename.endsWith(".htm"))   return "text/html";
  else if (filename.endsWith(".html"))  return "text/html";
  else if (filename.endsWith(".css"))   return "text/css";
  else if (filename.endsWith(".js"))    return "application/javascript";
  else if (filename.endsWith(".png"))   return "image/png";
  else if (filename.endsWith(".gif"))   return "image/gif";
  else if (filename.endsWith(".jpg"))   return "image/jpeg";
  else if (filename.endsWith(".ico"))   return "image/x-icon";
  else if (filename.endsWith(".xml"))   return "text/xml";
  else if (filename.endsWith(".pdf"))   return "application/x-pdf";
  else if (filename.endsWith(".zip"))   return "application/x-zip";
  else if (filename.endsWith(".gz"))    return "application/x-gzip";
  return "text/plain";
}


void allInfoJson() {

  // D3 set HIGH
  digitalWrite(interruptPin, HIGH);

  // send command
  String command = "^all#$";
  Serial.println(command);


  char comData[43] = "", order[21], param[21];
  char temp, *flag = NULL;
  String jsonData = "{ ";

  // read LED Status
  delay(20);
  while (Serial.available() > 0) {
    temp = Serial.read();
    // start read
    if (temp == '^') {
      comData[0] = temp;
      
      delay(2);
      for (int i=1; Serial.available() > 0 && i <= 42; i++) {
        comData[i] = Serial.read();
        
        if (comData[i] == '$') {
          comData[i] = '\0';
          break;
        }
        
        if (comData[i] == '#') {
          comData[i] = '\0';
          flag = &comData[i+1];
        } 
        delay(2);
      }
      // Save one
      strcpy(order, &comData[1]);
      strcpy(param, flag);
      jsonData += "\"" + String(order) + "\": \"" + String(param) + "\", ";
    }
  }
  jsonData += " }";
  
  server.send(200, "application/json", jsonData);
}
