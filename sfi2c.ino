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

#define HALF_CYCLE_DELAY delayMicroseconds(5)

//100 000 urinih ciklov na sekundo = 100 khz
//100 khz -> 0.01 ms = 10us

void i2c_start() {
	SDA_LOW;
	HALF_CYCLE_DELAY;
	SCL_LOW;
	HALF_CYCLE_DELAY;
}
uint8_t i2c_write(uint8_t data) {
	for(uint8_t i = 0; i < 8; ++i) {
		if(data & _BV(7-i))
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
	uint8_t ack = (PINC & _BV(SDA_PIN)) == 0;
	SCL_LOW;

	if(ack) {
		HALF_CYCLE_DELAY;
	}
	SDA_LOW;

	return ack;
}

uint8_t i2c_read(uint8_t ack) {
	SDA_HIGH;
	uint8_t data = 0;
	for(uint8_t i = 0; i < 8; ++i) {
		HALF_CYCLE_DELAY;
		SCL_HIGH;
		HALF_CYCLE_DELAY;

		uint8_t bit = (PINC & _BV(SDA_PIN)) > 0;
		data |= (bit << (7-i));

		SCL_LOW;
	}
	if(ack)
		SDA_LOW;//ack mnde
	else
		SDA_HIGH;

	HALF_CYCLE_DELAY;	
	SCL_HIGH;
	HALF_CYCLE_DELAY;
	SCL_LOW;
	SDA_LOW; 

	return data;
}

void i2c_stop() {
	HALF_CYCLE_DELAY;
	SCL_HIGH;
 	HALF_CYCLE_DELAY;
	SDA_HIGH;
}

void scan_i2c_devices() {
	for(uint8_t i = 1; i < 127; ++i) {
		i2c_start();
		uint8_t stat = i2c_write(i << 1 | I2C_WRITE);
		i2c_stop();

		if(stat) {
			Serial.print("Device found: 0x");
			Serial.print(i, HEX);
			Serial.println();
		}
	}
}

void reg_write(uint8_t address, uint8_t reg, uint8_t data) {
	i2c_start();
	i2c_write(address | I2C_WRITE);
	i2c_write(reg);
	i2c_write(data);
	i2c_stop();
}
uint8_t reg_read(uint8_t address, uint8_t reg) {
	i2c_start();
	i2c_write(address | I2C_WRITE);
	i2c_write(reg);
	i2c_stop();

	i2c_start();	
	i2c_write(address | I2C_READ);
	i2c_read(0);
	i2c_stop();
}

void reg_read_arr(uint8_t address, uint8_t reg, uint8_t size, void* buffer) {
	i2c_start();
	i2c_write(address | I2C_WRITE);
	i2c_write(reg);
	i2c_stop();

	i2c_start();	
	i2c_write(address | I2C_READ);
	for(uint8_t i = 0; i < size; ++i) {
		((uint8_t*)buffer)[i] = i2c_read( i < (size-1) );
	}
	i2c_stop();
}

void mpu_init() {
	reg_write(0x68 << 1, 0x6B, 0x00);
	reg_write(0x68 << 1, 0x1B, 0x08); //FS_SEL
	reg_write(0x68 << 1, 0x1C, 0x10); //AFS_SEL
	reg_write(0x68 << 1, 0x1A, 0x03); //LOW PASS FILTER (DLPF_CFG)
}

void setup() {
	Serial.begin(115200);	
	Serial.println("OK");
	delay(100);
	mpu_init();
}

void loop() {
//	reg_write(0x68 << 1, 0x1B, 0x08);
	uint8_t mpu_data[14] = {0};
	reg_read_arr(0x68 << 1, 0x3B, 14, mpu_data); 

	int16_t x = mpu_data[8] << 8 | mpu_data[9];
	int16_t y = mpu_data[10] << 8 | mpu_data[11];
	int16_t z = mpu_data[12] << 8 | mpu_data[13];

	char str[100];
	sprintf(str, "%d %d %d\n", x,y,z);
	Serial.print(str);

	//reg_read(0x68 << 1, 0x3B); 
//	Serial.print("val: ");
//	Serial.print(val, HEX);
//	Serial.println();
	delay(100);
}
