//
//  Balance.h
//  
//
//  Created by David Moeller on 3/16/17.
//
//

#ifndef Balance_h
#define Balance_h

#include <stdio.h>


/*
 input:
    SPI device queue
 output:
    The correct relay to be turned on
 Description:
    This function parses through the relay queue until it pops off the relay that needs to be active
 */
uint8_t SelectRelay(spi_device_t device);

/*
 output:
    Voltage values from all calculations
 Description:
    This function adds the voltage values from the different equations used in the cell block diagram of the simulink model
 */
int BalanceCells();

/*
 input:
    current
 output:
    Voltage
 Description:
    This function calculates the estimated voltgae of the cell using previous voltage calues and constants
 */
int EstimatedVoltage(int current);

/*
 input:
    current
 output:
    voltage
 Description:
    This function calculates the voltage of a cell by multiplying the current going in and a constant resistance
 */
int VoltageCalc(int current);

//Still have no idea what this is supposed to do
int VoltageLookUp(int current);

#endif /* Balance_h */
