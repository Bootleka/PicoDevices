#include "EncMotor.hpp"

#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/timer.h>

#include <map>

// Map that relates pSpeed to a motor so that the callback function can update the classes directly
std::map<uint, EncMotor*> callbackMap;

// The callback function for IRQ
//    It translates the distance between interrupts as speed the motor has gone and reads pDir to get direction,
//    then updating speed and isClockwise to the new values
void callback(uint gpio, uint32_t events);

EncMotor::EncMotor(uint pPwm, uint pPwrA, uint pPwrB, uint pDir, uint pSpeed) 
                  : pPwrA(pPwrA), pPwrB(pPwrB), pDir(pDir) {
  // Initialize gpio pins
  gpio_init(pPwrA);
  gpio_set_dir(pPwrA, GPIO_OUT);
  gpio_init(pPwrB);
  gpio_set_dir(pPwrB, GPIO_OUT);
  gpio_init(pDir);
  gpio_set_dir(pDir, GPIO_IN);
  gpio_pull_up(pDir);
  gpio_init(pSpeed);
  gpio_set_dir(pSpeed, GPIO_IN);
  gpio_pull_up(pSpeed);

  // IRQ setup
  callbackMap[pSpeed] = this;
  gpio_set_irq_enabled_with_callback(pSpeed, GPIO_IRQ_EDGE_FALL, true, &callback);

  // PWM setup
  pwmChannel = pwm_gpio_to_channel(pPwm);
  pwmSlice = pwm_gpio_to_slice_num(pPwm);
  pwm_config cfg = pwm_get_default_config();

  gpio_set_function(pPwm, GPIO_FUNC_PWM);
  pwm_config_set_clkdiv(&cfg, 12);
  pwm_init(pwmSlice, &cfg, true);

  // Enable PWM with 0% duty cycle
  pwm_set_enabled(pwmSlice, true);
  pwm_set_chan_level(pwmSlice, pwmChannel, 0);
}

// Set motor direction (depending on which way the motor is wired, isClockwise may have to be inverted to be accurate)
void EncMotor::set_direction(const bool isClockwise) {
  // Decide which pin to power based on direction
  gpio_put(pPwrA, isClockwise);
  gpio_put(pPwrB, !isClockwise);

  this->isClockwise = isClockwise;
}

// Set the PWM duty percent (0 - 100; decimal values are ok)
//    WARNING: Low values will likely stall the motor
void EncMotor::set_dutyPercent(const float dutyPercent) {
  this->dutyPercent = dutyPercent;

  // Calculate duty cycle
  uint16_t cycle = (uint16_t)((dutyPercent * 65535) / 100);

  // Set PWM duty cycle
  pwm_set_enabled(pwmSlice, true);
  pwm_set_chan_level(pwmSlice, pwmChannel, cycle);
}

// Stop the motor by disabling pPwrA & pPwrB (PWM stays the same)
void EncMotor::stop() {
  // Disable both power pins
  gpio_put(pPwrA, 0);
  gpio_put(pPwrB, 0);
}

// The callback function for IRQ
//    It translates the distance between interrupts as speed the motor has gone and reads pDir to get direction,
//    then updating speed and isClockwise to the new values
void callback(uint gpio, uint32_t events) {
  // Set isClockwise by getting the state of pDir
  callbackMap[gpio]->isClockwise = !gpio_get(callbackMap[gpio]->get_pDir());

  // Get RPM from the delta between this time and the time of the last callback
  static uint32_t lastTime = 0;
  uint32_t thisTime = time_us_32();
  // Ensure time has passed so we dont divide by 0
  if (lastTime < thisTime) {
    uint32_t deltaTime = thisTime - lastTime;

    // Update revolutions
    callbackMap[gpio]->revs += 1 / (20.4 * 14);
    // Turn microseconds into RPM
    callbackMap[gpio]->rpm = 60000000.0f / (deltaTime * 20.4 * 14);
  }

  lastTime = thisTime;
};