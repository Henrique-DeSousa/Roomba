const byte frameStartByte = 0x7E;
const byte frameTypeTXrequest = 0x10;
const byte frameTypeRXpacket = 0x90;
const byte frameTypeATresponse = 0x88;
const long destAddressHigh = 0x13A200;
const long destAddressLow = 0x4092FF7A;

#define US1TP A0
#define US1EP A1

#define LED_Manual 12
#define LED_automatico 13

#define US2TP 10  //trigger port
#define US2EP 8



boolean curva = true;
int horizontal_dist, vertical_dist;
int dur, dur2;
int speed_automatic, speed_manual;
int sensorPin = 0; //the analog pin the TMP36's Vout (sense) pin is connected to
byte ATcounter=0; // for identifying current AT command frame
byte rssi=0; // RSSI value of last received packet


  void
  setup() {
  Serial.begin(9600); 
  DDRD = 1 << DDD4 |  1 << DDD7 | 1 << DDD3 | 1 << DDD5 | 1<< DDB3;
  pinMode(LED_Manual, OUTPUT);
  pinMode(LED_automatico, OUTPUT);
  pinMode(US1TP, OUTPUT);
  pinMode(US2TP, OUTPUT);
  pinMode(US1EP, INPUT);
  pinMode(US2EP, INPUT);
  
  Serial.println("Chose Mode (a or m) for Automatic or Manual. Press 'e' to exit any mode.");
}

void loop() {
  char key='nul';
  if(Serial.available() > 14) {
      key=decodeAPIpacket();
    if (key == 'a') {  // Modo automatico
      Serial.println("Entering in Automatic Mode.");
      analogWrite(11, map(5, '0', '9', 0, 255));
      while (true) {
        if (Serial.available()  > 14) {
            key=decodeAPIpacket();
        }
        if(key == 'e'){
          break;
        }
          digitalWrite(LED_automatico, HIGH);
          Sense();
          char spd = '5';
          speed_automatic = map(spd, '0', '9', 0, 255);
          analogWrite(5, speed_automatic);
          analogWrite(3, speed_automatic);
          PORTD |= 1 << PORTD4;
          PORTD &= ~(1 << PORTD7);
          if(horizontal_dist < 7){
            while(horizontal_dist <=7){
              if(curva==true){
                analogWrite(3, 0);
                curva=false;
                break;
              }
              else{
                analogWrite(5, 0);
                curva=true;
                break;
              }
              Sense();
            }
          }else if(vertical_dist >5){
            while(vertical_dist >5){
            PORTD |= 1 << PORTD7; 
            PORTD &= ~(1 << PORTD4);
              if(curva==true){
                analogWrite(3, 0);
                curva=false;
                break;
              }else{
                analogWrite(5, 0);
                 curva=true;
                break;
              }
            Sense();
            }
          }
      }
      Serial.println("Exiting Automatic mode.");
      digitalWrite(LED_automatico, LOW);
      analogWrite(5, 0);
      analogWrite(3, 0);
      analogWrite(11, 0);
      Serial.println("Chose Mode (a or m) for Automatic or Manual. Press 'o' to turn off.");
  } else if (key == 'm') {  // Modo Manual use [''] for anything not [""]
      Serial.println("Entering in Manual Mode.");
      digitalWrite(LED_Manual, HIGH);
      analogWrite(11, map(5, '0', '9', 0, 255));
      Serial.println("Speed (0-9) or + - to set direction.");
        while (key != 'e') {   // To exit Manual mode
          if (Serial.available()> 14) {
            key=decodeAPIpacket();
            if (isDigit(key)) {  // is ch a number?
              Serial.println(key);
              speed_manual = map(key, '0', '9', 0, 255); // para ter um valor -> '9' ou '0' ou '6'
              Serial.println(key);
              analogWrite(5, speed_manual);
              analogWrite(3, speed_manual);  //aplicar onda PWM no pino PD5 que controla o Enable da H-bridge.
            }else{
              switch(key){
                case '+': // Forward
                  Serial.println("CW");
                  PORTD |= 1 << PORTD4;     // Colocar pino PD4 (PORTD4) a HIGH sem estragar os outros! (Sugestão: Usar ‘|’)
                  PORTD &= ~(1 << PORTD7);  // Colocar pino PD7 (PORTD7) a LOW sem estragar os outros! (Sugestão: Usar ‘&’)
                  break;

                case '-': // Backwards
                  Serial.println("CCW"); 
                  PORTD |= 1 << PORTD7;     // Colocar pino PD7 (PORTD7) a HIGH sem estragar os outros! (Sugestão: Usar ‘|’)
                  PORTD &= ~(1 << PORTD4);  // Colocar pino PD4 (PORTD4) a LOW sem estragar os outros! (Sugestão: Usar ‘&’)
                  break;

                case 'r': // Right 
                  analogWrite(3, 0);

                  delay(3000);
                  analogWrite(3, speed_manual);
                  break;
                case 'l': //Left
                  analogWrite(5, 0);

                  delay(3000);
                  analogWrite(5, speed_manual);
                  break;
                case 'e':
                  break;
                
              default: 
                Serial.print("Unexpected character: ");
                Serial.println(key);
              }
            }
          }
        } 
        Serial.println("Exiting Manual mode.");
        digitalWrite(LED_Manual, LOW);
        analogWrite(5, 0);
        analogWrite(3, 0);
        analogWrite(11, 0);
        Serial.println("Chose Mode (a or m) for Automatic or Manual. Press 'o' to turn off.");
    }else if(key == 'o'){
      Serial.println("Turning off.");
      analogWrite(11, 0);
      delay(1000);
      exit(0);
    }
  }  
}

void Sense() {
  delay(1500);
  digitalWrite(US1TP, LOW);
  delayMicroseconds(10);
  digitalWrite(US1TP, HIGH);
  delayMicroseconds(10);
  digitalWrite(US1TP, LOW);
  dur = pulseIn(US1EP, HIGH);
  horizontal_dist = dur * 0.034 / 2;
  delayMicroseconds(120);

  digitalWrite(US2TP, LOW);
  delayMicroseconds(10);
  digitalWrite(US2TP, HIGH);
  delayMicroseconds(10);
  digitalWrite(US2TP, LOW);
  dur2 = pulseIn(US2EP, HIGH);
  vertical_dist = dur2 * 0.034 / 2;

  Serial.print("Horizontal Distance:  ");
  Serial.print(horizontal_dist);
  Serial.println("cm");
    
  Serial.print("Vertical Distance:    ");
  Serial.print(vertical_dist);
  Serial.println("cm");

  int values[] = {horizontal_dist, vertical_dist};
  formatTXAPIpacket(values, 2);
}

void formatATcommandAPI(char* command) { 
  // Format and transmit a AT Command API frame
  long sum = 0; // Accumulate the checksum

  ATcounter += 1; // increment frame counter

  // API frame Start Delimiter
  Serial.write(frameStartByte);

  // Length - High and low parts of the frame length (Number of bytes between the length and the checksum)
  Serial.write(0x00);
  Serial.write(0x04);

  // Frame Type - Indicate this frame contains a AT Command
  Serial.write(0x08); 
  sum += 0x08;

  // Frame ID – cannot be zero for receiving a reply
  Serial.write(ATcounter); 
  sum += ATcounter;

  // AT Command
  Serial.write(command[0]);
  Serial.write(command[1]);
  sum += command[0];
  sum += command[1];

  // Parameter Value for the Command - Optional

  // Checksum = 0xFF - the 8 bit sum of bytes from offset 3 (Frame Type) to this byte.
  Serial.write( 0xFF - ( sum & 0xFF));

  delay(10); // Pause to let the microcontroller settle down if needed
}

char decodeAPIpacket(void) {
  // Function for decoding the received API frame from XBEE
  char rxbyte='nul';
  byte frametype;

  while (Serial.read() != frameStartByte){
    if (Serial.available()==0)
      return rxbyte; // No API frame present. Return 'nul'
  }

  // Skip over length field bytes in the API frame
  Serial.read(); // MSB
  Serial.read(); // LSB

  // Read received frame type
  frametype=Serial.read();

  if (frametype==frameTypeRXpacket){
    // Zigbee Receive Packet API Frame
    while (Serial.available()<13); // the remainder of the Receive Packet API Frame should have 13 bytes, so only proceed when the full frame is in the Serial buffer

    // Skip over the bytes in the API frame we don't care about (addresses, receive options)
    for (int i = 0; i < 11; i++) {
      Serial.read();
    }
    // The next byte is the key pressed and sent from the remote XBEE
    rxbyte = Serial.read();

    Serial.read(); // Read  the last byte (Checksum) but don't store it
    formatATcommandAPI("DB");  // query the RSSI of the last received packet
  } 
  else if (frametype==frameTypeATresponse){
    // AT Command Response API frame
    Serial.read();
    Serial.read();
    Serial.read();
    Serial.read();
    rssi = Serial.read();
    Serial.read();

  }

  return rxbyte;
}


void formatTXAPIpacket(int values[], int size) {
  int sum = 0; // Accumulate the checksum  

  // API frame Start Delimiter
  Serial.write(frameStartByte);

  // Length - High and low parts of the frame length (Number of bytes between the length and the checksum)
  Serial.write(0x00);
  // base with rssi (15) + X ints (X * 2)
  Serial.write(0x0F + (0x02 * size));

  // Frame Type - Indicate this frame contains a Transmit Request
  Serial.write(frameTypeTXrequest); 
  sum += frameTypeTXrequest;

  // Frame ID - set to zero for no reply
  Serial.write(0x00); 
  sum += 0x00;

  // 64-bit Destination Address - The following 8 bytes indicate the 64 bit address of the recipient (high and low parts).
  // Use 0xFFFF to broadcast to all nodes.
  Serial.write((destAddressHigh >> 24) & 0xFF);
  Serial.write((destAddressHigh >> 16) & 0xFF);
  Serial.write((destAddressHigh >> 8) & 0xFF);
  Serial.write(destAddressHigh & 0xFF);
  sum += ((destAddressHigh >> 24) & 0xFF);
  sum += ((destAddressHigh >> 16) & 0xFF);
  sum += ((destAddressHigh >> 8) & 0xFF);
  sum += (destAddressHigh & 0xFF);
  Serial.write((destAddressLow >> 24) & 0xFF);
  Serial.write((destAddressLow >> 16) & 0xFF);
  Serial.write((destAddressLow >> 8) & 0xFF);
  Serial.write(destAddressLow & 0xFF);
  sum += ((destAddressLow >> 24) & 0xFF);
  sum += ((destAddressLow >> 16) & 0xFF);
  sum += ((destAddressLow >> 8) & 0xFF);
  sum += ((destAddressLow & 0xFF));

  // 16-bit Destination Network Address - The following 2 bytes indicate the 16-bit address of the recipient.
  // Use 0xFFFE if the address is unknown.
  Serial.write(0xFF);
  Serial.write(0xFE);
  sum += 0xFF+0xFE;

  // Broadcast Radius - when set to 0, the broadcast radius will be set to the maximum hops value
  Serial.write(0x00);
  sum += 0x00;

  // Options
  // 0x01 - Disable retries and route repair
  // 0x20 - Enable APS encryption (if EE=1)
  // 0x40 - Use the extended transmission timeout
  Serial.write(0x20);
  sum += 0x20;

  // RF Data
  for (int i = 0; i < size; i++) {
    Serial.write((values[i] >> 8) & 0xFF);
    Serial.write(values[i] & 0xFF);
    sum += ((values[i] >> 8) & 0xFF) + (values[i] & 0xFF); 
  }

  Serial.write((byte)rssi);
  sum += rssi;


  // Checksum = 0xFF - the 8 bit sum of bytes from offset 3 (Frame Type) to this byte.
  Serial.write(0xFF - (sum & 0xFF));

  delay(10); // Pause to let the microcontroller settle down if needed
}
