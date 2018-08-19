/**
 *  Modbus master example 2:
 *  The purpose of this example is to query an array of data
 *  from an external Modbus slave device.
 *  This example is similar to "simple_master", but this example
 *  allows you to use software serial instead of hardware serial
 *  in case that you want to use D1 & D2 for other purposes.
 *  The link media can be USB or RS232.

  The circuit:
 * software serial rx(D3) connect to tx pin of another device
 * software serial tx(D4) connect to rx pin of another device

 * In this example, we will use two important methods so that we can use
 * software serial.
 *
 * 1. Modbus::Modbus(uint8_t u8id)
 * This is a constructor for a Master/Slave through USB/RS232C via software serial
 * This constructor only specifies u8id (node address) and should be only
 * used if you want to use software serial instead of hardware serial.
 * This method is called if you create a ModBus object with only on parameter "u8id"
 * u8id is the node address of the arduino that will be programmed on,
 * 0 for master and 1..247 for slave
 * for example: Modbus master(0);
 * If you use this constructor you have to begin ModBus object by
 * using "void Modbus::begin(SoftwareSerial *softPort, long u32speed)".
 *
 * 2. void Modbus::begin(SoftwareSerial *sPort, long u32speed)
 * Initialize class object.
 * This is the method you have to use if you construct the ModBus object by using
 * Modbus::Modbus(uint8_t u8id) in order to use software serial and to avoid problems.
 * You have to create a SoftwareSerial object on your own, as shown in the example.
 * sPort is a pointer to your SoftwareSerial object, u32speed is the baud rate, in
 * standard increments (300..115200)
 created long time ago
 by smarmengol
 modified 29 July 2016
 by Helium6072
 This example code is in the public domain.
 */

#include <ModbusRtu.h>

extern void processSlavePing(uint8_t slaveId, boolean isOk);

// data array for modbus network sharing
uint16_t au16data[16];
uint8_t u8state;

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for master, = 1..247 for slave
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI
 *               or any pin number > 1 for RS-485
 */
Modbus master(0); // this is master and RS-232 or USB-FTDI via software serial

/**
 * This is an structe which contains a query to an slave device
 */
modbus_t telegram;

unsigned long u32wait;
uint8_t dstSlaveId = 1;

#define SSerialGND       10
#define SSerialRX        9  //Serial Receive pin
#define SSerialTX        8  //Serial Transmit pin
#define SSerialVCC       7
#define SSerialTxControl 6   //RS485 Direction control
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX


void modbus_setup() {
	Serial.println("ModBus Master");

#ifdef EMULATE_RS3485_POWER_PINS
	pinMode(SSerialVCC, OUTPUT);
	digitalWrite(SSerialVCC, HIGH);
	pinMode(SSerialGND, OUTPUT);
	digitalWrite(SSerialGND, LOW);
	delay(10);
#endif

	pinMode(SSerialTxControl, OUTPUT);
	digitalWrite(SSerialTxControl, LOW);

	// begin the ModBus object. The first parameter is the address of your SoftwareSerial address.
	// Do not forget the "&". 9600 means baud-rate at 9600
	master.begin(&RS485Serial, 31250);
	master.setTimeOut(300); // if there is no answer in 2000 ms, roll over
	u32wait = millis() + 1000;
	u8state = 0;
}

void modbus_loop() {
	switch (u8state) {
		case 0:
			if (millis() > u32wait) u8state++; // wait state
			break;
		case 1:
			au16data[0] = 0xFF;

			telegram.u8id = dstSlaveId; // slave address
			telegram.u8fct = 3; // function code (this one is registers read)
			telegram.u16RegAdd = 0; // start address in slave
			telegram.u16CoilsNo = 1; // number of elements (coils or registers) to read
			telegram.au16reg = au16data; // pointer to a memory array in the Arduino

			digitalWrite(SSerialTxControl, HIGH);
			master.query(telegram); // send query (only once)
			digitalWrite(SSerialTxControl, LOW);

			u8state++;
			break;
		case 2:
			master.poll(); // check incoming messages
			if (master.getState() == COM_IDLE) {
				u8state = 0;
				u32wait = millis() + 2000;
				Serial.print(dstSlaveId);
				Serial.print(": ");
				Serial.print(au16data[0]);
				Serial.print(" LastErr: ");
				Serial.print(master.getLastError());
				Serial.print(" ErrCount: ");
				Serial.println(master.getErrCnt());

				processSlavePing(dstSlaveId, au16data[0] != 0xFF);

				dstSlaveId++;
				if (dstSlaveId > 7)
					dstSlaveId = 1;
			}
			break;
	}
}