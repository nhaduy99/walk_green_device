#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`


// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);

/* Set these to your desired credentials. */
const char *ssid = "seppy";
const char *password = "123456789";
String url = "";

String user_wifi = "";
String user_password = "";

const char* host = "54.172.116.30";
//const char* host = "54.172.116.30:3000";    //http://54.172.116.30:3000/notify?device_id=test2&humidity=0.9

char no_wifi=0;
String local_wifi[10];

String sensor = "";
int humidity = 0;
float value = 0;
char token=0;
ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
  char temp[1000];
  char temp_2[1000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  
void handleRoot() {
    snprintf ( temp, 1000,
      "<html>\
        <head>\
          <meta charset=\"/utf-8\"/>\
          <title>ESP WALK_GREEN access portal - IoT Protocol from Konel Inc</title>\
          <style>\
            body { background-color: #blue; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
          </style>\
        </head>\
        <body>\
          <h1>Welcome to the WALK_GREEN project</h1>\
        <p>Please access the internet in order to use the functions of the device</p>\
            <form class=\"/\"/action=\'/submit\'/ method=\'/POST\'/>\
                <input type=\"/text\"/ name=\"/wifi\" /placeholder=\"Wifi name\"/>\
                <input type=\"/text\"/ name=\"/password\"/placeholder=\"Password\"/>\
                <button type=\"/submit\"/>Access</button>\
        </body>\
      </html>"
      
  );

server.send(200, "text/html",temp);

}


void handleSubmit() {

  if (server.args() > 0 ) {
      user_wifi += server.arg(0);
      user_password += server.arg(1);
  }
      Serial.println ("Wifi name:" + user_wifi);
      Serial.println ("Password:" + user_password);
      delay(1000);

  //--------------Checking  wifi---------------
 //  WiFi.mode(WIFI_AP);
  //WiFi.begin(user_wifi, user_password);
  int counter = 0;
  WiFi.begin(user_wifi.c_str(), user_password.c_str());

  jump:

 
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter>20)
    {
      Serial.println("You may access the wrong password!!!!");
  //    WiFi.mode(WIFI_STA);
      counter = 0;
      snprintf ( temp, 1000,
      "<html>\
        <head>\
          <meta charset=\"/utf-8\"/>\
          <title>ESP WALK_GREEN access portal - IoT Protocol from Konel Inc</title>\
          <style>\
            body { background-color: #red; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
          </style>\
        </head>\
        <body>\
          <h1>You may give the wrong wifi&password </h1>\
           <p>Please back the main website to correct your information</p>\
        </body>\
      </html>"
      );
    server.send(200, "text/html",temp);
      goto jump;
    }
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);
  //correct_password_html();
   snprintf ( temp, 1000,
      "<html>\
        <head>\
          <meta charset=\"/utf-8\"/>\
          <title>ESP WALK_GREEN access portal - IoT Protocol from Konel Inc</title>\
          <style>\
            body { background-color: #green; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
          </style>\
        </head>\
        <body>\
          <h1>Congratulation!!Connected successfully</h1>\
           <h2>Now enjoy the IoT world</h2>\
        </body>\
      </html>"
      );
  server.send(200, "text/html",temp);
  token = 1;
  user_wifi = "";
  user_password="";
}
       



void setup() {
  //Sensor pin declaration
   pinMode(16, INPUT_PULLUP);
   pinMode(A0, INPUT_PULLUP);

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  //Wifi configuration (Booting internal wifi)
	delay(1000);
	Serial.begin(9600);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
  WiFi.mode(WIFI_AP);

  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.begin();
  
  Serial.println("HTTP server started");
  wifi_scan();
//  server.on("/inline", [](){
//    server.send(200, "text/plain", "this works as well");
//  });

  
}

void loop() {
	server.handleClient();
  if(token ==1){ Data();}
  
}

//---------------------FUNCTIONS----------------
String wet_dry(){
   if(digitalRead(16)) { return "DRY"; }
    else{ return "WET";}
}
int wet_dry_2(){
   if(digitalRead(16)) { return 1; }
    else{ return 0; }
}
float percent()
{
  value = analogRead(A0);

  value = 100-((value - 360)/600)*100;
  if(value>100){value = 100;}
  if(value<0){value = 0;}
  return value;
}
float humid()
{
  value = analogRead(A0);

  value = 1-((value - 360)/600);
  if(value>1){value = 1;}
  if(value<0){value = 0;}
  return value;
}
//-------------Sending sensor data--------------
void Data(){
 // Sensor_OLED();
 sensor = wet_dry();
  Serial.print("connecting to ");
//  Serial.println(host);
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
        Serial.print("Sensor data:");
        Serial.println(sensor);

        String url = "/notify?device_id=test&humidity=0.7";

        
        Serial.println(url);
        Serial.println(host);
        Serial.println(String(httpPort));
        
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" + 
                  //   "Content-Length:" + String(url) +"\r\n"+
                     "Connection: close\r\n\r\n");

          
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  delay(5000);
  
}

void correct_password_html()
{
      snprintf ( temp, 1000,
      "<html>\
        <head>\
          <meta charset=\"/utf-8\"/>\
          <title>ESP WALK_GREEN access portal - IoT Protocol from Konel Inc</title>\
          <style>\
            body { background-color: #green; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
          </style>\
        </head>\
        <body>\
          <h1>Congratulation!!Connected successfully</h1>\
           <h2>Now enjoy the IoT world</h2>\
        </body>\
      </html>"
      );
server.send(200, "text/html",temp);
}

void wrong_password_html()
{
  snprintf ( temp_2, 1000,
      "<html>\
        <head>\
          <meta charset=\"/utf-8\"/>\
          <title>ESP WALK_GREEN access portal - IoT Protocol from Konel Inc</title>\
          <style>\
            body { background-color: #red; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
          </style>\
        </head>\
        <body>\
          <h1>You may give the wrong wifi&password </h1>\
           <p>Please back the main website to correct your information</p>\
        </body>\
      </html>"
      );
    server.send(200, "text/html",temp_2);
}

void wifi_scan()
{
    Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  no_wifi = n;
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
      local_wifi[i] = WiFi.SSID(i); 
    }
  }
  Serial.println("");

}

