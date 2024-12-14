# Pico Devices
This is a c++ repository of all the device classes I wrote for the Raspberry Pi Pico. Any implementing PIO will say so in the name. 

## Extra info
* Variables starting with 'p' (such as `pData`) are the integer pointers for GPIO pins
* 'p' variables that are of type `int` instead of `unsigned int` can be passed -1 if they are not in use wich will skip their intialization (likely changing some functionalities in the class)


Feel free to modify, implement, and publish however you please.
