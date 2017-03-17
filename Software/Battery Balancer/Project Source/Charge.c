//
//  Charge.c
//  
//
//  Created by David Moeller on 3/16/17.
//
//

#include "Charge.h"

/*
 Inputs:
    -Charge Enable
    -Cell Voltages
 Outputs:
    -Charge State
*/


/*
 Description:
    -This function should be activated once bulk charging is enabled. 
    -Function is continuously called until one of the cell voltages reach the max value
 */

state_t Bulk_Charge(state_t state){
    
    
    if (state == CHARGE){
        
        //check cells as long as the state is CHARGE
        for (int i = 0; i < CELLS_IN_SERIES; i++){
            
            //check if any cells are over max voltage
            if (cells[i].voltage >= MAX_CELL_VOLTAGE){
                
                //set state to stop if so
                state = STOP;
            }
        }
    }
    
    //return state (setState should be in complimented with this function)
    return state;
    
}

