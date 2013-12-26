#include <SD.h>
#include <TimerThree.h>

int CS2 = 5;
int CS = 4;
int RD = 3;
int WR = 2;
int POWER = 12;
int SI = 19;
int SO = 18;
int SD_ = 13;
int SC_ = 14;
long currentRead = 0;
volatile boolean clock = false;
volatile uint32_t readData = 0;
#define F_CPU 16000000
#define CHARLEN ((F_CPU/115200)-13)

File romFile;

void setup() 
{         
  Serial.begin(9600);  
  Serial1.begin(262144); //256 kb/s for the GBA
  
  pinMode(53, OUTPUT);
   
  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    while(true){};
  }
  Serial.println("initialization done.");
  
  pinMode(RD, OUTPUT); 
  pinMode(CS, OUTPUT); 
  digitalWrite(RD, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(CS, LOW);
  digitalWrite(CS2, HIGH);
  digitalWrite(SI, HIGH);
  digitalWrite(SD_, HIGH);
  analogWrite(POWER, 255); //Supply Power
  
  Serial.println("ROM dumper active.");
  //CheckChecksum(); //TODO - Checksums
  String filename = getGBAROMCode() + ".gba ";
  char fileNameCharArray[filename.length()];
  filename.toCharArray(fileNameCharArray, filename.length());
  romFile = SD.open(fileNameCharArray, O_CREAT | O_WRITE);
  Serial.println("Dumping game " + getGBAGameName() + " with code " + getGBAROMCode());
  latchAddress(0);
}

int mode = 3;
// the loop routine runs over and over again forever:
void loop() 
{
  if(mode == 1)
  {
    dumpROM();
    return;
  }
  else if(mode == 2)
  {
    dumpSave();
    return;
  }
  else if(mode == 3)
  {
    doMultiboot();
    return;
  }
    //TODO - GameBoy ROMs!
    
  String content = "";
  char character;

  while(Serial.available()) {
      character = Serial.read();
      content.concat(character);
  }

  if (content == "D") 
  {
    mode = 1;
  }
}

byte buffer1[2048];
byte buffer2[2048];
int currentBuffer = 0;

void dumpROM()
{
  Serial.println("Dumping GBA ROM...");
  latchAddress(0);
  unsigned long currentAddress = 0;
  int bufferCnt = 0;
  while(currentAddress < 0x800000)
  {
    latchAddress(currentAddress / 2);
    strobeRS();
    //String hex = String(PINL + (PINC << 8), HEX);
    if(currentBuffer == 0)
    {
      buffer1[bufferCnt] = PINC;
      buffer1[bufferCnt+1] = PINL;
      
      if(bufferCnt == 2048)
      {
        bufferCnt = 0;
        currentBuffer = 1;
        romFile.write(buffer1,2048);
        romFile.flush();
      }
    }
    else
    {
      buffer2[bufferCnt] = PINC;
      buffer2[bufferCnt+1] = PINL;
      
      if(bufferCnt == 2048)
      {
        bufferCnt = 0;
        currentBuffer = 0;
        romFile.write(buffer2,2048);
        romFile.flush();
      }
    }
    
    //Write every 512 bytes
    //if(currentAddress % 0x200000 == 0)
      //romFile.flush();
    
    if(currentAddress % 0x10000 == 0)
      Serial.println(String(currentAddress, HEX));
    //Serial.println(hex); //TODO - SD Card
    //Serial.println();
    currentAddress += 2;
    bufferCnt+=2;
  }
  romFile.close();
  Serial.println("Dump completed!");
  while(true){}
}

void dumpSave()
{
  Serial.println("Dumping GBA Save...");
  digitalWrite(CS2, LOW);
  Serial.println("Warming up SRAM...");
  delay(10000);
  /*while(currentRead < 90)
  {
    latchRAMAddress(currentRead);
    strobeRS(1);
    delay(100);
    currentRead++;
  }*/
  currentRead = 0x0;
  byte nextByte = 0;
  int firstAdd = 0;
  boolean trigger = false;
  while(currentRead < 0x10000)
  {
    latchRAMAddress(currentRead);
    strobeRS(1);
    int b = PINA;
    if(b == 0x56)
    {
      firstAdd = currentRead;
    }
    else if(b == 0x1B && firstAdd > 0)
    {
      //nextByte = b;
      Serial.println(String(firstAdd, HEX));
      firstAdd = 0;
      nextByte = 0;
      trigger = true;
    }
    else if(b == 0xEC && nextByte > 0)
    {
      nextByte = 0xEC;
    }
    else if(b == 0xFF && nextByte == 0xEC)
    {
      Serial.println(String(firstAdd, HEX));
      firstAdd = 0;
      nextByte = 0;
    }
    if(trigger)
    {
      String hex = String(b, HEX);
      Serial.println(hex); //TODO - SD Card
      Serial.println(String(currentRead, HEX));
    }
    if(currentRead % 0x2000 == 0)
    {
      Serial.println(String(currentRead,HEX) + "/0x10000 bytes read.");
    }
    currentRead++;
  }
  Serial.println("Save Dump Complete.");
  unlatchRAMAddress();
  while(true){};
}

void doMultiboot()
{
  while(true)
  {
    Serial.println("Awaiting GBA Connection...");
    while(true)
    {
      //if(readData != 0xFFFFFFFF)
       Serial.println(String(readData,HEX));
      //Send status check
      send16Bits(0x6202);
      //get16Bits();
    }
    Serial.println("Connection established! Serial code " + Serial1.read());
  }
}

//TimerThree Timer3;              // preinstatiate

void send16Bits(uint32_t value)
{
  int t = 0;
  digitalWrite(SO,HIGH);
  pinMode(SC_, INPUT);
  pinMode(SD_, INPUT);
  while(digitalRead(SC_) == 0 && digitalRead(SD_) == 0)
  {
    wait(CHARLEN/2);
    t++;
    if (t>10000) {
	return;
    }
  }
  
  wait(CHARLEN*8);
  pinMode(SC_, OUTPUT);
  pinMode(SD_, OUTPUT);
  digitalWrite(SC_, LOW);
  digitalWrite(SD_, LOW);
  
  for(int i = 0; i < 16; i++)
  {
    //digitalWrite(SC_, LOW);
    digitalWrite(SD_, (value & i) >> i);
    //digitalWrite(SC_, HIGH);
    //readData |= digitalRead(SI) << i;
    wait(CHARLEN);
  }
  pinMode(SD_, INPUT);
  wait(CHARLEN);
  get16Bits();
}

void get16Bits()
{
  readData = 0;
    int t = 0;
  digitalWrite(SO,LOW);
  pinMode(SC_, INPUT);
  pinMode(SD_, INPUT);
  while(digitalRead(SC_) == 1 && digitalRead(SD_) == 1)
  {
    wait(CHARLEN/2);
    t++;
    if (t>10000) {
        readData = 0xFFFF;
	return;
    }
  }
  
  wait(CHARLEN);
  for(int i = 0; i < 16; i++)
  {
    readData |= digitalRead(SD_) << i;
    wait(CHARLEN);
  }
  pinMode(SD_, INPUT);
  digitalWrite(SO,HIGH);
  wait(CHARLEN);
}

void wait(int i)
{
  for(int j = 0; j < i; j++)
  {
    __asm__("nop\n\t"); 
  }
}

String getGBAROMCode()
{
  String code = "";
  latchAddress(0x56);
  strobeRS();
  code.concat((char)PINC);
  code.concat((char)PINL);
  latchAddress(0x57);
  strobeRS();
  code.concat((char)PINC);
  code.concat((char)PINL);
  return code;
}

String getGBAGameName()
{
  String code = "";
  latchAddress(0x50);
  boolean hadZero = false;
  for(int i = 0; i < 6; i++)
  {
    latchAddress(0x50 + i);
    strobeRS();
    if(PINC == 0 && hadZero) //Two consecutive zeros
      break;
    code.concat((char)PINC);
    code.concat((char)PINL);
    hadZero = (PINL == 0);
  }
  return code;
}

void latchAddress(unsigned long address)
{
  DDRC = 0xFF;
  DDRL = 0xFF;
  DDRA = 0xFF;
  PORTC = address &     0xFF;
  PORTL = (address &   0xFF00) >> 8;
  PORTA = (address & 0xFF0000) >> 16;
  digitalWrite(CS, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(CS, LOW);   
    PORTC = 0x0;
 PORTL = 0x0;
 PORTA = 0x0;
  DDRC = 0x0;
 DDRL = 0x0;
 DDRA = 0x0;
}

void latchRAMAddress(int address)
{
  DDRC = 0xFF;
  DDRL = 0xFF;
  PORTC = address &     0xFF;
  PORTL = (address &   0xFF00) >> 8;  
}

void unlatchRAMAddress()
{
      PORTC = 0x0;
 PORTL = 0x0;
  DDRC = 0x0;
 DDRL = 0x0;
}

void strobeRS()
{
  strobeRS(10);  
}

void strobeRS(int del)
{
  digitalWrite(RD, LOW); 
  delayMicroseconds(del);
  digitalWrite(RD, HIGH);  
}

byte getByte(int address)
{
    latchAddress(address / 2);
    strobeRS();
    if(address & 0x1 == 1)
      return PINL;
    else
      return PINC;
}

int calculateCheckSum()
{
  int checksum = 0;

  for (int j = 0xA0; j < 0xBC; j++)
  {
      checksum -= getByte(j);
  }

  return ((checksum - 0x19) & 0xFF); //This doesn't seem to be returning the proper value. Will investigate.
}

int getCheckSum()
{
  latchAddress(0xBD);
  strobeRS();
  return PINC;
}

boolean ValidCheckSum()
{
  return (getCheckSum() == calculateCheckSum());
}

void CheckChecksum()
{
  boolean check = ValidCheckSum();
    
  if(!check)
    {
      Serial.println("Checksum failed!\nMake sure your cartridge is inserted properly\nand reset to try again.");
      Serial.println("Read Checksum:    " + String(getCheckSum(), HEX));
      Serial.println("Correct Checksum: " + String(calculateCheckSum(), HEX));
      while(true)
      {
        
      }
    }
}
