/*
	Softwarski prenosljiv I2C driver.
	Probal bom naredit nekak tak da mi bo dokaj poznan API
	(Podobno kot Wire)
	Ugalvnem, najeprej bom naredo na arduinout,
	Pol bom pa probo prenesit na STM8, STM32 ...
	
	Delat mora na kirih koli dveh pinih.
	Za hitrost bom vido kak hitro lahko gre max

	Ciljam na 100khz, ce bi pa slo,
	bi blo pa fajn 400khz

*/

#define SDA_PIN 4
#define SCL_PIN 5
#define TRANSFER_SPEED_KBPS 100

#define SDA_LOW (DDRC |= _BV(SDA_PIN))
#define SCL_LOW (DDRC |= _BV(SCL_PIN))
#define SDA_HIGH (DDRC &= ~_BV(SDA_PIN))
#define SCL_HIGH (DDRC &= ~_BV(SCL_PIN))

#define I2C_READ  1
#define I2C_WRITE 0

#define CYCLE_DELAY delayMicroseconds(10)
#define HALF_CYCLE_DELAY delayMicroseconds(5)

//100 000 urinih ciklov na sekundo = 100 khz
//100 khz -> 0.01 ms = 10us

inline void i2c_start() {
	SDA_LOW;
	HALF_CYCLE_DELAY;
	SCL_LOW;
	HALF_CYCLE_DELAY;
}
inline uint8_t i2c_address_write(uint8_t address) {
	for(uint8_t i = 0; i < 8; ++i) {
		if(address & _BV(7-i))
			SDA_HIGH;
		else
			SDA_LOW;

		HALF_CYCLE_DELAY;
		SCL_HIGH;
		HALF_CYCLE_DELAY;
		SCL_LOW;
	}

	SDA_HIGH;

	HALF_CYCLE_DELAY;	
	SCL_HIGH;
	HALF_CYCLE_DELAY;
	uint8_t ret = (PINC & _BV(SDA_PIN)) == 0;
	SCL_LOW;

	if(!ret)
		SDA_LOW;

	return ret;
}
inline void i2c_stop() {
	HALF_CYCLE_DELAY;
	SCL_HIGH;
 	HALF_CYCLE_DELAY;
	SDA_HIGH;
 }

void setup() {
	Serial.begin(115200);	
	Serial.println("OK");
}

void scan_i2c_devices() {
	for(uint8_t i = 1; i < 127; ++i) {
		i2c_start();
		uint8_t stat = i2c_address_write(i << 1 | I2C_WRITE);
		i2c_stop();

		if(stat) {
			Serial.print("Device found: 0x");
			Serial.print(i, HEX);
			Serial.println();
		}
	}
}

void loop() {
	scan_i2c_devices();
	delay(100);
}
