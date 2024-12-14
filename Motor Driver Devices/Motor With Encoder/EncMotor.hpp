#ifndef ENCMOTOR_HPP
#define ENCMOTOR_HPP

typedef unsigned int uint;

/* Important info regarding this device:
 *
 * Motor wire order and color
 * (Based on my model. Your colors or order may be different. In parentesis I include how I connected them. CHECK YOUR DATASHEET)
 * 
 *    FROM SIDE A TO B:
 *    Green   -  Terminal A for the motor         (Connected to OUT1 on the L298N - Enabled by pPwrA connected to IN1)
 *    Blue    -  5v VCC for the encoder           (Connected to steady 5v)
 *    White   -  Direction wire from the encoder  (Connected to pDir)
 *    Yellow  -  Speed wire from the encoder      (Connected to pSpeed)
 *    Black   -  GND for the encoder              (Connected to common ground)
 *    Red     -  Terminal B for the motor         (Connected to OUT2 on the L298N - Enabled by pPwrA connected to IN2)
 * 
 * 
 * Depending on which direction the motor is wired, isClockwise may be inaccurate. If
 * this is the case then either re-wire the motor or invert isClockwise
 * 
 * As far as I can tell, there is no need to read pDir to get the direction of the motor
 * as you would know it already since you have to input isClockwise into set_direction(...).
 * If you too find this functionality useless then remove all references to pDir and remove 
 * the volatile on isClockwise, the code will work just fine.
*/

// Designed for the MG310P20 motor with a magnetic encoder and the L298N motor driver module
class EncMotor 
{
public:
  // RPM the motor is traveling at
  // Volatile due to being updated during interrupts
  volatile double rpm;
  // Current dutyPercent of the motor
  float dutyPercent;
  // Whether the motor is going clockwise or not
  // Volatile due to being updated during interrupts
  volatile bool isClockwise;

  volatile float revs = 0;

  // Initialize the class members, PWM, and IRQ
  // @param pPwm Pin for PWMing the motor
  // @param pPwrA Pin to power the A side of the motor
  // @param pPwrB Pin to power the B side of the motor
  // @param pDir Pin from the encoder that signals direction
  // @param pSpeed Pin from the encoder that interrupts the CPU to calculate speed
  EncMotor(uint pPwm, uint pPwrA, uint pPwrB, uint pDir, uint pSpeed);

  // Set motor direction
  void set_direction(const bool isClockwise);

  // Set the PWM duty percent
  void set_dutyPercent(const float dutyPercent);

  // Stop the motor
  void stop();

  // Get the direction input pin (needed in callback for the IRQ)
  uint get_pDir() { return pDir; }

private:
  uint pPwrA, pPwrB; // Output pins to power sides A or B of the motor
  uint pDir; // Input pin for the motor direction
  uint pwmSlice, pwmChannel; // The PWM slice and channel
};

#endif // ENCMOTOR_HPP