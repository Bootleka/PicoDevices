#include "ShiftRegister.hpp"

#include <pico/stdlib.h>
#include <hardware/gpio.h>

/* Important info:
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
ShiftRegister::ShiftRegister(uint pData, uint pShift, uint pStore, int pReclear, int pEnable, bool doAutoStore, bool initCleared) 
					  : pData(pData), pShift(pShift), pStore(pStore), pReclear(pReclear), pEnable(pEnable), doAutoStore(doAutoStore) {
	// Initialize necessary GPIO
	gpio_init(pData);
	gpio_set_dir(pData, GPIO_OUT);
	gpio_init(pShift);
	gpio_set_dir(pShift, GPIO_OUT);
	gpio_init(pStore);
	gpio_set_dir(pStore, GPIO_OUT);

	// Initialize (or don't) optional GPIO pins
	if (pReclear != -1) {
		gpio_init(pReclear);
		gpio_set_dir(pReclear, GPIO_OUT);

		// Set reclear to 1 to allow writing of the shift register
		gpio_put(pReclear, 1);
	}
	if (pEnable != -1) {
		gpio_init(pEnable);
		gpio_set_dir(pEnable, GPIO_OUT);
	}

	// Run extra initialization commands
	// If initCleared, clear the shift register and store it
		if (initCleared) {
			clear();
			// If clear() hasnt already called store(), call it
			if (!doAutoStore) {
				store();
			}
		}
}

// Write a byte to the shift register
// If doAutoStore is true it will automatically update the storage register
void ShiftRegister::write(uint8_t byte) {
	// Ensure noting is copied to storage register during write
	gpio_put(pStore, 0);

	// For each bit in the byte
	for (int i = 7; i >= 0; i--) {
		// Get the bit
		bool bit = ((byte >> i) & 1);

		// Write the bit to the shift register
		write_bit(bit);

		// Wait 10 us before next bit
		sleep_us(10);		
	}

	if (doAutoStore) {
		// Push to storage registor
		store();
	}
}

// Write a single bit to the shift register
// Does NOT call store()
void ShiftRegister::write_bit(bool bit) {
	// Set the data pin to the bit
	gpio_put(pData, bit);

	// Pulse the clock for 10 us to set the bit
	gpio_put(pShift, 1);
	sleep_ms(10);
	gpio_put(pShift, 0);
}

// Store the contents of the shift regisror to the storage register
void ShiftRegister::store() {
	gpio_put(pStore, 1);
	sleep_us(10);
	gpio_put(pStore, 0);
}

// Clear the shift register
// [if pReclear is not -1] Clear the shift register using pReclear
// OR
// [if pReclear is -1] Clear the shift register using write(0)
//
// If doAutoStore is true it will automatically update the storage register
void ShiftRegister::clear() {
	// Do write(0) to clear if there is no reclear pin set
	if (pReclear == -1) { 
		write(0);
		return; 
	}

	// Send a 10 us low pulse to pReclear
	gpio_put(pReclear, 0);
	sleep_us(10);
	gpio_put(pReclear, 1);
	
	if (doAutoStore) {
		// Push to storage register
		store();
	}
}

// [if pEnable is not -1] Enable or disable registor output
void ShiftRegister::set_enabled(bool isEnabled) {
	// Do nothing if pEnable was skipped
	if (pEnable == -1) { return; }

	// 0 = enabled
	// 1 = disabled
	gpio_put(pEnable, !isEnabled);
}