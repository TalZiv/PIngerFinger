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


const long delayMS = 30L * 1000L; // delay between successive pings (60 * 1000 = 60 seconds) the L is for the long datatype
long delaySEC = delayMS / 1000; // delay between successive pings (60 * 1000 = 60 seconds) the L is for the long datatype

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
//const unsigned long postingInterval = 30L * 1000L; // delay between updates, in milliseconds

int HowManyFailes = 50;
#define ledPing 2
#define ledOk 3
#define ledFail 4
#define relayPin 5

void ClientStart()
{
  digitalWrite(ledPing, LOW);
}

void ClientEnd()
{
  digitalWrite(ledPing, HIGH);
}

void ClientSuccess()
{
  digitalWrite(ledFail, HIGH);
  digitalWrite(ledOk, LOW);
  Serial.println("Client Connected OK...");
  RebootEnabled = 1;
  PreBootCounter = 0;
}

void ClientFail()
{
  digitalWrite(ledFail, LOW);
  digitalWrite(ledOk, HIGH);
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
      digitalWrite(relayPin, LOW);
      delay(1000);
      digitalWrite(relayPin, HIGH);
      RebootEnabled = 0;
    }else{
      Serial.println("Nas Still Down... No Reboot Needed... Waiting...");
    }
  }
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
  
  // initialising, turn all LEDs on
  digitalWrite(ledFail, LOW);
  digitalWrite(ledOk, LOW);
  digitalWrite(ledPing, LOW);

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
  while(1){
    noInterrupts(); // Call to disable interrupts
    EthernetClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
      if (client.available()) {
      char c = client.read();
      Serial.write(c);
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the http request has ended,
      // so you can send a reply
      if (c == '\n' && currentLineIsBlank) {
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        // output the value of each analog input pin
        client.println("The Pinger Finger - Tal Ziv");
        client.println("<br />");
        client.print("My IP: ");
        client.println(Ethernet.localIP());
        client.println("<br />");
        client.print("Ping Destination: ");
        client.println("192.168.1.251");
        client.println("<br />");
        client.print("Web GET DealyInSec: ");
        client.println(delaySEC);
        client.println("<br />");
        client.print("How Many Failes takes to do a Reboot: ");
        client.println(HowManyFailes);
        client.println("<br />");
        client.print("PreBootCounter: ");
        client.println(PreBootCounter);
        client.println("<br />");
        client.print("RebootEnabled: ");
        client.println(RebootEnabled);;
        client.println("<br />");
        if(PreBootCounter >= HowManyFailes && RebootEnabled == 0){
          client.println("NAS was Rebooted<br />NAS Still Down...<br />No Need To Reboot Again...<br />Waiting...");
          client.println("<br />");
        }
        client.println("</html>");
        break;
      }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    }
    interrupts(); // Call to enable interrupts
      // checks if thread should run
    if(ClientThread.shouldRun()){
      ClientThread.run();
    }
  }
}


