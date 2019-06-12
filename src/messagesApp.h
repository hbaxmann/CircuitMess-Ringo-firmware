#ifndef messages_h
#define messages_h

#include "main.h"
extern int16_t y;
extern String smsContent[smsNumber];
extern String phoneNumber[smsNumber];
extern String tempDate[smsNumber];
extern uint16_t smsYear[smsNumber];
extern uint8_t smsDay[smsNumber];
extern uint8_t smsMonth[smsNumber];
extern uint8_t smsMinute[smsNumber];
extern uint8_t smsSecond[smsNumber];
extern uint8_t smsHour[smsNumber];
extern uint32_t start;
extern uint32_t end;
extern String input;
extern String buffer;
extern int textPointer;

uint16_t countSubstring(String string, String substring);
String readSerial();
String readSms(uint8_t index);
String readAllSms();
void viewSms(String content, String contact, uint32_t date, bool direction);
void smsMenuDrawBox(String contact, DateTime date, String content, uint8_t i, int32_t y);
void smsMenuComposeBox(uint8_t i, int32_t y);
void smsMenuDrawCursor(uint8_t i, int32_t y);
void smsMenuComposeBoxCursor(uint8_t i, int32_t y);
int16_t smsMenu(JsonArray& messages, int16_t prevCursor);
void messagesApp();
void composeSMS(JsonArray *messages);
void incomingMessagePopup();

#endif
