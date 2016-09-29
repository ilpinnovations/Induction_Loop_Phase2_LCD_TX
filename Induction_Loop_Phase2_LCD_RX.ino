#include <LiquidCrystal595.h>
LiquidCrystal595 lcd(3,4,5);
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
unsigned long dataBuffer;
int ledPin = 10;
void setup(){
  lcd.begin(16,2);
  printBootScreen();
  Mirf.cePin = 8;  
  Mirf.csnPin = 9;  
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.configRegister(RF_SETUP,0x26);//250 Kbps
  Mirf.setRADDR((byte *)"NODE0");
  Mirf.setTADDR((byte *)"NODE1");
  Mirf.payload =sizeof(dataBuffer);
  Mirf.config();
  // Mirf.channel=83;
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
 }
void loop(){
 while(Mirf.dataReady()){
   Mirf.getData((byte *) &dataBuffer);
   Serial.println(dataBuffer);
   printCountData();
  }
 delay(1);
}

void printBootScreen()
{
  lcd.setCursor(0,0);
  lcd.print(" ILP Innovations ");
  lcd.setCursor(0,1);
  lcd.print(" Initilazing ... ");
  delay(500);
  lcd.clear();
}
void printCountData()
{ 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" ILP Innovations ");
  lcd.setCursor(0,1);
  String line2 = "CAR "+String(dataBuffer);
  lcd.print(line2);
}
