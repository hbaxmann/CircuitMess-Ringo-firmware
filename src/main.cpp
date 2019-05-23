#include <Arduino.h>
#include "main.h"
#include "lockscreen.h"
#include "mainMenu.h"
MAKERphone mp;
Oscillator *osc;
int backgroundColors[7] PROGMEM = {
	TFT_CYAN,
	TFT_GREEN,
	TFT_RED,
	TFT_YELLOW,
	TFT_WHITE,
	TFT_ORANGE,
	TFT_PINK
};
String backgroundColorsNames[7] PROGMEM = {
	"Cyan",
	"Green",
	"Red",
	"Yellow",
	"White",
	"Orange",
	"Pink"
};
String titles[10] PROGMEM = {
	"Messages",
	"Media",
	"Contacts",
	"Settings",
	"Phone",
	"Apps",
	"Clock",
	"Calculator",
	"Flashlight",
	"Calendar"
};
int textPointer = 0;
StaticJsonBuffer<capacity> jb;
uint8_t audioCount = 0;
String audioFiles[100];

void menuDrawBox(String text, uint8_t i, int32_t y) {
	uint8_t scale;
	uint8_t offset;
	uint8_t boxHeight;
	if(mp.resolutionMode)
	{
		scale = 1;
		offset = menuYOffset;
		boxHeight = 7;
	}
	else
	{
		scale = 2;
		offset = 17;
		boxHeight = 15;
	}
	y += i * (boxHeight + 1) + offset;
	if (y < 0 || y > mp.display.height()) {
		return;
	}
	mp.display.fillRect(1, y + 1, mp.display.width() - 2, boxHeight - (scale-1), TFT_DARKGREY);
	mp.display.setTextColor(TFT_WHITE);
	mp.display.setCursor(2, y + 2);
	if(mp.resolutionMode)
		mp.display.drawString(text, 3, y + 2);
	else
		mp.display.drawString(text, 3, y);
}
int8_t menu(const char* title, String* items, uint8_t length) {
	int32_t cameraY = 0;
	int32_t cameraY_actual = 0;
	int16_t cursor = 0;
	uint8_t offset;
	uint8_t boxHeight;
	if(mp.resolutionMode)
	{
		offset = menuYOffset;
		boxHeight = 7;
	}
	else
	{
		offset = 19;
		boxHeight = 15;
	}
	while (1) {
		while (!mp.update());
		mp.display.fillScreen(TFT_BLACK);
		mp.display.setCursor(0, 0);
		cameraY_actual = (cameraY_actual + cameraY) / 2;
		if (cameraY_actual - cameraY == 1) {
			cameraY_actual = cameraY;
		}

		for (uint8_t i = 0; i < length; i++) {
			menuDrawBox(items[i], i, cameraY_actual);
		}
		menuDrawCursor(cursor, cameraY_actual);

		// last draw the top entry thing
		if(mp.resolutionMode)
		{
			mp.display.fillRect(0, 0, mp.display.width(), 6, TFT_DARKGREY);
			mp.display.setFreeFont(TT1);
			mp.display.setCursor(0,5);
			mp.display.drawFastHLine(0, 6, mp.display.width(), TFT_WHITE);
		}
		else
		{
			mp.display.fillRect(0, 0, mp.display.width(), 14, TFT_DARKGREY);
			mp.display.setTextFont(2);
			mp.display.setCursor(1,-2);
			mp.display.drawFastHLine(0, 14, mp.display.width(), TFT_WHITE);
		}
		mp.display.print(title);

		if (mp.buttons.released(BTN_A)) {   //BUTTON CONFIRM
			osc->note(75, 0.05);
			osc->play();
			while (!mp.update());// Exit when pressed
			break;
		}
		if (mp.buttons.released(BTN_B))   //BUTTON BACK
			return -1;

		if (mp.buttons.released(BTN_UP)) {  //BUTTON UP
			osc->note(75, 0.05);
			osc->play();
			mp.leds[3] = CRGB::Blue;
			mp.leds[4] = CRGB::Blue;
			while(!mp.update());

			if (cursor == 0) {
				cursor = length - 1;
				if (length > 6) {
					cameraY = -(cursor - 5) * 8;
				}
			}
			else {
				cursor--;
				if (cursor > 0 && (cursor * 8 + cameraY + offset) < 14) {
					cameraY += 8;
				}
			}
		}

		if (mp.buttons.released(BTN_DOWN)) { //BUTTON DOWN
			osc->note(75, 0.05);
			osc->play();
			mp.leds[0] = CRGB::Blue;
			mp.leds[7] = CRGB::Blue;
			while (!mp.update());


			cursor++;
			if ((cursor * 8 + cameraY + offset) > 54) {
				cameraY -= 8;
			}
			if (cursor >= length) {
				cursor = 0;
				cameraY = 0;

			}

		}
	}
	return cursor;

}
void menuDrawCursor(uint8_t i, int32_t y) {
	uint8_t offset;
	uint8_t boxHeight;
	if(mp.resolutionMode)
	{
		offset = menuYOffset;
		boxHeight = 7;
	}
	else
	{
		offset = 17;
		boxHeight = 15;
	}
	if (millis() % 500 <= 250) {
		return;
	}
	y += i * (boxHeight + 1) + offset;
	mp.display.drawRect(0, y, mp.display.width(), boxHeight + 2, TFT_RED);
}
int16_t audioPlayerMenu(const char* title, String* items, uint16_t length, uint16_t index) {

	int32_t cameraY = 0;
	int32_t cameraY_actual = 0;
	String Name;
	uint8_t scale = 2;
	uint8_t offset = 17;
	uint8_t boxHeight = 15;
	uint16_t start = 0;
	int16_t cursor = index;
	if (length > 12) {
		cameraY = -cursor * (boxHeight + 1) - 1;
	}
	while (1) {
		mp.display.fillScreen(TFT_BLACK);
		mp.display.setCursor(0, 0);
		cameraY_actual = (cameraY_actual + cameraY) / 2;
		if (cameraY_actual - cameraY == 1) {
			cameraY_actual = cameraY;
		}

		for (uint8_t i = 0; i < length; i++) {
			Name = items[i];
			while (Name.indexOf("/", start) != -1)
				start = Name.indexOf("/", start) + 1;
			Name = Name.substring(start, Name.indexOf("."));
			start = 0;
			menuDrawBox(Name, i, cameraY_actual);
		}
		menuDrawCursor(cursor, cameraY_actual);

			mp.display.fillRect(0, 0, mp.display.width(), 14, TFT_DARKGREY);
			mp.display.setTextFont(2);
			mp.display.setCursor(0,-2);
			mp.display.drawFastHLine(0, 14, mp.display.width(), TFT_WHITE);
		mp.display.setTextSize(1);
		mp.display.setTextColor(TFT_WHITE);
		mp.display.print(title);

		if (mp.buttons.released(BTN_A)) {   //BUTTON CONFIRM
			while (!mp.update());
			break;
		}

		if (mp.buttons.released(BTN_UP)) {  //BUTTON UP

			while (!mp.update());
			if (cursor == 0) {
				cursor = length - 1;
				if (length > 6*scale) {
					cameraY = -(cursor - 5) * (boxHeight + 1) - 1;
				}
			}
			else {
				if (cursor > 0 && (cursor * (boxHeight + 1) - 1 + cameraY + offset) <= boxHeight) {
					cameraY += (boxHeight + 1);
				}
				cursor--;
			}

		}

		if (mp.buttons.released(BTN_DOWN)) { //BUTTON DOWN
			while (!mp.update());
			cursor++;
			if ((cursor * (boxHeight + 1) + cameraY + offset) > 54*scale) {
				cameraY -= (boxHeight + 1);
			}
			if (cursor >= length) {
				cursor = 0;
				cameraY = 0;

			}

		}
		if (mp.buttons.released(BTN_B)) //BUTTON BACK
		{
			while (!mp.update());
			return -1;
		}
		mp.update();
	}
	return cursor;

}
void listAudio(const char * dirname, uint8_t levels) {
	audioCount = 0;
	while(!mp.SD.begin(5, SPI, 9000000))
		Serial.println(F("SD ERROR"));
	SDAudioFile root = mp.SD.open(dirname);
	if (!root) {
		Serial.println(F("Failed to open directory"));
		return;
	}
	if (!root.isDirectory()) {
		Serial.println(F("Not a directory"));
		return;
	}
	int counter = 1;
	uint8_t start = 0;
	SDAudioFile file = root.openNextFile();
	while (file) {
		String Name(file.name());
		if (Name.endsWith(F(".MP3")) || Name.endsWith(F(".mp3"))
		 || Name.endsWith(F(".wav")) || Name.endsWith(F(".WAV")))
		{
			audioFiles[counter - 1] = Name;
			audioCount++;
			counter++;
		}
		file = root.openNextFile();
	}
	root.close();
	file.close();
}
String readSerial() {
	uint8_t _timeout = 0;
	while (!Serial1.available() && _timeout < 300)
	{
		delay(20);
		_timeout++;
	}
	if (Serial1.available()) {
		return Serial1.readString();
	}
	return "";
}
uint16_t countSubstring(String string, String substring) {
	if (substring.length() == 0) return 0;
	int count = 0;
	for (size_t offset = string.indexOf(substring); offset != -1;
		offset = string.indexOf(substring, offset + substring.length()))
	{
		count++;
	}
	return count;
}
void callNumber(String number) {
	mp.dataRefreshFlag = 0;

	String localBuffer = "";
	Serial1.print(F("ATD"));
	Serial1.print(number);
	Serial1.print(";\r\n");
	mp.display.setFreeFont(TT1);
	mp.display.setTextColor(TFT_BLACK);
	bool firstPass = 1;
	uint32_t timeOffset = 0;
	uint16_t textLength;
	uint8_t scale;
	unsigned int tmp_time = 0;
	String temp;
	if(mp.resolutionMode)
	{
		scale = 1;
		mp.display.setFreeFont(TT1);
	}
	else
	{
		scale = 2;
		mp.display.setTextFont(2);
	}
	mp.display.setTextSize(1);

	mp.display.printCenter(number);
	textLength = mp.display.cursor_x - textLength;
	while (1)
	{
		mp.display.fillScreen(TFT_WHITE);
		if (Serial1.available())
			localBuffer = Serial1.readString();
		// Serial.println(localBuffer);
		delay(5);
		if (localBuffer.indexOf("CLCC:") != -1)
		{
			if (localBuffer.indexOf(",0,0,0,0") != -1)
			{
				if (firstPass == 1)
				{
					timeOffset = millis();
					firstPass = 0;
				}


				temp = "";
				if ((int((millis() - timeOffset) / 1000) / 60) > 9)
					temp += (int((millis() - timeOffset) / 1000) / 60) ;
				else
				{
					temp += "0";
					temp += (int((millis() - timeOffset) / 1000) / 60);
				}
				temp += ":";
				if (int((millis() - timeOffset) / 1000) % 60 > 9)
					temp += (int((millis() - timeOffset) / 1000) % 60);
				else
				{
					temp += "0";
					temp += (int((millis() - timeOffset) / 1000) % 60);
				}
				mp.display.setCursor(9, 9);
				mp.display.printCenter(temp);
				Serial.println("CALL ACTIVE");
				mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_GREEN, scale);
			}

			else if (localBuffer.indexOf(",0,3,") != -1)
			{
				mp.display.setCursor(25, 9);
				Serial.println("ringing");
				mp.display.printCenter("Ringing...");
				mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_DARKGREY, scale);
			}
			else if (localBuffer.indexOf(",0,2,") != -1)
			{
				mp.display.setCursor(25, 9);
				mp.display.printCenter("Calling...");
				mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_DARKGREY, scale);
			}
			else if (localBuffer.indexOf(",0,6,") != -1)
			{
				mp.display.fillScreen(TFT_WHITE);
				mp.display.setCursor(32, 9);
				if (timeOffset == 0)
					mp.display.printCenter("00:00");
				else
				{
					temp = "";
					if ((int((millis() - timeOffset) / 1000) / 60) > 9)
						temp += (int((millis() - timeOffset) / 1000) / 60) ;
					else
					{
						temp += "0";
						temp += (int((millis() - timeOffset) / 1000) / 60);
					}
					temp += ":";
					if (int((millis() - timeOffset) / 1000) % 60 > 9)
						temp += (int((millis() - timeOffset) / 1000) % 60);
					else
					{
						temp += "0";
						temp += (int((millis() - timeOffset) / 1000) % 60);
					}
					mp.display.setCursor(9, 9);
					mp.display.printCenter(temp);
				}
				mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_RED, scale);
				if(mp.resolutionMode)
					mp.display.setCursor(11, 20);
				else
					mp.display.setCursor(11, 28);
				mp.display.printCenter(number);
				mp.display.fillRect(0, 51*scale, 80*scale, 13*scale, TFT_RED);
				if(mp.resolutionMode)
					mp.display.setCursor(2, 62);
				else
					mp.display.setCursor(2, 112);
				mp.display.print("Call ended");
				Serial.println("ENDED");
				while (!mp.update());

				mp.updateTimeRTC();
				// 2019-04-18 12:00:00
				String dateTime = String(mp.clockYear);
				dateTime += "-";
				if(mp.clockMonth < 10){
					dateTime += "0";
				}
				dateTime += String(mp.clockMonth);
				dateTime += "-";
				if(mp.clockDay < 10){
					dateTime += "0";
				}
				dateTime += String(mp.clockDay);
				dateTime += " ";

				if(mp.clockHour < 10){
					dateTime += "0";
				}
				dateTime += String(mp.clockHour);
				dateTime += ":";
				if(mp.clockMinute < 10){
					dateTime += "0";
				}
				dateTime += String(mp.clockMinute);
				dateTime += ":";
				if(mp.clockSecond < 10){
					dateTime += "0";
				}
				dateTime += String(mp.clockSecond);

				addCall(number, dateTime, tmp_time);

				delay(1000);
				break;
			}
			if(mp.resolutionMode)
					mp.display.setCursor(11, 20);
			else
				mp.display.setCursor(11, 28);
			mp.display.printCenter(number);
			mp.display.fillRect(0, 51*scale, 80*scale, 13*scale, TFT_RED);
			if(mp.resolutionMode)
			{
				mp.display.setCursor(2, 62);
				mp.display.print("press");
				mp.display.drawBitmap(24, 52, letterB, TFT_BLACK);
				mp.display.setCursor(35, 62);
				mp.display.print("to hang up");
			}
			else
			{
				mp.display.setCursor(2, 112);
				mp.display.print("press");
				mp.display.drawBitmap(37, 105, letterB, TFT_BLACK, scale);
				mp.display.setCursor(55, 112);
				mp.display.print("to hang up");
			}

		}
		else if (localBuffer.indexOf("CLCC:") == -1)
		{
			if (localBuffer.indexOf("ERROR") != -1)
			{

				mp.display.setCursor(3, 9);
				mp.display.printCenter("Couldn't dial number!");
				mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_RED, scale);
				if(mp.resolutionMode)
					mp.display.setCursor(11, 20);
				else
					mp.display.setCursor(11, 28);
				mp.display.printCenter(number);
				mp.display.fillRect(0, 51*scale, 80*scale, 13*scale, TFT_RED);
				if(mp.resolutionMode)
				{
					mp.display.setCursor(2, 57);
					mp.display.print("Invalid number or");
					mp.display.setCursor(2, 63);
					mp.display.print("SIM card missing!");
				}
				else
				{
					mp.display.setCursor(2, 100);
					mp.display.print("Invalid number or");
					mp.display.setCursor(2, 112);
					mp.display.print("SIM card missing!");
				}
				while (!mp.buttons.released(BTN_B))
					mp.update();
				break;
			}
			else
			{
				mp.display.setCursor(25, 9);
				mp.display.printCenter("Calling...");
				mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_DARKGREY, scale);
				if(mp.resolutionMode)
					mp.display.setCursor(11, 20);
				else
					mp.display.setCursor(11, 28);
				mp.display.printCenter(number);
				mp.display.fillRect(0, 51*scale, 80*scale, 13*scale, TFT_RED);
				if(mp.resolutionMode)
				{
					mp.display.setCursor(2, 62);
					mp.display.print("press");
					mp.display.drawBitmap(24, 52, letterB, TFT_BLACK);
					mp.display.setCursor(35, 62);
					mp.display.print("to hang up");
				}
				else
				{
					mp.display.setCursor(2, 112);
					mp.display.print("press");
					mp.display.drawBitmap(37, 105, letterB, TFT_BLACK, scale);
					mp.display.setCursor(55, 112);
					mp.display.print("to hang up");
				}
			}
		}
		if (mp.buttons.pressed(BTN_B)) // hanging up
		{
			Serial.println("B PRESSED");
			Serial1.println("ATH");
			while (readSerial().indexOf(",0,6,") == -1)
			{
				Serial1.println("ATH");
			}
			mp.display.fillScreen(TFT_WHITE);
			mp.display.setCursor(32, 9);
			if (timeOffset == 0)
				mp.display.printCenter("00:00");
			else
			{
				temp = "";
				if ((int((millis() - timeOffset) / 1000) / 60) > 9)
					temp += (int((millis() - timeOffset) / 1000) / 60) ;
				else
				{
					temp += "0";
					temp += (int((millis() - timeOffset) / 1000) / 60);
				}
				temp += ":";
				if (int((millis() - timeOffset) / 1000) % 60 > 9)
					temp += (int((millis() - timeOffset) / 1000) % 60);
				else
				{
					temp += "0";
					temp += (int((millis() - timeOffset) / 1000) % 60);
				}
				mp.display.setCursor(9, 9);
				mp.display.printCenter(temp);
			}
			mp.display.drawBitmap(29*scale, 24*scale, call_icon, TFT_RED, scale);
			if(mp.resolutionMode)
					mp.display.setCursor(11, 20);
			else
				mp.display.setCursor(11, 28);
			mp.display.printCenter(number);
			mp.display.fillRect(0, 51*scale, 80*scale, 13*scale, TFT_RED);
			mp.display.setCursor(2, 112);
			mp.display.print("Call ended");
			Serial.println("ENDED");
			while (!mp.update());
			mp.updateTimeRTC();
			// 2019-04-18 12:00:00
			String dateTime = String(mp.clockYear);
			dateTime += "-";
			if(mp.clockMonth < 10){
				dateTime += "0";
			}
			dateTime += String(mp.clockMonth);
			dateTime += "-";
			if(mp.clockDay < 10){
				dateTime += "0";
			}
			dateTime += String(mp.clockDay);
			dateTime += " ";

			if(mp.clockHour < 10){
				dateTime += "0";
			}
			dateTime += String(mp.clockHour);
			dateTime += ":";
			if(mp.clockMinute < 10){
				dateTime += "0";
			}
			dateTime += String(mp.clockMinute);
			dateTime += ":";
			if(mp.clockSecond < 10){
				dateTime += "0";
			}
			dateTime += String(mp.clockSecond);

			addCall(number, dateTime, tmp_time);
			delay(1000);
			break;
		}

		tmp_time = int((millis() - timeOffset) / 1000);
		mp.update();
	}
}
bool startupWizard()
{
	// mp.inHomePopup = 1;
	// mp.inShutdownPopup = 1;
	// Connect charger
	// while(1)
	// {
	// 	mp.display.fillScreen(TFT_WHITE);
	// 	mp.display.setTextFont(2);
	// 	mp.display.setTextSize(1);
	// 	mp.display.setCursor(4,4);
	// 	mp.display.setTextColor(TFT_BLACK);
	// 	mp.display.printCenter("Startup wizard");
	// 	mp.display.setCursor(0, mp.display.height()/2 - 14);
	// 	mp.display.printCenter("Connect your charger");
	// 	mp.display.drawBitmap(72, 74, batteryCharging, TFT_BLACK, 3);
	// 	if(batteryVoltage > 4000)
	// 	{
	// 		mp.display.setTextColor(TFT_BLACK);
	// 		mp.display.setTextSize(1);
	// 		mp.display.setTextFont(2);
	// 		mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
	// 		mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
	// 		mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
	// 		mp.display.setCursor(47, 55);
	// 		mp.display.printCenter("Charger connected!");
	// 		uint32_t tempMillis = millis();
	// 		while(millis() < tempMillis + 1500)
	// 		{
	// 			mp.update();
	// 			if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
	// 			{
	// 				while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
	// 					mp.update();
	// 				break;
	// 			}
	// 		}
	// 		while(!mp.update());
	// 		break;
	// 	}
	// 	mp.update();
	// }
	// while(!mp.update());


	// // Buttons testing
	// bool buttonsArray[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// bool allPressed = 1; 
	// while(1)
	// {
	// 	mp.display.fillScreen(TFT_WHITE);
	// 	mp.display.setTextFont(2);
	// 	mp.display.setTextSize(1);
	// 	mp.display.setCursor(4,4);
	// 	mp.display.setTextColor(TFT_BLACK);
	// 	mp.display.printCenter("Startup wizard");
	// 	mp.display.setCursor(3, 97);
	// 	mp.display.print("Press every button");
	// 	mp.display.setCursor(3, 112);
	// 	mp.display.print("to continue");
	// 	allPressed = 1;
	// 	for(int i = 0; i < 18; i++)
	// 	{
	// 		if(buttonsArray[i])
	// 			mp.display.fillCircle(13 + 26*(i%6), 33 + int(i/6)*22, 8, TFT_GREEN);
	// 		else
	// 		{
	// 			allPressed = 0;
	// 			switch (i)
	// 			{
	// 			case 0:
	// 				if(mp.buttons.getKey() == 'A')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 1:
	// 				if(mp.buttons.currentKey == 'B')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 2:
	// 				if(mp.buttons.currentKey == 'C')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 3:
	// 				if(mp.buttons.getKey() == 'D')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 4:
	// 				if(mp.buttons.pressed(BTN_A))
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 5:
	// 				if(mp.buttons.pressed(BTN_B))
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 6:
	// 				if(mp.buttons.getKey() == '1')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 7:
	// 				if(mp.buttons.getKey() == '2')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 8:
	// 				if(mp.buttons.getKey() == '3')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 9:
	// 				if(mp.buttons.getKey() == '4')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 10:
	// 				if(mp.buttons.getKey() == '5')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 11:
	// 				if(mp.buttons.getKey() == '6')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 12:
	// 				if(mp.buttons.getKey() == '7')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 13:
	// 				if(mp.buttons.getKey() == '8')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 14:
	// 				if(mp.buttons.getKey() == '9')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 15:
	// 				if(mp.buttons.getKey() == '*')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 16:
	// 				if(mp.buttons.getKey() == '0')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			case 17:
	// 				if(mp.buttons.getKey() == '#')
	// 					buttonsArray[i] = 1;
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	mp.display.setCursor(12, 25);
	// 	mp.display.drawBitmap(33, 27, homeButton);
	// 	mp.display.drawBitmap(59, 27, powerButton);
	// 	mp.display.print("-            -   A   B");
	// 	mp.display.setCursor(10, 47);
	// 	mp.display.print("1   2   3   4   5   6");
	// 	mp.display.setCursor(10, 69);
	// 	mp.display.print("7   8   9   *   0   #");
	// 	mp.update();

	// 	if(allPressed)
	// 	{
	// 		mp.display.setTextColor(TFT_BLACK);
	// 		mp.display.setTextSize(1);
	// 		mp.display.setTextFont(2);
	// 		mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
	// 		mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
	// 		mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
	// 		mp.display.setCursor(47, 55);
	// 		mp.display.printCenter("Buttons OK!");
	// 		uint32_t tempMillis = millis();
	// 		while(millis() < tempMillis + 1500)
	// 		{
	// 			mp.update();
	// 			if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
	// 			{
	// 				while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
	// 					mp.update();
	// 				break;
	// 			}
	// 		}
	// 		while(!mp.update());
	// 		break;
	// 	}
	// }

	// // Joystick testing
	// bool tested[8] = {0,0,0,0,0,0,0,0};
	// bool allTested = 1;
	// while(1)
	// {
	// 	mp.display.fillScreen(TFT_WHITE);
	// 	mp.display.setTextFont(2);
	// 	mp.display.setTextSize(1);
	// 	mp.display.setCursor(4,4);
	// 	mp.display.setTextColor(TFT_BLACK);
	// 	mp.display.printCenter("Startup wizard");
	// 	mp.display.setCursor(3, 97);
	// 	mp.display.print("Test every direction");
	// 	mp.display.setCursor(3, 112);
	// 	mp.display.print("to continue");
	// 	mp.display.drawBitmap(74, 24, joystickArrowUp);
	// 	mp.display.drawBitmap(98, 30, joystickArrowUpRight);
	// 	mp.display.drawBitmap(100, 54, joystickArrowRight);
	// 	mp.display.drawBitmap(98, 73, joystickArrowDownRight);
	// 	mp.display.drawBitmap(74, 76, joystickArrowDown);
	// 	mp.display.drawBitmap(45, 73, joystickArrowDownLeft);
	// 	mp.display.drawBitmap(40, 54, joystickArrowLeft);
	// 	mp.display.drawBitmap(45, 30, joystickArrowUpLeft);
	// 	allTested = 1;
	// 	for(uint8_t i = 0; i < 8; i++)
	// 	{
	// 		if(tested[i])
	// 		{
	// 			switch (i)
	// 			{
	// 				case 0:
	// 					mp.display.drawBitmap(74, 24, joystickArrowUp, TFT_GREEN);
	// 					break;
	// 				case 1:
	// 					mp.display.drawBitmap(98, 30, joystickArrowUpRight, TFT_GREEN);
	// 					break;
	// 				case 2:
	// 					mp.display.drawBitmap(100, 54, joystickArrowRight, TFT_GREEN);
	// 					break;
	// 				case 3:
	// 					mp.display.drawBitmap(98, 73, joystickArrowDownRight, TFT_GREEN);
	// 					break;
	// 				case 4:
	// 					mp.display.drawBitmap(74, 76, joystickArrowDown, TFT_GREEN);
	// 					break;
	// 				case 5:
	// 					mp.display.drawBitmap(45, 73, joystickArrowDownLeft, TFT_GREEN);
	// 					break;
	// 				case 6:
	// 					mp.display.drawBitmap(40, 54, joystickArrowLeft, TFT_GREEN);
	// 					break;
	// 				case 7:
	// 					mp.display.drawBitmap(45, 30, joystickArrowUpLeft, TFT_GREEN);
	// 					break;
				
	// 			}
	// 		}
	// 		else
	// 		{
	// 			allTested = 0;
	// 			switch (i)
	// 			{
	// 				case 0:
	// 					if(mp.buttons.getJoystickY() < 100 && mp.buttons.getJoystickX() < 750 && mp.buttons.getJoystickX() > 350)
	// 						tested[i] = 1;
	// 					break;
	// 				case 1:
	// 					if(mp.buttons.getJoystickY() < 200 && mp.buttons.getJoystickX() < 200)
	// 						tested[i] = 1;
	// 					break;
	// 				case 2:
	// 					if(mp.buttons.getJoystickY() < 750 && mp.buttons.getJoystickY() > 350 && mp.buttons.getJoystickX() < 100)
	// 						tested[i] = 1;
	// 					break;
	// 				case 3:
	// 					if(mp.buttons.getJoystickY() > 900 && mp.buttons.getJoystickX() < 200)
	// 						tested[i] = 1;
	// 					break;
	// 				case 4:
	// 					if(mp.buttons.getJoystickY() > 1000 && mp.buttons.getJoystickX() < 750 && mp.buttons.getJoystickX() > 350)
	// 						tested[i] = 1;
	// 					break;
	// 				case 5:
	// 					if(mp.buttons.getJoystickY() > 900 && mp.buttons.getJoystickX() > 900)
	// 						tested[i] = 1;
	// 					break;
	// 				case 6:
	// 					if(mp.buttons.getJoystickY() < 750 && mp.buttons.getJoystickY() > 350 && mp.buttons.getJoystickX() > 1000)
	// 						tested[i] = 1;
	// 					break;
	// 				case 7:
	// 					if(mp.buttons.getJoystickY() < 200 && mp.buttons.getJoystickX() > 900)
	// 						tested[i] = 1;
	// 					break;
				
	// 			}
	// 		}
	// 	}
	// 	if(allTested)
	// 	{
	// 		mp.display.setTextColor(TFT_BLACK);
	// 		mp.display.setTextSize(1);
	// 		mp.display.setTextFont(2);
	// 		mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
	// 		mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
	// 		mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
	// 		mp.display.setCursor(47, 55);
	// 		mp.display.printCenter("Buttons OK!");
	// 		uint32_t tempMillis = millis();
	// 		while(millis() < tempMillis + 1500)
	// 		{
	// 			mp.update();
	// 			if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
	// 			{
	// 				while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
	// 					mp.update();
	// 				break;
	// 			}
	// 		}
	// 		while(!mp.update());
	// 		break;
	// 	}
	// 	mp.update();
	// }


	//SIM module testing
	mp.display.fillScreen(TFT_WHITE);
	mp.display.setTextFont(2);
	mp.display.setTextSize(1);
	mp.display.setCursor(4,4);
	mp.display.setTextColor(TFT_BLACK);
	mp.display.printCenter("Startup wizard");
	mp.display.setCursor(0,mp.display.height()/2 - 22);
	mp.display.printCenter("SIM functionality test");
	mp.display.setCursor(0,mp.display.height()/2 - 2);
	mp.display.printCenter("Performing test 1/3");
	while(!mp.update());
	uint32_t tempMillis = millis();
	String reply = "";
	bool response = 0;
	while(millis() < tempMillis + 1500)
	{
		Serial1.println("AT");
	
		reply = Serial1.readString();
		if(reply.indexOf("OK") != -1)
		{
			response = 1;
			break;
		}
	}
	if(response)
	{
		mp.display.setTextColor(TFT_BLACK);
		mp.display.setTextSize(1);
		mp.display.setTextFont(2);
		mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
		mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
		mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
		mp.display.setCursor(47, 55);
		mp.display.printCenter("AT OK!");
		tempMillis = millis();
		while(millis() < tempMillis + 1000)
			mp.update();
	}
	else
	{
		Serial1.begin(9600, SERIAL_8N1, 17, 16);
		while(millis() < tempMillis + 1500)
		{
			Serial1.println("AT");
			if(reply.indexOf("OK") != -1)
			{
				response = 1;
				break;
			}
			reply = Serial1.readString();
		}
		if(response)
		{
			mp.display.setTextColor(TFT_BLACK);
			mp.display.setTextSize(1);
			mp.display.setTextFont(2);
			mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
			mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
			mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
			mp.display.setCursor(47, 55);
			mp.display.printCenter("AT OK!");
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
		}
		else
		{
			mp.display.setTextColor(TFT_BLACK);
			mp.display.setTextSize(1);
			mp.display.setTextFont(2);
			mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
			mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
			mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
			mp.display.setCursor(47, 55);
			mp.display.printCenter("AT OK failed!");
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
		}
	}
	
	while(1)
	{
		mp.display.fillScreen(TFT_WHITE);
		mp.display.setTextFont(2);
		mp.display.setTextSize(1);
		mp.display.setCursor(4,4);
		mp.display.setTextColor(TFT_BLACK);
		mp.display.printCenter("Startup wizard");
		mp.display.setCursor(0,mp.display.height()/2 - 22);
		mp.display.printCenter("SIM functionality test");
		mp.display.setCursor(0,mp.display.height()/2 - 2);
		mp.display.printCenter("Performing test 2/3");
		while(!mp.update());
		tempMillis = millis();
		response = 0;
		reply = "";
		while(millis() < tempMillis + 1500)
		{
			Serial1.println("AT+CSQ");
		
			reply = Serial1.readString();
			Serial.println(reply);
			if(reply.indexOf("+CSQ:") != -1)
			{
				response = 1;
				break;
			}
		}
		if(response)
		{
			uint16_t foo = reply.indexOf(" ", reply.indexOf("+CSQ:")) + 1;
			if(reply.substring(foo, reply.indexOf(",", foo)).toInt() == 99)
				response = 0;
		}
		if(response)
		{
			mp.display.setTextColor(TFT_BLACK);
			mp.display.setTextSize(1);
			mp.display.setTextFont(2);
			mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
			mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
			mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
			mp.display.setCursor(47, 55);
			mp.display.printCenter("Signal OK!");
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
			break;
		}
		else
		{
			mp.display.setTextColor(TFT_BLACK);
			mp.display.setTextSize(1);
			mp.display.setTextFont(2);
			mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
			mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
			mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
			mp.display.setCursor(47, 55);
			mp.display.printCenter("Signal error!");
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
			bool blinkState = 0;
			uint32_t blinkMillis = millis();
			bool cursor = 0;
			while(!mp.buttons.released(BTN_A))
			{
				if(millis() - blinkMillis >= 350)
				{
					blinkMillis = millis();
					blinkState = !blinkState;
				}
				mp.display.fillScreen(TFT_WHITE);
				mp.display.setCursor(0, 32);
				mp.display.printCenter("Check the antenna");
				mp.display.setCursor(0,70);
				mp.display.printCenter("Retry      Skip");
				mp.display.setCursor(4, 110);
				mp.display.print("Press A to confirm");
				if(cursor)
				{
					mp.display.drawRect(98, 70, 33, 18, blinkState ? TFT_RED : TFT_WHITE);
					mp.display.drawRect(97, 69, 35, 20, blinkState ? TFT_RED : TFT_WHITE);
				}
				else
				{
					mp.display.drawRect(28, 70, 42, 18, blinkState ? TFT_RED : TFT_WHITE);
					mp.display.drawRect(27, 69, 44, 20, blinkState ? TFT_RED : TFT_WHITE);
				}

				if(mp.buttons.released(BTN_LEFT) && cursor)
				{
					blinkState = 1;
					blinkMillis = millis();
					cursor = 0;
				}
				if(mp.buttons.released(BTN_RIGHT) && !cursor)
				{
					blinkState = 1;
					blinkMillis = millis();
					cursor = 1;
				}
				mp.update();
			}
			if(cursor)
				break;
			else
			{
				mp.display.fillScreen(TFT_WHITE);
				mp.display.setCursor(0,mp.display.height()/2 - 22);
				mp.display.printCenter("Refreshing signal data");
				mp.display.setCursor(0,mp.display.height()/2 - 2);
				mp.display.printCenter("Please wait...");
				tempMillis = millis();
				while(millis() < tempMillis + 4000)
					mp.update();
			}
			
		}
	}
	while(1)
	{
		mp.display.fillScreen(TFT_WHITE);
		mp.display.setTextFont(2);
		mp.display.setTextSize(1);
		mp.display.setCursor(4,4);
		mp.display.setTextColor(TFT_BLACK);
		mp.display.printCenter("Startup wizard");
		mp.display.setCursor(0,mp.display.height()/2 - 22);
		mp.display.printCenter("SIM functionality test");
		mp.display.setCursor(0,mp.display.height()/2 - 2);
		mp.display.printCenter("Performing test 3/3");
		while(!mp.update());
		// checkSim();
		if(mp.simInserted)
		{
			mp.display.setTextColor(TFT_BLACK);
			mp.display.setTextSize(1);
			mp.display.setTextFont(2);
			mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
			mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
			mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
			mp.display.setCursor(47, 55);
			mp.display.printCenter("SIM card OK!");
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
			bool blinkState = 0;
			uint32_t blinkMillis = millis();
			bool cursor = 0;
			while(!mp.buttons.released(BTN_A))
			{
				if(millis() - blinkMillis >= 350)
				{
					blinkMillis = millis();
					blinkState = !blinkState;
				}
				mp.display.fillScreen(TFT_WHITE);
				mp.display.setCursor(0, 22);
				mp.display.printCenter("Make a test call?");
				mp.display.setCursor(0,70);
				mp.display.printCenter("Yes        No");
				mp.display.setCursor(4, 110);
				mp.display.print("Press A to confirm");
				if(cursor)
				{
					mp.display.drawRect(98, 70, 33, 18, blinkState ? TFT_RED : TFT_WHITE);
					mp.display.drawRect(97, 69, 35, 20, blinkState ? TFT_RED : TFT_WHITE);
				}
				else
				{
					mp.display.drawRect(28, 70, 42, 18, blinkState ? TFT_RED : TFT_WHITE);
					mp.display.drawRect(27, 69, 44, 20, blinkState ? TFT_RED : TFT_WHITE);
				}

				if(mp.buttons.released(BTN_LEFT) && cursor)
				{
					blinkState = 1;
					blinkMillis = millis();
					cursor = 0;
				}
				if(mp.buttons.released(BTN_RIGHT) && !cursor)
				{
					blinkState = 1;
					blinkMillis = millis();
					cursor = 1;
				}
				mp.update();
			}
			if(!cursor)
			{

			}
			break;
		}
		else
		{
			mp.display.setTextColor(TFT_BLACK);
			mp.display.setTextSize(1);
			mp.display.setTextFont(2);
			mp.display.drawRect(14, 45, 134, 38, TFT_BLACK);
			mp.display.drawRect(13, 44, 136, 40, TFT_BLACK);
			mp.display.fillRect(15, 46, 132, 36, TFT_WHITE);
			mp.display.setCursor(47, 55);
			mp.display.printCenter("SIM card not found!");
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
			tempMillis = millis();
			while(millis() < tempMillis + 1000)
				mp.update();
			bool blinkState = 0;
			uint32_t blinkMillis = millis();
			bool cursor = 0;
			while(!mp.buttons.released(BTN_A))
			{
				if(millis() - blinkMillis >= 350)
				{
					blinkMillis = millis();
					blinkState = !blinkState;
				}
				mp.display.fillScreen(TFT_WHITE);
				mp.display.setCursor(0, 16);
				mp.display.printCenter("Reinsert SIM card");
				mp.display.setCursor(0, 32);
				mp.display.printCenter("and reset device");
				mp.display.setCursor(0,70);
				mp.display.printCenter("Reset      Skip");
				mp.display.setCursor(4, 110);
				mp.display.print("Press A to confirm"); // prompt za manual turn on
				if(cursor)
				{
					mp.display.drawRect(98, 70, 33, 18, blinkState ? TFT_RED : TFT_WHITE);
					mp.display.drawRect(97, 69, 35, 20, blinkState ? TFT_RED : TFT_WHITE);
				}
				else
				{
					mp.display.drawRect(28, 70, 42, 18, blinkState ? TFT_RED : TFT_WHITE);
					mp.display.drawRect(27, 69, 44, 20, blinkState ? TFT_RED : TFT_WHITE);
				}

				if(mp.buttons.released(BTN_LEFT) && cursor)
				{
					blinkState = 1;
					blinkMillis = millis();
					cursor = 0;
				}
				if(mp.buttons.released(BTN_RIGHT) && !cursor)
				{
					blinkState = 1;
					blinkMillis = millis();
					cursor = 1;
				}
				mp.update();
			}
			if(cursor)
				break;
			else
			{
				mp.tft.setTextFont(2);
				mp.tft.setTextSize(1);
				mp.tft.setTextColor(TFT_BLACK);
				mp.tft.fillRect(12, 36, 138, 56, TFT_WHITE);
				mp.tft.setCursor(40, 51);
				mp.tft.print("Turning off...");
				delay(750);
				Serial.println("TURN OFF");
				mp.buttons.kpd.pin_mode(2, OUTPUT);
  				mp.buttons.kpd.pin_write(2, 1);
			}
			
		}
		

		mp.update();
	}
	return 0;
}

void setup()
{
	Serial.begin(115200);
	mp.begin(0);
	mp.homePopupEnable(0);
	mp.inAlarmPopup = 1;
	Serial.print("Setup: ");
	Serial.println(EEPROM.readBool(65));
	if(!EEPROM.readBool(65))
	{
		if(startupWizard())
			EEPROM.writeBool(65, 1);
		else
			EEPROM.writeBool(65, 0);
		EEPROM.commit();
	}
	osc = new Oscillator();
	osc->setVolume(256);
	addOscillator(osc);
}
void loop()
{
	lockscreen();
	mainMenu();
}
