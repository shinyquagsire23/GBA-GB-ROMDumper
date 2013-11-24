int CS2 = 5;
int CS = 4;
int RD = 3;
int WR = 2;
int POWER = 12;

void setup() 
{         
  Serial.begin(9600);  
  
  pinMode(RD, OUTPUT); 
  pinMode(CS, OUTPUT); 
  digitalWrite(RD, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(CS, LOW);
  digitalWrite(CS2, HIGH);
  analogWrite(POWER, 255); //Supply Power
  
  Serial.println("ROM dumper active.");
  //CheckChecksum(); //TODO - Checksums
  Serial.println("Dumping game " + getGBAGameName() + " with code " + getGBAROMCode());
  latchAddress(0);
}

int mode = 1;
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
    //TODO - Dump GBA SRAM
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

void dumpROM()
{
  Serial.println("Dumping GBA ROM...");
  latchAddress(0);
  while(true)
  {
    strobeRS();
    String hex = String(PINL + (PINC << 8), HEX);
    Serial.println(hex); //TODO - SD Card
    Serial.println();
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

void latchAddress(int address)
{
  DDRC = 0xFF;
  DDRL = 0xFF;
  DDRA = 0xFF;
  PORTC = address &     0xFF;
  PORTL = (address &   0xFF00) >> 8;
  PORTA = (address & 0xFF0000) >> 16;
  digitalWrite(CS, HIGH);   
  delayMicroseconds(10000);           
  digitalWrite(CS, LOW);   
    PORTC = 0x0;
 PORTL = 0x0;
 PORTA = 0x0;
  DDRC = 0x0;
 DDRL = 0x0;
 DDRA = 0x0;
}

void strobeRS()
{
  digitalWrite(RD, LOW); 
  delayMicroseconds(10000); 
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
