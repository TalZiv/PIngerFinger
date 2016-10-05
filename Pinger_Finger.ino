/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 */

#include <SPI.h>
#include <Ethernet.h>
#include <Thread.h>

int PreBootCounter = 0;
int RebootEnabled = 1;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // mac address for ethernet shield
// the dns server ip
IPAddress dnServer(8, 8, 8, 8);
// the router's gateway address:
IPAddress gateway(10, 1, 1, 254);
// the subnet:
IPAddress subnet(255, 255, 255, 0);
//the IP address is dependent on your network
IPAddress ip(10, 1, 1, 100);
//The Ip To Ping
byte ServerAddr[] = {192,168,1,251}; // ip address to ping
//char ServerAddr[] = "www.arduino.cc";
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;
String readString;

const long delayMS = 30L * 1000L; // delay between successive pings (60 * 1000 = 60 seconds) the L is for the long datatype
long delaySEC = delayMS / 1000; // delay between successive pings (60 * 1000 = 60 seconds) the L is for the long datatype

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
//const unsigned long postingInterval = 30L * 1000L; // delay between updates, in milliseconds

int HowManyFailes = 50;
#define ledPing 2
#define ledOk 3
#define ledFail 4
#define relayPin 5
#define on LOW
#define off HIGH

void ClientStart()
{
  digitalWrite(ledPing, on);
}

void ClientEnd()
{
  digitalWrite(ledPing, off);
}

void ClientSuccess()
{
  digitalWrite(ledFail, off);
  digitalWrite(ledOk, on);
  Serial.println("Client Connected OK...");
  RebootEnabled = 1;
  PreBootCounter = 0;
}

void ClientFail()
{
  digitalWrite(ledFail, on);
  digitalWrite(ledOk, off);
  Serial.println("Client Connection Failed...");
  PreBootCounter++;
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(ServerAddr, 8003)) {
    Serial.println("connected...");
    // send the HTTP GET request:
    client.println("GET /index.html HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
    ClientSuccess();    
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    ClientFail();
  }
}

//My Thread
Thread ClientThread = Thread();

// callback for myThread
void ClientThreadCallback(){
  Serial.println("ClientThread\tcallback");
    ClientStart();
    // if there's incoming data from the net connection.
    // send it out the serial port.  This is for debugging
    // purposes only:
    //if (client.available()) {
    //  char c = client.read();
    //  Serial.write(c);
    //}
    
    // if ten seconds have passed since your last connection,
    // then connect again and send data:
    if (millis() - lastConnectionTime > delayMS) {
      httpRequest();
    }
  delay(250);
  ClientEnd();
 
  Serial.print("PreBootCounter: ");
  Serial.println(PreBootCounter);
  Serial.print("RebootEnabled: ");
  Serial.println(RebootEnabled);
  
  if(PreBootCounter >= HowManyFailes){
    if(RebootEnabled == 1){
      Serial.println("Rebooting NAS...");
      digitalWrite(relayPin, on);
      delay(1000);
      digitalWrite(relayPin, off);
      RebootEnabled = 0;
    }else{
      Serial.println("Nas Still Down... No Reboot Needed... Waiting...");
    }
  }
}

void Blink(int DigitalPin){
  digitalWrite(DigitalPin, on);
  delay(1000);
  digitalWrite(DigitalPin, off);

}

void setup()
{
  // Configure PingThread
  ClientThread.onRun(ClientThreadCallback);
  ClientThread.setInterval(delayMS);
 
  pinMode(ledPing, OUTPUT);
  pinMode(ledOk, OUTPUT);
  pinMode(ledFail, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, off);
  
  // blink  all LEDs on
  Blink(ledFail);
  Blink(ledPing);
  Blink(ledOk);
  digitalWrite(ledOk, on);

  // start serial port:
  Serial.begin(9600);
  Serial.println("Starting ethernet connection");

  // start Ethernet
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Pinger is at ");
  Serial.println(Ethernet.localIP());
  

}

void loop()
{
    noInterrupts(); // Call to disable interrupts
     EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
     
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
         }

         //if HTTP request has ended
         if (c == '\n') {          
          Serial.println(readString); //print to serial monitor for debuging
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println(F("Content-Type: text/html"));
          client.println();     
          client.println(F("<HTML>"));
          client.println(F("<HEAD>"));
          client.println(F("<meta name='apple-mobile-web-app-capable' content='yes' />"));
          client.println(F("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />"));
          client.println(F("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />"));
          client.println(F("<TITLE>The Pinger Finger - Tal Ziv</TITLE>"));
          client.println(F("</HEAD>"));
          client.println(F("<H1>The Pinger Finger v1</H1>"));
          client.println(F("<hr />"));
          client.println(F("<H2>Status And details</H2>"));
          client.println(F("<br />"));
          client.print(F("My IP: "));
          client.println(Ethernet.localIP());
          client.println(F("<br />Ping Destination: 192.168.1.251 <br />"));
          client.print(F("Web GET DealyInSec: "));
          client.println(delaySEC);
          client.print(F("<br />How Many Failes takes to do a Reboot: "));
          client.println(HowManyFailes);
          client.print(F("<br />PreBootCounter: "));
          client.println(PreBootCounter);
          client.print(F("<br />RebootEnabled: "));
          client.println(RebootEnabled);
          client.println(F("<br /><br />"));
          if(PreBootCounter >= HowManyFailes && RebootEnabled == 0){
            client.println(F("NAS was Rebooted<br />NAS Still Down...<br />No Need To Reboot Again...<br />Waiting...<br /><br />"));
          }
          client.println(F("<a href=\"/?button1on\"\">Hold RESET Button</a>"));
          client.println(F("<a href=\"/?button1off\"\">Release RESET Button</a><br /><br /><br />"));
          client.println(F("<p>Created by Tal Ziv.</p><br />"));  
          client.println(F("</BODY></HTML>"));
    
          delay(1);
          //stopping client
          client.stop();
          //controls the Arduino if you press the buttons
          if (readString.indexOf("?button1on") >0){
              digitalWrite(relayPin, on);
          }
          if (readString.indexOf("?button1off") >0){
              digitalWrite(relayPin, off);
          }
          //clearing string for next read
          readString="";  
         }
       }
    }
}
    interrupts(); // Call to enable interrupts
      // checks if thread should run
    if(ClientThread.shouldRun()){
      ClientThread.run();
    }
  }


