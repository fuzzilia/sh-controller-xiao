#include "CL3GD20.h"

namespace {
	const float L3GD20_SENSITIVITY_250DPS = 0.00875f;	// Roughly 22/256 for fixed 
	const float L3GD20_SENSITIVITY_500DPS = 0.0175f;		// Roughly 22/256 for fixed 
	const float L3GD20_SENSITIVITY_2000DPS = 0.070f;		// Roughly 22/256 for fixed 
}

CL3GD20::CL3GD20(int8_t cs, int8_t miso, int8_t mosi, int8_t clk)
: m_cs(cs), m_mosi(miso), m_miso(mosi), m_clk(clk), m_range(RANGE::DPS_245), m_address(SLAVE_ADDRESS::GND)
{
}

CL3GD20::CL3GD20(void)
: m_cs(-1), m_mosi(-1), m_miso(-1), m_clk(-1), m_range(RANGE::DPS_245), m_address(SLAVE_ADDRESS::GND)
{
}

CL3GD20::~CL3GD20(void)
{
}

bool CL3GD20::begin(RANGE range, SLAVE_ADDRESS address)
{
  Serial.println(address, HEX);
	if (m_cs == -1) {
		Wire.begin();
	} else {
		pinMode(m_cs, OUTPUT);
		pinMode(m_clk, OUTPUT);
		pinMode(m_mosi, OUTPUT);
		pinMode(m_miso, INPUT);
		digitalWrite(m_cs, HIGH);
	}

	m_range = range;
	m_address = address;

	/* Make sure we have the correct chip ID since this checks
		 for correct address and that the IC is properly connected */
	const uint8_t id = read8(REGISTER::WHO_AM_I);
	//Serial.println(id, HEX);
	if ((id != (ID::L3GD20) && (id != ID::L3GD20H))) {
    Serial.println("3");
		return false;
	}

	/* Set CTRL1 (0x20)
	 ====================================================================
	 BIT  Symbol    Description                                   Default
	 ---  ------    --------------------------------------------- -------
	 7-6  DR1/0     Output data rate                                   00
	 5-4  BW1/0     Bandwidth selection                                00
		 3  PD        0 = Power-down mode, 1 = normal/sleep mode          0
		 2  ZEN       Z-axis enable (0 = disabled, 1 = enabled)           1
		 1  YEN       Y-axis enable (0 = disabled, 1 = enabled)           1
		 0  XEN       X-axis enable (0 = disabled, 1 = enabled)           1 */

	/* Switch to normal mode and enable all three channels */
	write8(REGISTER::CTRL1, 0b1111);
	/* ------------------------------------------------------------------ */

	/* Set CTRL2 (0x21)
	 ====================================================================
	 BIT  Symbol    Description                                   Default
	 ---  ------    --------------------------------------------- -------
	 5-4  HPM1/0    High-pass filter mode selection                    00
	 3-0  HPCF3..0  High-pass filter cutoff frequency selection      0000 */

	/* Nothing to do ... keep default values */
	/* ------------------------------------------------------------------ */

	/* Set CTRL3 (0x22)
	 ====================================================================
	 BIT  Symbol    Description                                   Default
	 ---  ------    --------------------------------------------- -------
		 7  I1_Int1   Interrupt enable on INT1 (0=disable,1=enable)       0
		 6  I1_Boot   Boot status on INT1 (0=disable,1=enable)            0
		 5  H-Lactive Interrupt active config on INT1 (0=high,1=low)      0
		 4  PP_OD     Push-Pull/Open-Drain (0=PP, 1=OD)                   0
		 3  I2_DRDY   Data ready on DRDY/INT2 (0=disable,1=enable)        0
		 2  I2_WTM    FIFO wtrmrk int on DRDY/INT2 (0=dsbl,1=enbl)        0
		 1  I2_ORun   FIFO overrun int on DRDY/INT2 (0=dsbl,1=enbl)       0
		 0  I2_Empty  FIFI empty int on DRDY/INT2 (0=dsbl,1=enbl)         0 */

	/* Nothing to do ... keep default values */
	/* ------------------------------------------------------------------ */

	/* Set CTRL4 (0x23)
	 ====================================================================
	 BIT  Symbol    Description                                   Default
	 ---  ------    --------------------------------------------- -------
		 7  BDU       Block Data Update (0=continuous, 1=LSB/MSB)         0
		 6  BLE       Big/Little-Endian (0=Data LSB, 1=Data MSB)          0
	 5-4  FS1/0     Full scale selection                               00
																	00 = 250 dps
																	01 = 500 dps
																	10 = 2000 dps
																	11 = 2000 dps
		 0  SIM       SPI Mode (0=4-wire, 1=3-wire)                       0 */

	/* Adjust resolution if requested */
	switch(m_range) {
		case RANGE::DPS_245:
			write8(REGISTER::CTRL4, 0b0000'0000);
			break;
		case RANGE::DPS_500:
			write8(REGISTER::CTRL4, 0b0001'0000);
			break;
		case RANGE::DPS_2000:
			write8(REGISTER::CTRL4, 0b0010'0000);
			break;
		default:
			write8(REGISTER::CTRL4, 0b0000'0000);
			break;
	}
	/* ------------------------------------------------------------------ */

	/* Set CTRL5 (0x24)
	 ====================================================================
	 BIT  Symbol    Description                                   Default
	 ---  ------    --------------------------------------------- -------
		 7  BOOT      Reboot memory content (0=normal, 1=reboot)          0
		 6  FIFO_EN   FIFO enable (0=FIFO disable, 1=enable)              0
		 4  HPen      High-pass filter enable (0=disable,1=enable)        0
	 3-2  INT1_SEL  INT1 Selection config                              00
	 1-0  OUT_SEL   Out selection config                               00 */

	/* Nothing to do ... keep default values */
	/* ------------------------------------------------------------------ */

	return true;
}

CL3GD20Result CL3GD20::read()
{
	CL3GD20Result result;
	uint8_t xhi, xlo, ylo, yhi, zlo, zhi;

	if (m_cs == -1) {
		Wire.beginTransmission(m_address);
		// Make sure to set address auto-increment bit
		Wire.write(REGISTER::OUT_X_L | 0x80);
		Wire.endTransmission();
		Wire.requestFrom(m_address, (byte)6);

		// Wait around until enough data is available
		while (Wire.available() < 6);

		xlo = Wire.read();
		xhi = Wire.read();
		ylo = Wire.read();
		yhi = Wire.read();
		zlo = Wire.read();
		zhi = Wire.read();

	} else {
		digitalWrite(m_clk, HIGH);
		digitalWrite(m_cs, LOW);

		SPIxfer(REGISTER::OUT_X_L | 0x80 | 0x40); // SPI read, autoincrement
		delay(10);
		xlo = SPIxfer(0xFF);
		xhi = SPIxfer(0xFF);
		ylo = SPIxfer(0xFF);
		yhi = SPIxfer(0xFF);
		zlo = SPIxfer(0xFF);
		zhi = SPIxfer(0xFF);

		digitalWrite(m_cs, HIGH);
	}
	// Shift values to create properly formed integer (low byte first)
	result.x = static_cast<float>((int16_t)(xlo | (xhi << 8)));
	result.y = static_cast<float>((int16_t)(ylo | (yhi << 8)));
	result.z = static_cast<float>((int16_t)(zlo | (zhi << 8)));

	// Compensate values depending on the resolution
	switch(m_range)
	{
		case RANGE::DPS_245:
			result.x *= SENSITIVITY_250DPS;
      result.y *= SENSITIVITY_250DPS;
      result.z *= SENSITIVITY_250DPS;
			break;
		case RANGE::DPS_500:
			result.x *= SENSITIVITY_500DPS;
      result.y *= SENSITIVITY_500DPS;
      result.z *= SENSITIVITY_500DPS;
			break;
		case RANGE::DPS_2000:
			result.x *= SENSITIVITY_2000DPS;
      result.y *= SENSITIVITY_2000DPS;
      result.z *= SENSITIVITY_2000DPS;
			break;
		default:
      result.x *= SENSITIVITY_250DPS;
      result.y *= SENSITIVITY_250DPS;
      result.z *= SENSITIVITY_250DPS;
			break;
	}

	return result;
}

void CL3GD20::write8(REGISTER reg, byte value)
{
	if (m_cs == -1) {
		// use i2c
		Wire.beginTransmission(m_address);
		Wire.write((byte)reg);
		Wire.write(value);
		Wire.endTransmission();
	} else {
		digitalWrite(m_clk, HIGH);
		digitalWrite(m_cs, LOW);

		SPIxfer(reg);
		SPIxfer(value);

		digitalWrite(m_cs, HIGH);
	}
}

byte CL3GD20::read8(REGISTER reg)
{
	byte value;

	if (m_cs == -1) {
		// use i2c.
		Wire.beginTransmission(m_address);
		Wire.write((byte)reg);
		Wire.endTransmission();
		Wire.requestFrom(m_address, (byte)1);
		value = Wire.read();
		Wire.endTransmission();
	} else {
		// use spi.
		digitalWrite(m_clk, HIGH);
		digitalWrite(m_cs, LOW);

		SPIxfer((uint8_t)reg | 0x80); // set READ bit
		value = SPIxfer(0xFF);

		digitalWrite(m_cs, HIGH);
	}

	return value;
}

uint8_t CL3GD20::SPIxfer(uint8_t x) {
	uint8_t value = 0;

	for (int i = 7; i >= 0; --i) {
		digitalWrite(m_clk, LOW);
		if (x & (1 << i)) {
			digitalWrite(m_mosi, HIGH);
		} else {
			digitalWrite(m_mosi, LOW);
		}
		digitalWrite(m_clk, HIGH);
		if (digitalRead(m_miso)) {
			value |= (1 << i);
		}
	}

	return value;
}
