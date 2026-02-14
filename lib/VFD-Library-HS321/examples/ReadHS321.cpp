#include <Arduino.h>
#include <HS321.h>

#define rs485TransceiverReceive 4 // Вывод разрешения работы передатчика и приёмника

HS321 hs321(0x0001, Serial1, &Serial, 9600, rs485TransceiverReceive);

void testConnection();

void setup() {
	hs321.begin();
}

void loop() {
	testConnection();
}

void testConnection() {
	Serial.println("\n=== CONNECTION TEST ===");

	// Тест 1: Простой пинг
	Serial.println("1. Sending test command...");
	uint16_t value;

	if (hs321.readSingleGroupParameter(GROUP_d, 0, &value)) {
		Serial.println("*** SUCCESS: dDevice responde! ***");
		Serial.print("Value: 0x");
		Serial.println(value, HEX);
	} else {
		Serial.println("*** FAILED: No response from device ***");

		// Проверка напряжения на линиях
		Serial.println("Check:");
		Serial.println("- RS485 A/B lines connection");
		Serial.println("- Common GND");
		Serial.println("- Device power");
		Serial.println("- MAX485 power (5V)");
		Serial.println("- DE/RE pin connection");
	}

	Serial.println("=== TEST COMPLETE ===\n");
}
