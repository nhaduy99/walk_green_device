#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`


// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);

/* Set these to your desired credentials. */
//Yasu has changed somthing on development branch!
const char *ssid = "WalkGreenID:1A-FE-34-06-97-08";
const char *password = "34069708";
String url = "";

String user_wifi = "";
String user_password = "";

const char* host = "54.172.116.30";

String sensor = "";
int humidity = 0;
float value = 0;
char token=0;
int counter = 0;

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
  char temp[1000];
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
          <h2>Welcome to the WALK_GREEN project</h2>\
          <h3>Please access the internet in order to use the functions of the device</h3>\
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
// read the value of input name and password of local wifi
  if (server.args() > 0 ) {
      user_wifi += server.arg(0);
      user_password += server.arg(1);
  }
      Serial.println ("Wifi name:" + user_wifi);
      Serial.println ("Password:" + user_password);
      delay(1000);
 //--------------Checking  wifi---------------
  WiFi.mode(WIFI_AP);
 
  WiFi.begin(user_wifi.c_str(), user_password.c_str());  //Giving the wifi and password to esp8266 for connecting to local wifi

  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        counter++;
          if (counter>30){
            Serial.println("You may access the wrong password!!!!");
           //WiFi.mode(WIFI_AP);
           wrong_password_html();
           Serial.println("Counter 2: "+String(counter));
          // counter = 0;
           user_wifi = "";
           user_password="";
          break;
         }
    }
 if(counter<30)
 {
   correct_password_html();
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
//    correct_password_html();
    delay(500);
    token = 1;
    counter = 0;
    user_wifi = "";
    user_password="";
 }
}
       
//------------SETTING UP FOR ALL FUNCTIONS----------------
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
	WiFi.softAP(ssid, password);  // this is the wifi and password of the device itself
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
  WiFi.mode(WIFI_AP_STA);
  //Server hangling, setup the html...
  server.on("/", handleRoot);
  server.on("/submit", handleSubmit); //Handling for
  server.begin();
  
  Serial.println("HTTP server started"); 
  
}

//------------LOOP----------------
void loop() {
	server.handleClient();
  if(counter>30){
      WiFi.softAP(ssid, password);  // this is the wifi and password of the device itself
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      wrong_password_html();
      user_wifi = "";
      user_password="";
      counter = 0;
  }
  if(token ==1){ Data();}  //using token after connected to the server
}

//------------FUNCTIONS FOR SENSOR VALUES----------------
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

//-------------SENDING SENSOR DATA TO SERVER--------------
void Data(){
 // Sensor_OLED();
  sensor = wet_dry(); // getting  data from sensor
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
  Serial.print("Humidity:");
  Serial.println(String(percent())+"%");
  
  //   String url = "/notify?device_id=test&humidity=0.7";
  String url = "/notify?device_id=1A-FE-34-06-97-08";
  url += "&humidity=";
  url += String(humid());
  
  Serial.println("Frame send to server: GET " + url + " HTTP/1.1");
  Serial.println("<------------Begining of receiving frame from server------------->");
  
 //--------FRAME OF SENDING DATA TO THE SERVER------------------
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");


 //--------CHECKING THE RESPONSE FROM THE SERVER----------------         
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 //---------EXPORT SERVER'S RESPONSE TO EXAMINE------------------ 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("<------------Ending of receiving frame from server------------->");
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
}

