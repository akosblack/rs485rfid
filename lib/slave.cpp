#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

// RS485
#define RX A0
#define TX A1
#define DE A2
#define RE A3
SoftwareSerial rs485(RX, TX); // RX, TX

// Oda mis
//  start - [
//  port_num - hova
//  RFID - ID
//  end - ]
// example: [FF01xxxxxxxx]
// FF01xxxxxxxx

// Mastertől mi
//  start - [
//  port_num - where / back to the correct slave
//  ok/no - 1/0
//  end - ]
// example: [311]

String to_check_id = "";

// RFID
#define SS_PIN 10 // rfidsda
#define RST_PIN 9 // rfidrst

// LEDs
#define RED 2   // redled
#define GREEN 3 // greenled

String MASTER_PORT = "32"; // master id, the last
String SLAVE_PORT = "00";  // broadcast for future applications
String PORT = "31";        // own id

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

// For RS485 and serial communication
char incomingByte;
String command;
boolean messageCompleted = false;
boolean newMessage = false;
String card = "";
String lastestCard = "";
boolean acces = false;
String lastestResult = "";

void setup()
{
  // RS485
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);
  pinMode(RE, OUTPUT);
  digitalWrite(RE, LOW);

  // LEDs
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  pinMode(GREEN, OUTPUT);
  digitalWrite(GREEN, LOW);
  pinMode(RED, OUTPUT);
  digitalWrite(RED, LOW);

  // Serial
  Serial.begin(9600);
  rs485.begin(19200);

  // RFID card
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  Serial.println("////");
  Serial.println("The Terminal is up and running!");
  Serial.println();
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

// When accessing the door, blink a led accordingly
void blink(int led) // which led do you want to active
{
  digitalWrite(led, HIGH);
  delay(500); // 0.5sec
  digitalWrite(led, LOW);
  delay(200); // 0.2sec
  digitalWrite(led, HIGH);
  delay(500); // 0.5sec
  digitalWrite(led, LOW);
  return;
}

void RFID_send(String slave_port, String ID)
{
  lastestCard = ID;
  digitalWrite(DE, HIGH);  
  rs485.println("[" + MASTER_PORT + slave_port + ID +  "]");
  rs485.flush(); // wait for complete the message
  digitalWrite(DE, LOW);
  return;
}

// void RFID_resend(String slave_port)
// {
//   digitalWrite(DE, HIGH);  
//   rs485.println("[" + MASTER_PORT + slave_port + 'r' +  "]");
//   rs485.flush(); // wait for complete the message
//   digitalWrite(DE, LOW);  
//   return;
// }

void deceider(String result){
  if (result == "1")
      {
        Serial.println(" Authorized access");
        Serial.println();
        digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
        digitalWrite(GREEN, HIGH);
        delay(1500); // 1.5sec
        digitalWrite(GREEN, LOW);
      }
      else if(result == "0")
      {
        Serial.println(" Access denied");
        digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
        blink(RED);
      }
      lastestResult = result;
  return;    
}

void RFID_read()
{ 
  //RFID
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  { 
    //Serial.println("UJRA NYIT A GYÁR HÁT ZÁRJON BE A NYÁR");
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) // beolvasas
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  to_check_id = content.substring(1);  

  // Send to master
  Serial.println("ID: "+to_check_id+" Latest sent: "+lastestCard);
  if(to_check_id != lastestCard){
    to_check_id.trim();
    RFID_send(PORT, to_check_id);
    delay(300);
  }
  //Or blink the correct led/decide again
  else{
    deceider(lastestResult);
  }
  // else{
  //   RFID_resend(PORT);
  //    delay(2000); // 2sec
  // }
  // if ((content.substring(1) == "00 D1 1B 83") || (content.substring(1) == "CC CC A3 16")) // change here the UID of the card/cards that you want to give access
  // {
  //   Serial.println("Authorized access");
  //   Serial.println();
  //   digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  //   digitalWrite(GREEN, HIGH);
  //   delay(1500); // 1.5sec
  //   digitalWrite(GREEN, LOW);
  // }
  // else
  // {
  //   Serial.println(" Access denied");
  //   digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  //   blink(RED);
  // }

  return;
}

void serialCommunication()
{
  if (rs485.available())
  {
    incomingByte = rs485.read();
    if (incomingByte == ']')
    {
      messageCompleted = true;
      newMessage = false;
    }
    else if (incomingByte == '[')
    {
      newMessage = true;
    }

    if (newMessage)
    {
      command.concat(incomingByte);
    }
  }

  // TODO: access-t lekezelni ITT

  if (messageCompleted)
  {
    Serial.println();
    Serial.println("The received full-command: " + command.substring(1));
    //Serial.println("The received port: " + command.substring(1,3));
    //Serial.println("The received result: " + command.substring(3));
    Serial.println();

    if (command.substring(1,3) == PORT)
    {
      String result = command.substring(3);
      //Serial.println();
      //Serial.println("The received result: " + result);
      //Serial.println();
      deceider(result);
    }

    command = "";
    messageCompleted = false;
  }
}

void loop()
{
  serialCommunication();
  RFID_read();
}