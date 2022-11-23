#include <Arduino.h>
#include <SoftwareSerial.h>

String MASTER_PORT = "32";
String SLAVE_PORT = "00";  // broadcast for future applications

char incomingByte;
String command;
boolean messageCompleted = false;
boolean newMessage = false;
int approve = 0;
//String newCard = "";
String cardID = "";
String latestPORT = "";
const int maxCardNumber = 5;
String cards[maxCardNumber] = {"00 D1 1B 83", "CC CC A3 16", "", "", ""};

//Oda mi
// start - [
// port_num - where / back to the correct slave
// ok/no - 1/0
// end - ]
//example: [311]

//Ide az
// start - [
// port_num - hova
// RFID - ID
// end - ]
//example: [FF01xxxxxxxx]

// RS485
#define RX A0
#define TX A1
#define DE A2
#define RE A3
SoftwareSerial rs485(RX, TX); // RX, TX

void setup()
{
  // RS485
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);
  pinMode(RE, OUTPUT);
  digitalWrite(RE, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW

  Serial.begin(9600);
  rs485.begin(19200);
}

int RFID_checkCard(String checkID)
{
  int message = 0;
  for (int i = 0; i <= maxCardNumber; i++)
  {
    if (checkID == cards[i])
    {
      message = 1;
      break;
    }
  }
  return message;
}

void Approve_send(String Port,int result)
{
  digitalWrite(DE, HIGH);
  // start - [
  // port_num - where / back to the correct slave
  // ok/no - 1/0
  // end - ]
  rs485.println("[" + Port + result +"]");
  rs485.flush(); //wait for complete the message
  digitalWrite(DE, LOW);
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

  // TODO: if(messageCompleted)
  
  if (messageCompleted)
  { 
    Serial.println();
    Serial.println("The received full-command: " + command.substring(1));
    Serial.println();

    if (command.charAt(0) == MASTER_PORT[0] && command.charAt(1) == MASTER_PORT[1])
    {
      latestPORT = command.substring(2,4);
      //Print slave port
      Serial.println("Slave port: "+latestPORT);
      Serial.println();
      {
        //Ellenorzes a id-ra
        Serial.println("CardID: " + command.substring(4));
        Serial.println();
        cardID = command.substring(4);
        approve = RFID_checkCard(cardID);

        if(approve == 1){ //has accesss -> approved
          Serial.println();
          Serial.println("The following card with: " + cardID + " was approved. (" + approve + ")");
        }
        else{ //denied
          Serial.println("The following card with: " + cardID + " was declined. (" + approve + ")");
        }
      }
      Approve_send(latestPORT,approve);
    }
    cardID="";
    approve=0;
    command = "";
    messageCompleted = false;
  }
}

void rs485Communication()
{
  if (Serial.available())
  {
    incomingByte = Serial.read();
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
  if (messageCompleted)
  {
    if (command.substring(1, 3) != MASTER_PORT)
    {
      digitalWrite(DE, HIGH);
      rs485.print(command + "]");
    }
    // else
    // {
    //   Serial.println(newCard);
    //   newCard = "";
    // }
    command = "";
    messageCompleted = false;
  }
  digitalWrite(DE, LOW);
}

void loop()
{

  serialCommunication();
  rs485Communication();

}