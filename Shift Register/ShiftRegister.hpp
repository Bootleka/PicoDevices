#ifndef SHIFTREGISTER_HPP
#define SHIFTREGISTER_HPP

typedef unsigned int uint;
typedef unsigned char uint8_t;

// Designed for the the (74HC595||74HCT595) 8-bit serial in shift register with output latches

/* Tips for the register:
 *
 * To clear register on startup without pins:
 * 		You can put a capacitor (100nF) from the MR pin to ground
 * 		and a resistor (10K) from the MR pin to 5V
 * 		[Also add a Diode (1N4148) from MR to 5V ?]
 *		Solution from: https://forum.arduino.cc/t/prevent-shift-register-to-start-at-random-state/488244/5
 *
 *		Havent tested above tip but it seems useful
 */

class ShiftRegister
{
public:
	// Automatically call store after each write(...) and clear()
	bool doAutoStore = true;

	/* Important info about initialization:
	 *
	 * If pReclear is valid  ->  the SHIFT register will initialize cleared (STORAGE register will maintain its value).
	 * If pEnable is valid   ->  the register will initialize enabled.
	 * 
	 * This is because pico gpio pins intialize LOW so external pull up resisters would be needed to avoid above behavior
	 * Possibly you could decide to power the regiser (likely using a transistor) after setting pins to what you want.
	*/

	// Initialize all pins and vars, running setup commands as defined by bool params
	//
	// 	Pins pReclear and pEnabled can be passed -1 to skip their initialization.
	//   clear() will use pReclear if it is valid, otherwise it will write(0) to clear.
	//   set_enabled(...) will do nothing if pEnabled is not valid.
	//
	// @param doAutostore Automatically call store after each write(...) and clear()
	// @param initCleared Automatically clear the shift register and push it to the storage register in initialization (aka clears the entire register)
	ShiftRegister(uint pData, uint pShift, uint pStore, int pReclear, int pEnable, bool doAutoStore, bool initCleared);

	// Write a byte to the shift register
	// If doAutoStore is true it will automatically update the storage register
	void write(uint8_t byte);
	
	// Write a single bit to the shift register
	// Does NOT call store()
	void write_bit(bool bit);

	// Store the contents of the shift register to the storage register
	void store();

	// Clear the shift register
	// [if pReclear is not -1] Clear the shift register using pReclear
	// OR
	// [if pReclear is -1] Clear the shift register using write(0)
	//
	// If doAutoStore is true it will automatically update the storage register
	void clear();
	
	// [if pEnable is not -1] Enable or disable registor output
	void set_enabled(bool isEnabled);
private:
	int pData; // Serial data input pin for the register
	int pShift; // Shift register clock pin
	int pStore; // Sorage register clock pin
	
	/* These pins should be initialized as -1 if they are not in use: */
	int pReclear; // Master Reclear pin (LOW to clear register)
	int pEnable; // Output enable pin (LOW to enable register output)
};

#endif // SHIFTREGISTER_HPP
