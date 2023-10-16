/*
 * Author: Matthew Vecchio
 * Ticket Printer - recieves data from serial port after a 'handshake' message is sent and recieved using a hand shake
 * Once connection is established sending messages can be done to print various tickets. Since the p 
 * 
 *  A = player 1  
 *  B = player 2
 *  C = player 3
 *  D = player 4
 *  
 *  a - d + num   = print tickets
 *  w + num       = set pulse width (Default is 60)
 *  s + num       = Set pulse spacing (default is 240)
 *  x + num       = set Skip Time
 *  'Skip'        = wait 5 seconds
 *  'Clear'       = clears all tickets
 *  
 *  Because the program has to wait for serial messages this can effect the speed of printing tickets - so when the connection 
 *  application doesn't have any tickets to add it should send the 'Skip' command to speed up the printing process.
 *  
 */


#define BAUD 9600                       // the baud rate

#define DELAY_REV_MSG_TIME 500          // the time between the program will wait before it checks its serial messages 
#define SERIAL_READ_TIMEOUT 3000        // how long in milliseconds the serial connection waits for a message to be received 
#define LED_SUSTAIN 500                 // how long the RX and TX LEDS stay on

#define RXLED 17                        // The RX LED has a defined Arduino pin
#define TXLED 30                        // The TX LED has a defined Arduino pin
  
#define PLAYER_1A_TICKET_PIN 9          // Pin for player 1A's ticket printer
#define PLAYER_2B_TICKET_PIN 6          // Pin for player 2B's ticket printer
#define PLAYER_3C_TICKET_PIN 7          // Pin for player 3C's ticket printer
#define PLAYER_4D_TICKET_PIN 8          // Pin for player 4D's ticket printer

#define INT_MAX 32767                  

String handShakeMessage = "Handshake"; // the hand Shake Message to recieve 
String receivedString = " ";
String ticketString = " ";

int ledCount = 0;
int ledTXSustain = 0;
int ledRXSustain = 0;

int pinA = 0;
int pinB = 0;
int pinC = 0;
int pinD = 0;

int PULSE_WIDTH = 60;           // Notch Width
int TICKET_PRINT_DELAY = 240;   // Notch Spacing
int SKIP_TIME = 5000;           // Skip Time

double  serialDelayTime = 0;
int     tickets2PrintToA = 0;
int     tickets2PrintToB = 0;
int     tickets2PrintToC = 0;
int     tickets2PrintToD = 0;

double ticket1ADelay = 0;
double ticket2BDelay = 0;
double ticket3CDelay = 0;
double ticket4DDelay = 0;
bool handShaken = false;
bool isConnected = false;
int i = 0;

void ledManagement() {

 ledTXSustain--;
 if (ledTXSustain <= 0) {
    digitalWrite(TXLED, HIGH);
 }

  if (ledRXSustain > 0) {
     ledRXSustain--;
    digitalWrite(RXLED, LOW);
 }
  
}

void setup() {
  Serial.begin(BAUD);
  Serial.setTimeout(SERIAL_READ_TIMEOUT);
 
  pinMode(PLAYER_1A_TICKET_PIN, OUTPUT);
  pinMode(PLAYER_2B_TICKET_PIN, OUTPUT);
  pinMode(PLAYER_3C_TICKET_PIN, OUTPUT);
  pinMode(PLAYER_4D_TICKET_PIN, OUTPUT);
}



void loop() {

  ledManagement();


 if (!isConnected) {

  digitalWrite(RXLED, HIGH);
  digitalWrite(TXLED, HIGH);
    
  if (Serial.available() > 0){
    
    receivedString = Serial.readStringUntil('\n');
  }
  
  if (receivedString == handShakeMessage) {
    isConnected = true;
    Serial.setTimeout(500);
    Serial.write("<Handshake>"); 
    Serial.setTimeout(SERIAL_READ_TIMEOUT);
    handShaken = true;
  }
  
 }

 if (isConnected) {

  if (millis() > serialDelayTime ||  handShaken == false ) {
    
    digitalWrite(TXLED, LOW); 
    serialDelayTime = millis() + DELAY_REV_MSG_TIME;
    ledTXSustain = LED_SUSTAIN;


      if (Serial.available() > 0)        
         {
         
         ledRXSustain = LED_SUSTAIN;
         receivedString = Serial.readStringUntil('\n');
         if (receivedString == "Clear") {
           tickets2PrintToA = 0;
           tickets2PrintToB = 0;
           tickets2PrintToC = 0;
           tickets2PrintToD = 0;
           receivedString = " ";
              Serial.write("<Got Message>"); 
        }
         else if (receivedString.charAt(0) == 'a' || receivedString.charAt(0) == 'A' ||
             receivedString.charAt(0) == 'b' || receivedString.charAt(0) == 'B' ||
             receivedString.charAt(0) == 'c' || receivedString.charAt(0) == 'C' ||
             receivedString.charAt(0) == 'd' || receivedString.charAt(0) == 'D') {
              Serial.write("<Got Message>"); 
             }

        else if (receivedString.charAt(0) == 'w' || receivedString.charAt(0) == 'W' ) {
              Serial.write("<Got Message>"); 
             }

        else if (receivedString.charAt(0) == 's' || receivedString.charAt(0) == 'S' ) {
              Serial.write("<Got Message>"); 
             }

        else if (receivedString.charAt(0) == 'x' || receivedString.charAt(0) == 'X' ) {
              Serial.write("<Got Message>"); 
             }

        else if (receivedString == "Skip") {
           Serial.write("<Got Message>"); 
           serialDelayTime = millis() + SKIP_TIME;
        
        }
         else if (receivedString == "Quit") {
           Serial.write("<Got Message>"); 
            isConnected = false;
            handShaken = false;
        } 

        else if (receivedString == handShakeMessage) {
            isConnected = true;
            Serial.write("<Handshake>"); 
            handShaken = true;
           }
                
      } 

  }

  
  if (receivedString != " " && receivedString.length() > 1) {
   
    ticketString = receivedString.charAt(1);
    
    for (i = 2; i < receivedString.length(); i++) {
    //if (ticketString.concat(receivedString.charAt(i)) < 48 || ticketString.concat(receivedString.charAt(i)) > 57) continue;
    ticketString.concat(receivedString.charAt(i));
    }

    switch(receivedString.charAt(0)) {
      
      case 'a':
      case 'A':
        tickets2PrintToA += ticketString.toInt();
      break;
      
      case 'b':
      case 'B':
        tickets2PrintToB += ticketString.toInt();
      break;
      
      case 'c':
      case 'C':
        tickets2PrintToC += ticketString.toInt();
      break;
      
      case 'd':
      case 'D':
        tickets2PrintToD += ticketString.toInt();
      break;
      
      case 'W':
      case 'w':
        PULSE_WIDTH = ticketString.toInt();
      break;
      
      case 'S':
      case 's':
        TICKET_PRINT_DELAY = ticketString.toInt();
      break;
      
      case 'X':
      case 'x':
        SKIP_TIME = ticketString.toInt();
      break;
    }

    receivedString = " ";
  
  } 
 }

  /// PRINTER DEBUG
  Serial.println(" ");
  Serial.print("Connection : ");
  if (isConnected)   Serial.print("1 ");
  else               Serial.print("0 ");
  Serial.print("HandS : ");
  if (handShaken)   Serial.print("1 ");
  else               Serial.print("0 ");
  Serial.print("Tickets ::: A:");
  Serial.print((int)tickets2PrintToA);
  Serial.print(", B:");
  Serial.print((int)tickets2PrintToB);
  Serial.print(", C:");
  Serial.print((int)tickets2PrintToC);
  Serial.print(", D:");
  Serial.print((int)tickets2PrintToD);

  
 if (isConnected) {


  /// PRINT TICKETS

  if (tickets2PrintToA > INT_MAX) tickets2PrintToA = INT_MAX - 1;
  if (tickets2PrintToB > INT_MAX) tickets2PrintToB = INT_MAX - 1;
  if (tickets2PrintToC > INT_MAX) tickets2PrintToC = INT_MAX - 1;
  if (tickets2PrintToD > INT_MAX) tickets2PrintToD = INT_MAX - 1;

  if (tickets2PrintToA > 0 && millis() > ticket1ADelay)   { 
    ticket1ADelay = TICKET_PRINT_DELAY + millis();
    digitalWrite(PLAYER_1A_TICKET_PIN,HIGH);
    pinA = 1;
    tickets2PrintToA--;
  }
 
  if (tickets2PrintToB > 0 && millis() > ticket2BDelay)   { 
    ticket2BDelay = TICKET_PRINT_DELAY + millis();
    digitalWrite(PLAYER_2B_TICKET_PIN,HIGH);
    pinB = 2;
    tickets2PrintToB--;
  }
  
  if (tickets2PrintToC > 0 && millis() > ticket3CDelay)   { 
    ticket3CDelay = TICKET_PRINT_DELAY + millis();
    digitalWrite(PLAYER_3C_TICKET_PIN,HIGH);
    pinC = 1;
    tickets2PrintToC--;
  }

  if (tickets2PrintToD > 0 && millis() > ticket4DDelay)   { 
    ticket4DDelay = TICKET_PRINT_DELAY + millis();
    digitalWrite(PLAYER_4D_TICKET_PIN,HIGH);
    pinD = 1;
    tickets2PrintToD--;
  }


  if (pinA == 1 || pinB == 1|| pinC == 1|| pinD == 1) delay(PULSE_WIDTH);
 }
  
digitalWrite(PLAYER_1A_TICKET_PIN,LOW);
digitalWrite(PLAYER_2B_TICKET_PIN,LOW);
digitalWrite(PLAYER_3C_TICKET_PIN,LOW);
digitalWrite(PLAYER_4D_TICKET_PIN,LOW);

}
