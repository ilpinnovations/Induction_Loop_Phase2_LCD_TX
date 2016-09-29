#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
//ADC Reading
#define ARRAY_SIZE 5
#define ADC_PIN A0
int ADC_SignalArray[ARRAY_SIZE]={0};
int filteredSignal=0;
int scaledSignal=0;
//Refresh Functions
unsigned long previousMillis = 0;
const unsigned long refreshInterval = 3;
//Maxmin generation
bool carPresent = false; //Assuming car is not present while getting MAX value
int dynamic_MAX=0;
int dynamic_MIN=0;
const int MAX_MIN_gap = 10;
bool refreshReady= false;
unsigned long currentMillis =0;
//Scaling
#define ARRAY_SIZE 5
int ScaledSignalArray[ARRAY_SIZE]={0};
int filterdscaledSignal = 0;
//Hard Threshold
const int hard_threshold=700;
bool outputState = false;         // current state of the button
bool lastOutputState = false;     // previous state of the button
//Counter
unsigned long carCount=0;
//RF Setups
unsigned long dataBuffer=0;

void setup() {
  // put your setup code here, to run once:
	Mirf.cePin = 8;
	Mirf.csnPin = 9;
	Mirf.spi = &MirfHardwareSpi; 
	Mirf.init();
	Mirf.configRegister(RF_SETUP,0x26);
	Mirf.setRADDR((byte *)"NODE1");
	Mirf.setTADDR((byte *)"NODE0");
	Mirf.payload =sizeof(dataBuffer);
	Mirf.config();
  Serial.begin(9600);
  filteredSignal=readAndFilterInput();
  filteredSignal=readAndFilterInput();
  filteredSignal=readAndFilterInput();
  filteredSignal=readAndFilterInput();
  filteredSignal=readAndFilterInput();
  filteredSignal=readAndFilterInput();
  generateMaxMin();
}

void loop() {
  // put your main code here, to run repeatedly:
  	filteredSignal=readAndFilterInput();
	scaledSignal=limiter(scaleSignal(),1024,0);//data,max,min;	
	filterdscaledSignal = filterScaledSignal();
	outputState = hardThresholding(filterdscaledSignal,hard_threshold);
	edgeDetector();
	SerialDebug();
	lastOutputState = outputState;
	currentMillis = millis()/1000;
	if (currentMillis - previousMillis >= refreshInterval) {
		previousMillis = currentMillis;
		generateMaxMin();
		transmitCount();
		// return true;
	}
	// refreshReady = readyToRefreshLevels();
	// if(refreshReady){
	// 	generateMaxMin();
	// 	refreshReady = false;
		// Serial.println("Update");
	// 	// transmitCount();
	// }
}
int readAndFilterInput(){
	return smooth(analogRead(ADC_PIN),ADC_SignalArray);
}
bool readyToRefreshLevels(){
	currentMillis = millis();
	if (currentMillis - previousMillis >= refreshInterval) {
		previousMillis = currentMillis;
		return true;
	}
}
void generateMaxMin(){
	// carPresent = digitalRead(Hardware_pin);
	if(!carPresent){
		dynamic_MAX = filteredSignal;
		dynamic_MIN = dynamic_MAX - MAX_MIN_gap;
	}
}
int scaleSignal(){
	return map(filteredSignal, dynamic_MIN, dynamic_MAX, 0, 1023);
}
int filterScaledSignal(){
	return smooth(scaledSignal,ScaledSignalArray);
}
bool hardThresholding(int signal,int threshold)
{
	return (signal<threshold)?true:false;
}
void edgeDetector(){
	if(outputState != lastOutputState){
	    if(outputState == false){ //falling edge
	    	carCount++;
	    	dataBuffer = carCount;	
	    	transmitCount();        
	    }
	}
}
int limiter(int data,int max,int min)
{
	return (data>max)?max:(data<min)?min:data;
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
void SerialDebug()
{
	Serial.print(filteredSignal);
	Serial.print(",");
	Serial.print(dynamic_MAX);
	Serial.print(",");
	Serial.print(dynamic_MIN);
	Serial.print(",");
	Serial.print(filterdscaledSignal);
	Serial.print(",");
	Serial.println(carCount); 
}
void transmitCount(){
	Mirf.send((byte *) &dataBuffer);
	delay(1); 
	while(Mirf.isSending())
		{}
}