// #include <LiquidCrystal595.h>
// LiquidCrystal595 lcd(11,12,13);
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
unsigned long dataBuffer=0;
volatile unsigned long count = 0;
volatile unsigned long count1 = 0;
int dynamic_MAX=0;
int dynamic_MIN=0;
const int MAX_MIN_gap = 15;
bool carPresent = false; //Assuming car is not present while getting MAX value
const int ADC_pin=A0;
const int Hardware_pin=2;
volatile int raw_ADC;
volatile int normalized_ADC;
volatile int limited_ADC;
volatile int Previous_ADC=0;
volatile int threshold_ADC=256;
// String Count_LCDADC;
// String Count_LCDINT;
#define ARRAY_SIZE 5
int maxBufferArray[ARRAY_SIZE]={0};
int outputBufferArray[10]={0};
unsigned long previousMillis = 0;
const long interval = 5000;

void setup() {
// put your setup code here, to run once:
// lcd.begin(16,2);
Mirf.cePin = 8;
Mirf.csnPin = 9;
Mirf.spi = &MirfHardwareSpi; 
Mirf.init();
Mirf.configRegister(RF_SETUP,0x26);
Mirf.setRADDR((byte *)"NODE1");
Mirf.setTADDR((byte *)"NODE0");
Mirf.payload =sizeof(dataBuffer);
Mirf.config();
pinMode(2,INPUT);
// lcd.setCursor(0,0);
// lcd.print(" ILP Innovations ");
// lcd.setCursor(0,1);
// lcd.print(" Initilazing ... ");
// delay(1000);
attachInterrupt(digitalPinToInterrupt(2), UpCounter, RISING);  
// lcd.clear();
Serial.begin(9600);
for (int i = 0; i < ARRAY_SIZE-1; ++i)
{
updateLimits();
}
}

void loop() {
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {
previousMillis = currentMillis;
generateMAX();
updateLimits();
}

normalized_ADC = generateNormalizedSignal();
limited_ADC = limitNomalizedSignal();
if(limited_ADC <=threshold_ADC)
{
if(Previous_ADC-limited_ADC>=threshold_ADC){
UpCounter1();
ADC_lcd();
Previous_ADC=limited_ADC;
}
else
{
Previous_ADC=limited_ADC;
}
}
else
{
Previous_ADC=limited_ADC;
}
SerialDebug();
//int_lcd();
delay(10);
}
int limitNomalizedSignal(){
return (normalized_ADC>1023)?1023:(normalized_ADC<0)?0:normalized_ADC;
}
int generateNormalizedSignal(){ 
//raw_ADC = analogRead(ADC_pin);
raw_ADC = smooth(analogRead(ADC_pin),maxBufferArray);

return map(raw_ADC, dynamic_MIN, dynamic_MAX, 0, 1023);
}

void updateLimits(){
generateMAX();
dynamic_MIN = dynamic_MAX - MAX_MIN_gap;
}
void generateMAX()
{
// carPresent = digitalRead(Hardware_pin);
if(!carPresent){
dynamic_MAX = smooth(analogRead(ADC_pin),maxBufferArray);
}
}

int smooth(int current,int *previous)
{
for(int j=(ARRAY_SIZE-1);j>0;j--)
{
previous[j]=previous[j-1];
}
previous[0]=current;

float average=0;
for(int i=0;i<ARRAY_SIZE;i++)
{
average+=previous[i];
}
return round(average/ARRAY_SIZE);
}
void SerialDebug(){
Serial.print(raw_ADC);
Serial.print(",");
Serial.print(dynamic_MAX);
Serial.print(",");
Serial.print(dynamic_MIN);
Serial.print(",");
Serial.print(smooth(limited_ADC,outputBufferArray));
Serial.print(",");
Serial.println(threshold_ADC); 
}
void int_lcd()
{
// lcd.setCursor(0,1);
// Count_LCDINT = "CINT: " + String(count);
// lcd.print(Count_LCDINT);
}
void ADC_lcd()
{
// lcd.setCursor(0,0);
// Count_LCDADC = "CADC: " + String(count1);
// lcd.print(Count_LCDADC);
dataBuffer = count1;
Mirf.send((byte *) &dataBuffer);
delay(1); 
while(Mirf.isSending())
{}

}
void UpCounter(){
count++;
}
void UpCounter1(){
count1++;
}
