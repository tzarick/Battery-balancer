//
//  Charge.h
//  
//
//  Created by David Moeller on 3/16/17.
//
//

#ifndef Charge_h
#define Charge_h

#include <stdio.h>

/*
 Description:
 -This function should be activated once bulk charging is enabled.
 -Function is continuously called until one of the cell voltages reach the max value
 */
state_t Bulk_Charge(state_t state);

#endif /* Charge_h */