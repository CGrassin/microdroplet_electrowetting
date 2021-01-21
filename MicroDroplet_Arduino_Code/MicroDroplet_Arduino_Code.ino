/* MIT Open-Source LICENSE:
*
* Copyright 2018 Charles Grassin
* 
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the 
* "Software"), to deal in the Software without restriction, including 
* without limitation the rights to use, copy, modify, merge, publish, 
* distribute, sublicense, and/or sell copies of the Software, and to 
* permit persons to whom the Software is furnished to do so, subject 
* to the following conditions:
* 
* The above copyright notice and this permission notice shall be 
* included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// PINS
#define PIN_FEEDBACK (A0)
#define PIN_LE (A1)
#define PIN_CLK (A2)
#define PIN_BL (A3)
#define PIN_DI (A4)
#define PIN_SWA (12)
#define PIN_SWB (11)

// SETTINGS
#define ELECTRODE_ARRAY_WIDTH 8
#define ELECTRODE_ARRAY_HEIGHT 8
const byte HV507_LOOKUP_TABLE [] = {4,5,6,7,12,13,15,14,20,22,23,21,28,31,30,29,36,39,38,37,47,46,45,44,55,62,61,53,54,63,60,52,51,59,56,49,58,50,57,48,43,42,41,40,34,33,32,35,26,25,24,27,18,16,17,19,9,8,10,11,0,1,2,3};
#define BAUD_RATE 115200
#define SERIAL_BUFFER_LENGTH 10
#define TEST_PROG

// VARS
bool electrodes[ELECTRODE_ARRAY_WIDTH][ELECTRODE_ARRAY_HEIGHT];
char serialBuffer[SERIAL_BUFFER_LENGTH];
uint8_t currentIndex=0;

// Electrode array code
void clearElectrodes() {
  for (int x = 0; x <ELECTRODE_ARRAY_WIDTH ; x++) 
    for (int y = 0; y <ELECTRODE_ARRAY_HEIGHT ; y++) 
      setElectrode(x,y,false);
}

void setElectrodes() {
  for (int x = 0; x <ELECTRODE_ARRAY_WIDTH ; x++) 
    for (int y = 0; y <ELECTRODE_ARRAY_HEIGHT ; y++) 
    setElectrode(x,y,true);
}

void setElectrode(int x,int y,bool state) {
  if(state != electrodes[x][y]) {
    electrodes[x][y]=state;
    sendElectrode(x,y);
  }
}

void sendElectrode(int x,int y){
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.print(electrodes[x][y]);
  Serial.print("\n");
}

// Refer to HV507 datasheet
void writeHV507() {
  digitalWrite(PIN_LE, LOW);
  digitalWrite(PIN_CLK, LOW);
   
  for (int i = 0; i <64 ; i++) {
    digitalWrite(PIN_DI,electrodes[HV507_LOOKUP_TABLE[i]%8][HV507_LOOKUP_TABLE[i]/8]);
    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CLK, LOW);
  }

  digitalWrite(PIN_LE, HIGH);
  digitalWrite(PIN_LE, LOW);
}

void setup() {
  Serial.begin(BAUD_RATE);
  
  pinMode(PIN_LE, OUTPUT);
  digitalWrite(PIN_LE,LOW);
  pinMode(PIN_CLK, OUTPUT);
  digitalWrite(PIN_CLK,LOW);
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL,HIGH);
  pinMode(PIN_DI, OUTPUT);
  digitalWrite(PIN_DI,LOW);
  
  pinMode(PIN_SWA, INPUT_PULLUP);
  pinMode(PIN_SWB, INPUT_PULLUP);

  clearElectrodes();
  writeHV507();
  digitalWrite(PIN_BL,HIGH);

  for (int x = 0; x <ELECTRODE_ARRAY_WIDTH ; x++) 
    for (int y = 0; y <ELECTRODE_ARRAY_HEIGHT ; y++) 
      sendElectrode(x,y);
}

#ifdef TEST_PROG
  int step = 0;
  long unsigned int lastT = millis();
#endif

void loop() {
  if(serialReadCommand()){
    writeHV507();
  }

  // Test program
  #ifdef TEST_PROG
    if(millis()-lastT>500){
      lastT = millis();
      switch(step){
        case 0: setElectrode(4,3,false);
        setElectrode(4,4,true);
        break;
        case 1:setElectrode(4,4,false);
        setElectrode(4,5,true);
        break;
        case 2:setElectrode(4,5,false);
        setElectrode(5,5,true);
        break;
        case 3:setElectrode(5,5,false);
        setElectrode(5,4,true);
        break;
        case 4:setElectrode(5,4,false);
        setElectrode(5,3,true);
        break;
        case 5:setElectrode(5,3,false);
        setElectrode(4,3,true);
        break;
      }
      writeHV507();
      step ++;
      if(step > 5) step = 0;
    }
  #endif
}

uint8_t serialReadCommand(){
  uint8_t number_of_commands = 0;
  while (Serial.available() > 0) {
    char recieved = Serial.read();

    serialBuffer[currentIndex] = recieved;
    currentIndex ++;
    if (currentIndex >= SERIAL_BUFFER_LENGTH) {
      // Invalid command: command is too long
      currentIndex = 0;
    }

    if (recieved == '\n' || recieved == '\r') {
      // Close string
      serialBuffer[currentIndex-1] = '\0';
      
      // Clear buffer for next serial transaction
      currentIndex = 0; 

      // Split the string
      char* commaIndex = strchr(serialBuffer, ',');
      if (commaIndex==NULL) {
        // Invalid command: command is malformed
        continue;
      }
      commaIndex[0] = '\0';
      char* secondCommaIndex = strchr(commaIndex+1, ',');
      if (secondCommaIndex==NULL) {
        // Invalid command: command is malformed
        continue;
      }
      secondCommaIndex[0] = '\0';

      int x = atoi(serialBuffer);
      int y = atoi(commaIndex+1);
      if(x<0 || x>=ELECTRODE_ARRAY_WIDTH || y<0 || y>=ELECTRODE_ARRAY_HEIGHT){
        // Invalid command: out of bound
        continue;
      }
      
      setElectrode(x,y,strcmp(secondCommaIndex+1,"0")!=0);
      number_of_commands += 1;
    }
  }
  return number_of_commands;
}

/*void btnMatrixTest(){
  for (int y = 0; y <ELECTRODE_ARRAY_WIDTH ; y++) 
    for (int x = 0; x <ELECTRODE_ARRAY_HEIGHT ; x++){
      clearElectrodes();
      electrodes[x][y]=true;
      writeHV507();
      Serial.print(x);
      Serial.print("\t");
      Serial.println(y);
      delay(500);
      while(digitalRead(PIN_SWA)){}
    }
}*/
