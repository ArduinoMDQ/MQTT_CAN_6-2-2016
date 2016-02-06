#include <mcp_can.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte server[] = { 192, 168, 1, 107 };
byte ip[]     = { 192, 168, 1,  177};


// Set what PINs our Led's are connected to
int redPin = 5;                
int greenPin = 6;
int bluePin = 7;
int ledArdu= 8;

byte on[2] = "1";
byte off[2] = "0";

/////
const int SPI_CS_PIN = 9;
const int ledHIGH    = 1;
const int ledLOW     = 0;
const int LED=13;
const char ID_TX=0x80;
MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

/////

EthernetClient ethClient;

PubSubClient client(server, 1883, callback, ethClient);

// handles messages that are returned from the broker on our subscribed channel
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("New message from broker on topic:");
  Serial.println(topic);
  
  Serial.print("Payload:");
  Serial.write(payload, length);
  
  
  // This will blink our green LED
//  blink(greenPin);
    blink(ledArdu);
  
  // Check and see if our payload matches our simple trigger test
  if ((length == 1) & (memcmp(payload, on, 1) == 0) )
  {
    Relay(bluePin,1);
  }
  if ((length == 1) & (memcmp(payload, off, 1) == 0) )
  {
  Relay(bluePin,0);
  }

/*
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  client.publish("outTopic", p, length);
  // Free the memory
  free(p);
*/
}


void setup()
{
  
  
  // Open serial communications
  Serial.begin(9600);
  
  // Setup our Leds
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(ledArdu, OUTPUT);
  
  
  //////////
  START_INIT:

    if(CAN_OK == CAN.begin(CAN_50KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS inicio ok!");
    }
    else
    {
        Serial.println("CAN BUS fallo al iniciar");
        Serial.println("Reinicio CAN BUS otra vez");
        delay(2000);
        goto START_INIT;
    }

  
  ///////////
  
  Ethernet.begin(mac, ip);
   
  Serial.print("My address:");
  Serial.println(Ethernet.localIP());
 
   if (client.connect("arduinoClient")) {
      // Good, we connected turn on the red led
    digitalWrite(redPin, HIGH);
   // Publish a message to the status topic
   // client.publish("led","1");
    
    // Listen for messages on the control topic
    client.subscribe("led");
  }
}


unsigned char stmpON[8] = {ledHIGH, 1, 1, 3, ledLOW, 5, 6, 7};
unsigned char stmpOFF[8] = {ledHIGH, 1, 0, 3, ledLOW, 5, 6, 7};


void loop()
{
  client.loop();
}


// Anything with flashing lights.
void blink(int targetLed) 
{
 static boolean led = HIGH;
 static int count = 0;
 
 Serial.print("Starting to blink...");
 
 while (count < 2) 
 {
   digitalWrite(targetLed, led);
   
   count++;
   
   if (led == HIGH) 
   {
     led = LOW;
       CAN.sendMsgBuf(ID_TX,0, 8, stmpON);
   } 
   else 
   {
     led = HIGH;
       CAN.sendMsgBuf(ID_TX,0, 8, stmpOFF);
   }
   
   
   delay(100);
 }
 
 count = 0;
 
 Serial.print("done.");
}



// Anything with flashing lights.
void Relay(int pin, int estado) 
{
 
 
 switch(estado){
   case 0: digitalWrite(pin, LOW);
   client.publish("led","Led apagado ");
   break;
   case 1: digitalWrite(pin, HIGH);
    client.publish("led","Led encendid");
 
 break;
 default:
  client.publish("led","Error de Comando");
 
 break;
 }
 
   
 Serial.print("done.");
}

