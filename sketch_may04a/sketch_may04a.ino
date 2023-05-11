/* ENERGYGUARD */

#include <WiFiNINA.h>
#include "Firebase_Arduino_WiFiNINA.h"

/*------------------Device info-------------------------*/
#define ARDU "ARDUINO"
String mac = "ec:62:60:81:14:a8";

// IO ports
#define LED_PIN 2
#define ANALOGRESISTORPIN A1
#define ANALOGREFPIN A2
#define ANALOGPIN A3
/*------------------------------------------------------*/

/*-------------------constants--------------------------*/
const double r1 = 12.0; // change to used resistor value.
const double analogConstant = 1023.0; // analog input value 0-1024
const double voltConstant = 3.3; // arduino board voltage
const int wattMultiplier = 10000;
/*------------------------------------------------------*/

/*-------------------WiFi info--------------------------*/
#define WIFI_SSID "Bjornbar"     // your network SSID (name) /Bjornbar /HamDat
#define WIFI_PASSWORD "}63J998w"   // your network password  /}63J998w /837576Z!

int status = WL_IDLE_STATUS;
//IPAddress ip(192,168,137,17);  // Server
//char server[]="localhost";  // remote server we will connect to
int port = 8888;
/*------------------------------------------------------*/

/*-------------------Firebase info----------------------*/
#define FIREBASE_HOST "energyguard-4da4a-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyDHlksusvmxHY69if-Jf1HEZANvr98ZJas"
FirebaseData firebaseData;
String folder = "/mac_address/";
/*------------------------------------------------------*/

/*------------Variables for energy usage----------------*/
double current = 0; // current calc variable
double watt = 0; // watt calc variable
/*------------------------------------------------------*/

/*------------------Protocall info----------------------*/
String protocoll = "";
char buffr[10];
/*------------------------------------------------------*/

/*------------------Server info-------------------------*/
WiFiServer server(port);
/*------------------------------------------------------*/

void setup() {

  // Initialize arduino output.
  pinMode(LED_PIN, OUTPUT);
  // connect to wifi.
  connectToWiFi();
  // Initialize Firebase.
  firebaseConnect();
  //start server.
  server.begin();
}

void loop() {

  // checks if arduino is connected to wifi.
  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }
  
  int shutDownVariable;
  int c_available;
  
  // Calculates the current and stores it in current.
  current = currentCalc(analogRead(ANALOGREFPIN), analogRead(ANALOGPIN));

  // Calculates the watt and stores it in watt.
  watt = wattCalc(current, analogRead(ANALOGPIN));

  /*----Server variables----*/
  WiFiClient client = server.available();
  char protocollBuffer[12];
  /*------------------------*/

  /*------Communication-------*/
  if(client){
    
    if(client.connected()){
      
      c_available = client.available();
      
      // Read the data stream if data on
      if(c_available != -1){
        client.readBytes(protocollBuffer, c_available);
      }

      
      shutDownVariable = protocollRecieve(protocollBuffer);
      client.flush();

      deviceStatus(shutDownVariable);
      
      // int to char[]
      itoa(watt, buffr, 10);
      protocollSend(buffr, shutDownVariable);
      
      server.println(protocoll);
      protocoll = "";
      
    }
    client.stop();
  }
  /*----Communication Ends----*/
}

// connects arudino to wifi.
void connectToWiFi() {

  int status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Wait for the WiFi connection to be established
  while (WiFi.status() != WL_CONNECTED) {
    // Connect to WiFi
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
  }
}

// connects to firebase and sends IP.
void firebaseConnect(){
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  IPAddress ip = WiFi.localIP();
  
  String ipToString = String(ip[0])+ "." + String(ip[1]) + "." +String(ip[2])+ "." +String(ip[3]);
  String macfolder = "mac_address"; //folder + mac;
  String ipFolder = folder + ipToString;
  
  Firebase.setString(firebaseData, "users/rh9hxkJsnhRVqWYZfqUi6mEWAAx1/"+ macfolder+"/"+mac, ipToString);
  // String subfolder = macfolder +"/"+mac;
  // Firebase.setString(firebaseData, subfolder, ipToString);
  
  /* // for testing
  if(Firebase.pushString(firebaseData, macfolder, mac)){
    //Serial.println(mac);
  }
  if(Firebase.pushString(firebaseData, ipFolder, ipToString)){
    //Serial.println(ipToString);
  }
  */
}


// Splits up the data for the protocoll.
int protocollRecieve(char *messageRecieved){

  char *token;
  char str1[3], str2[2], str3[6];
  int shutdI = 0;

  token = strtok(messageRecieved, "?");  // Get the first token
  strcpy(str1, token);       // Copy the token to str1
  token = strtok(NULL, "?"); // Get the second token
  strcpy(str2, token);       // Copy the token to str2
  token = strtok(NULL, "?"); // Get the third token
  String hello(str1);
  String shutd(str2);
  
  //for error handling
  /*
  if(hello == "OK"){

  }
  
  if(shutd == "0"){
    Serial.println("0");
  }else if(shutd == "1"){
    Serial.println("1");
  }else{
    //Serial.println("NOT VIABLE DATA!");
  }
  */
  return (shutdI = shutd.toInt());
}

// Puts together the protocoll structure.
void protocollSend(char *data, int shutD){
  /* 
     Order ex:
     ARDUINO?MAC?SHUTDOWN?DATA?
  */

  // convert to string
  String vshutD = String(shutD);
  String vdata(data);
  
  // Puts together the protocoll in a string
  protocoll = ARDU;
  protocoll = protocoll + "?";
  // MAC?
  protocoll = protocoll + mac;
  protocoll = protocoll + "?";
  // shutdown?
  protocoll = protocoll + vshutD;
  protocoll = protocoll + "?";
  // data?
  protocoll = protocoll + vdata;
  protocoll = protocoll + "?";
}

// On/Off function for connected device.
int deviceStatus(int deviceStatus){
  
    int statusToDatabase = 0;
    if(deviceStatus == 1){
      digitalWrite(LED_PIN, HIGH);
      statusToDatabase = 1;
    }else if(deviceStatus == 0){
      digitalWrite(LED_PIN, LOW);
      statusToDatabase = 0;
    }
    return statusToDatabase;
}

// Calculates the current.
double currentCalc(double voltRef, double volt) {
  
  double value = 0;
  value = ((((voltRef - volt) * voltConstant) / analogConstant) / r1);
  return value;
}

// Calculates the watts used.
int wattCalc(double curr, double volt) {

  double value = 0;
  value = (curr * ((volt * voltConstant) / analogConstant)) * wattMultiplier;
  return value;
}
