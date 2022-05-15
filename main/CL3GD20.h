#ifndef CL3GD20H_H_
#define CL3GD20H_H_

#include "Arduino.h"
#include "Wire.h"

struct CL3GD20Result;

class CL3GD20
{
public:
	static constexpr float SENSITIVITY_250DPS = 0.00875f;	// Roughly 22/256 for fixed.
	static constexpr float SENSITIVITY_500DPS = 0.0175f;	// Roughly 45/256.
	static constexpr float SENSITIVITY_2000DPS = 0.070f;	// Roughly 18/256.
	static constexpr float DPS_TO_RADS = 0.070f;			// degress/s to rad/s multiplier.

	enum ID : uint8_t {
		L3GD20 = 0b1101'0100,
		L3GD20H = 0b1101'0111,
	};

	enum SLAVE_ADDRESS : uint8_t {
		GND = 0b110'1010,
		VCC = 0b110'1011,
	};

	enum REGISTER : uint8_t {
							  // DEFAULT    TYPE
		WHO_AM_I	= 0x0F,   // 11010100   r
		CTRL1		= 0x20,   // 00000111   rw
		CTRL2		= 0x21,   // 00000000   rw
		CTRL3		= 0x22,   // 00000000   rw
		CTRL4		= 0x23,   // 00000000   rw
		CTRL5		= 0x24,   // 00000000   rw
		REFERENCE	= 0x25,   // 00000000   rw
		OUT_TEMP	= 0x26,   //            r
		STATUS		= 0x27,   //            r
		OUT_X_L		= 0x28,   //            r
		OUT_X_H		= 0x29,   //            r
		OUT_Y_L		= 0x2A,   //            r
		OUT_Y_H		= 0x2B,   //            r
		OUT_Z_L		= 0x2C,   //            r
		OUT_Z_H		= 0x2D,   //            r
		FIFO_CTRL	= 0x2E,   // 00000000   rw
		FIFO_SRC	= 0x2F,   //            r
		IG_CFG		= 0x30,   // 00000000   rw
		IG_SRC		= 0x31,   //            r
		IG_TSH_XH	= 0x32,   // 00000000   rw
		IG_TSH_XL	= 0x33,   // 00000000   rw
		IG_TSH_YH	= 0x34,   // 00000000   rw
		IG_TSH_YL	= 0x35,   // 00000000   rw
		IG_TSH_ZH	= 0x36,   // 00000000   rw
		IG_TSH_ZL	= 0x37,   // 00000000   rw
		IG_DURATION	= 0x38,   // 00000000   rw
		LOW_ODR		= 0x39,   // 00000000   rw
	};

	enum class RANGE : uint8_t {
	  DPS_245 = 0,
	  DPS_500,
	  DPS_2000,
	};

	/*!
	 * @brief Construct a new Adafruit_L3GD20 object for SPI.
	 *
	 * @param cs 
	 * @param mosi 
	 * @param miso 
	 * @param clk 
	 */
	CL3GD20(const int8_t cs, const int8_t mosi, const int8_t miso, const int8_t clk);

	/*!
	 * @brief Construct a new Adafruit_L3GD20 object for I2C.
	 */
	CL3GD20(void);

	/*!
	 * @brief Destroy the CL3GD20 object
	 */
	~CL3GD20(void);

	/*!
	 * @brief Begin connection.
	 * 
	 * @param range Full scale selection(RANGE).
	 * @param address The Slave ADdress(SLAVE_ADDRESS).
	 * @return true 
	 * @return false 
	 */
	bool begin(RANGE range = RANGE::DPS_245, SLAVE_ADDRESS address = SLAVE_ADDRESS::GND);
	CL3GD20Result read(void);

private:
	void write8(REGISTER reg, byte value);
	byte read8(REGISTER reg);
	uint8_t SPIxfer(uint8_t x);

private:
	RANGE m_range;
	SLAVE_ADDRESS m_address;
	int8_t m_miso;
	int8_t m_mosi;
	int8_t m_clk;
	int8_t m_cs;
};

struct CL3GD20Result {
	CL3GD20Result()
	: x(-1), y(-1), z(-1)
	{}

	float x;
	float y;
	float z;
};

#endif
