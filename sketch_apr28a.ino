#include <SPI.h>
#include <WiFiNINA.h>

/*------------------Device info-------------------------*/
#define ARDU "ARDUINO"
char mac[] = "0xec,0x62,0x60,0x81,0x14,0xa8";

// IO ports
#define LED_PIN 2
#define ANALOGRESISTORPIN A1
#define ANALOGREFPIN A2
#define ANALOGPIN A3
/*------------------------------------------------------*/

/*-------------------constants--------------------------*/
const int r1 = 6; // change to used resistor value.
const int analogConstant = 1024; // analog input value 0-1024
const double voltConstant = 3.3; // arduino board voltage
/*------------------------------------------------------*/

/*-------------------WiFi info--------------------------*/
#define WIFI_SSID "HamDat"     // your network SSID (name) /Bjornbar
#define WIFI_PASSWORD "837576Z!"   // your network password  /}63J998w

int status = WL_IDLE_STATUS;
IPAddress ip(192,168,137,99);  // Server
//char server[]="localhost";  // remote server we will connect to
int port = 8888;
/*------------------------------------------------------*/

/*------------Variables for energy usage----------------*/
double current = 0;
double watt = 0;
/*------------------------------------------------------*/

// maybe not needed
/*-------------------Timer Info-------------------------*/
unsigned long currentTime = 0;
unsigned long previousTime = 0;
const long interval = 1000; // 1 second in milliseconds
/*------------------------------------------------------*/

/*------------------Protocall info----------------------*/
String protocoll = "";
char buffr[10];
String timerShutdown;
/*------------------------------------------------------*/

/*------------------Server info-------------------------*/
WiFiServer server(port);
/*------------------------------------------------------*/

void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  int status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WIFI_SSID);

  // Waits for WiFi to connect
  while(status != WL_CONNECTED){
    Serial.print(".");
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
    
  }
  
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);

  server.begin();

}
void loop() {
  // put your main code here, to run repeatedly:
  currentTime = millis();
  current = currentCalc(analogRead(ANALOGREFPIN), analogRead(ANALOGRESISTORPIN));

  // stores the calculated watt
  watt = wattCalc(current, analogRead(ANALOGPIN));

  // Server variables
  WiFiClient client = server.available();
  char buf[12];
  
  if(client){
    
    if(client.connected()){
      
      int c_available = client.available();
      
      // Read the data stream if data on
      if(c_available != -1){
        client.readBytes(buf, c_available);
      }

      
      int test = protocollRecieve(buf);
      
      itoa(watt, buffr, 10);
      protocollSend(buffr, test);
      
      server.println(protocoll);
      Serial.println();
      
    }
    client.stop();
  }
}

// Splits up the data for the protocoll
int protocollRecieve(char *data){

  char *token;
  char str1[3], str2[2], str3[6];
  String str11;
  String str12;
  String str13;
  int shutdI = 0;

  token = strtok(data, "?");  // Get the first token
  strcpy(str1, token);       // Copy the token to str1
  token = strtok(NULL, "?"); // Get the second token
  strcpy(str2, token);       // Copy the token to str2
  token = strtok(NULL, "?"); // Get the third token
  strcpy(str3, token);       // Copy the token to str3
  String hello(str1);
  String shutd(str2);
  String timer(str3);

  shutdI = shutd.toInt();
  
  Serial.print("answer:");
  Serial.print(hello);
  Serial.print(" shutdown:");
  Serial.print(shutd);
  Serial.print(" timer:");
  Serial.println(timer);
  return shutdI;
}

// Puts together the protocoll structure
void protocollSend(char *data, int test){
  
  protocoll = ARDU;
  protocoll = protocoll + "?";
  String vmac(mac);
  String vtest = String(test);
  protocoll.concat(vmac);
  protocoll = protocoll + "?";
  protocoll = protocoll + vtest;
  protocoll = protocoll + "?";
  String vdata(data);
  protocoll.concat(vdata);
}

// Calculates the current
double currentCalc(double voltRef, double volt) {
  
  double value = 0;
  value = ((((voltRef - volt) * voltConstant) / analogConstant) / r1);
  return value;
}

// Calculates the watts used.
double wattCalc(double curr, double volt) {

  double value = 0;
  value = (curr * ((volt * voltConstant) / analogConstant)) * 10000;
  return value;
}
