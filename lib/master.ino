#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PCD8544.h>
 PCD8544 lcd;

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
String cards[maxCardNumber] = {"00 D1 1B 83", "", "", "", ""};

// RS485
#define RX D1
#define TX A1
#define DE A2
#define RE D2
SoftwareSerial rs485(RX, TX); // RX, TX

// RFID
#define SS_PIN 10 // rfidsda
#define RST_PIN 9 // rfidrst

void setup()
{
  // LCD screen - Nokia 5110 
  lcd.begin(84, 48);
  // RS485
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);
  pinMode(RE, OUTPUT);
  digitalWrite(RE, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW

  Serial.begin(9600);
  rs485.begin(19200);

  Serial.println("Master is running!");
  
}

int RFID_checkCard(String checkID)
{
  Serial.println("Approving...");
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
  Serial.println("[" + Port + result +"]");
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


  //TODO!!!! ez nem így van 
  
    if (command.substring(1,3) == MASTER_PORT)
    {
      latestPORT = command.substring(3,5);
      //Print slave port
      Serial.println("Slave port: "+latestPORT);
      Serial.println();
      {
        //Ellenorzes a id-ra
        Serial.println("CardID: " + command.substring(5));
        Serial.println();
        cardID = command.substring(5);
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
  lcd.setCursor(0, 1);
  lcd.print("Waiting for Card to read...");
  serialCommunication();
  rs485Communication();

}
