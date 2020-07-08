/***************************************************
* This code is designed to measure the temperature *
* of something using a SparkFun MAX31855K breakout *
* for a K type thermocouple and store it on a      *
* microSD card for later data processing and       *
* visualiation. The controller is a teensy 3.1 and *
* the pinout is as follows:                        *
* Pin #, Function								   *	
*   8		Chip Detect (for SD card)			   *
*   9		Slave Select Thermocouple Module	   *
*  10		Slave Select SD card 				   *
*  11		MISO								   *
*  12		MOSI								   *
*  13		SCK									   *
*												   *
*		Teensy 3.1 3.3V out and GND pins are the   *
*		supply rails for the entire circuit		   *
*												   *
*		Serial debugging is on 9600 baud rate	   *
***************************************************/

//Requirements:
/* 
	1) take temperature measurements every 2 minutes
	2) store to SD card 
	3) Store in CSV format
	4) Store in deg. F
	
*/



#include "thermRead.h" // Using the max31855k driver, and a modified sparkfun library
#include <SPI.h>  // Included here too due Arduino IDE; Used in above header
#include <SD.h>   //SD card library

// Define SPI Arduino pin numbers (Arduino Pro Mini)
const uint8_t THERM_CS = 9; // Thermocouple Chip Select
const uint8_t SD_CS = 10; //SD Card Chip Select
const unsigned long dt = 120000; //time between measurements (in milliseconds), normally 120000
const int cardDetect = 8; //card detect (CD) pin, allows for the MCU to hold until the card is inserted

// Instantiate an instance of the SparkFunMAX31855k class
SparkFunMAX31855k probe(THERM_CS);


unsigned long currentTime;//current time

void setup() 
{
	
	pinMode(cardDetect, INPUT);
	if (!digitalRead(cardDetect))
	{
		Serial.println(F("No card detected. Waiting for card."));
		while (!digitalRead(cardDetect));
		delay(250); // 'Debounce insertion'
	}	
	Serial.begin(9600);
	Serial.println("\nBeginning...");
	delay(100);  // Let IC stabilize or first readings will be garbage
	
	Serial.print("Initializing SD card...");

	// see if the card is present and can be initialized:
	if (!SD.begin(SD_CS)) 
	{
		Serial.println("Card failed, or not present");
		// don't do anything more:
		while (1);
	}
	Serial.println("Card Initialized");
	
	File dataFile = SD.open("DATA.csv", FILE_WRITE);
	delay(50);
	// if the file is available, write to it:
	if (dataFile) 
	{
		dataFile.println("Time Since Boot (milliseconds) , Temperature (Deg F)");
		dataFile.close();
		// print to the serial port too:
		Serial.println("Formatting Printed: Time Since Boot (milliseconds) , Temperature (Deg F)");
	}
}


void loop() 
{
	currentTime = millis();
	
	float temperature = probe.readCJT();
	// Read methods return NAN if they don't have a valid value to return.
	// The following conditionals only print the value if it's not NAN.
	if (!isnan(temperature)) 
	{
		Serial.print("CJT is (˚C): ");
		Serial.println(temperature);
	}

	// Read the temperature in Celsius
	temperature = probe.readTempF();
	if (!isnan(temperature)) 
	{
		Serial.print("Temp[F]=");
		Serial.println(temperature);
	}


	// make a string for assembling the data to log:
	String toWrite = "";
	toWrite += currentTime;//current time in milliseconds
	toWrite += ",";
	toWrite += temperature;
	
	// open the file. note that only one file can be open at a time,
	// so you have to close this one before opening another.
	delay(50);
	File dataFile = SD.open("DATA.csv", FILE_WRITE);

	// if the file is available, write to it:
	if (dataFile) 
	{
		dataFile.println(toWrite);
		dataFile.close();
		// print to the serial port too:
		Serial.print("Line Printed: ");
		Serial.println(toWrite);
		
	}
	// if the file isn't open, pop up an error:
	else 
	{
		Serial.print("Error opening file at time: ");
		Serial.println(currentTime);
	}
	
	delay(dt);
}
