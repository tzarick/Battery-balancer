//
//  Balance.c
//  
//
//  Created by David Moeller on 3/16/17.
//
//

#include "Balance.h"

static uint8_t LastVoltageValue;

uint8_t SelectRelay(spi_device_t device){
    
    uint8_t relay;
    
    for (uint8_t i = 0; i < RELAY_TX_BUFFER_LENGTH; i++){
        //continuously add the bit values of the queue until it reaches the length of one relay buffer
        relay += mRelayTxBuffer[mRelayTxIndex + i];
    }
    
    return relay;
    
}

int BalanceCells(){
    
    return EstimatedVoltage(current) + VoltageCalc(current) + VoltageLookUp(current);
    
}

int EstimatedVoltage(int current){
    
    //new variable
    uint8_t volts;
    
    //division operation using current (still dont understand this)
    uint8_t idc_calc;
    
    idc_calc = current / C1;
    
    //calculates the estimated voltage
    volts = idc_calc - (LastVoltageValue / (C1 * R1));
    
    LastVoltageValue = volts;
    
    return volts;
    
}

int VoltageCalc(int current){
    
    //some constant value that needs to be found
    int FirstOrderResistance = NULL;
    
    // I * R = Volts
    return current * FirstOrderResistance;
    
}

int VoltageLookUp(int current){
    
    //still not sure how to properly implement this
    
}
