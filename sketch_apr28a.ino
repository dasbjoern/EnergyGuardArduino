//#include <SPI.h>
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
const double r1 = 6.5; // change to used resistor value.
const double analogConstant = 1023.0; // analog input value 0-1024
const double voltConstant = 3.3; // arduino board voltage
const int wattMultiplier = 10000;
/*------------------------------------------------------*/

/*-------------------WiFi info--------------------------*/
#define WIFI_SSID "HamDat"     // your network SSID (name) /Bjornbar
#define WIFI_PASSWORD "837576Z!"   // your network password  /}63J998w

int status = WL_IDLE_STATUS;
IPAddress ip(192,168,137,17);  // Server
//char server[]="localhost";  // remote server we will connect to
int port = 8888;
/*------------------------------------------------------*/

/*------------Variables for energy usage----------------*/
double current = 0; // current calc variable
double watt = 0; // watt calc variable
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
  //Serial.begin(9600);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  */
  int status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  //Serial.print("Attempting to connect to SSID: ");
  //Serial.println(WIFI_SSID);

  // Waits for WiFi to connect
  while(status != WL_CONNECTED){
    //Serial.print(".");
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
    
  }
  /*
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("on IP Address: ");
  Serial.println(WiFi.localIP());
  */
  server.begin();
}

void loop() {

  // Calculates the current and stores it in current
  current = currentCalc(analogRead(ANALOGREFPIN), analogRead(ANALOGRESISTORPIN));

  // Calculates the watt and stores it in watt
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

      
      int shutDInt = protocollRecieve(buf);
      client.flush();

      deviceStatus(shutDInt);
      
      // int to char[]
      itoa(watt, buffr, 10);
      protocollSend(buffr, shutDInt);
      
      server.println(protocoll);
      //Serial.println();
      protocoll = "";
      
    }
    client.stop();
  }
}

// Splits up the data for the protocoll
int protocollRecieve(char *shutdD){

  char *token;
  char str1[3], str2[2], str3[6];
  int shutdI = 0;

  token = strtok(shutdD, "?");  // Get the first token
  strcpy(str1, token);       // Copy the token to str1
  token = strtok(NULL, "?"); // Get the second token
  strcpy(str2, token);       // Copy the token to str2
  token = strtok(NULL, "?"); // Get the third token
  String hello(str1);
  String shutd(str2);

  if(hello == "OK"){
    /* --For debugging-- */
    /*
    Serial.print("Answer: ");
    Serial.println(hello);
    */
    /* ------------------*/
  }
  
  if(shutd == "0"){
    /* --For debugging-- */
    /*
    Serial.print("Device OFF:");
    Serial.println(shutd);
    */
    /* ------------------*/
  }else if(shutd == "1"){
    /* --For debugging-- */
    /*
    Serial.print("Device ON:");
    Serial.println(shutd);
    */
    /* ------------------*/
  }else{
    //Serial.println("NOT VIABLE DATA!");
  }
  
  return (shutdI = shutd.toInt());
}

// Puts together the protocoll structure
void protocollSend(char *data, int shutD){
  // Order ex:
  // ARDUINO?MAC?SHUTDOWN?DATA?

  // convert to string
  String vmac(mac);
  String vshutD = String(shutD);
  String vdata(data);
  
  // Puts together the protocoll in a string
  protocoll = ARDU;
  protocoll = protocoll + "?";
  // MAC?
  protocoll = protocoll + vmac;
  protocoll = protocoll + "?";
  // shutdown?
  protocoll = protocoll + vshutD;
  protocoll = protocoll + "?";
  // data?
  protocoll = protocoll + vdata;
  protocoll = protocoll + "?";
}

// On/Off function for connected device
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

// Calculates the current
double currentCalc(double voltRef, double volt) {
  
  double value = 0;
  value = ((((voltRef - volt) * voltConstant) / analogConstant) / r1);
  return value;
}

// Calculates the watts used.
int wattCalc(double curr, double volt) {

  int value = 0;
  value = (curr * ((volt * voltConstant) / analogConstant)) * wattMultiplier;
  return value;
}
