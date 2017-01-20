#include <stdio.h>
#include <all.h>

void charge_balance_state(float[] cell_voltages, int CHARGE_OFF_SAFETY_MAX_VOLT, int PACK_SIZE, int CHARGE_ON_MAX_VOLT, int CHARGE_ON_VOLT_TOL, int BALANCE_MIN_VOLT_THRESH, int BALANCE_ON_VOLT_TOL, int BALANCE_OFF_VOLT_TOL, int cell_voltage_validity, bool CHARGE_DISABLE){

  bool *charger_state;
  bool *safety_tripped;

  bool charge = true;

  //Charger control
  if ( safety_tripped || CHARGE_DISABLE) {
    charge = false; 
  }
  else {

    if (!charger_state) {
      //charger currently off
      for (int i = 0; i < (int) sizeof(cell_voltages); i++){
	if (cell_voltages[i] > CHARGE_ON_MAX_VOLT - CHARGE_ON_VOLT_TOL){
	  charge = false;
	}
      }
    }
    else {
      //charger currently on
      for (int i = 0; i < (int) sizeof(cell_voltages); i++){
	if (cell_voltages[i] > CHARGE_ON_MAX_VOLT){
	  charge = false;
	}
      }
    }
  }

  //disable charging permanently if safety check fails
  for (int i = 0; i < (int) sizeof(cell_voltages) && charge; i++){
    if (cell_voltages[i] > CHARGE_OFF_SAFETY_MAX_VOLT){
      charge = false;
      safety_tripped = true;
    }
  }

  //balance control - need to go through this part thouroughly
  float minCellVolt = cell_voltages[0];
  for (int i = 0; i < (int) sizeof(cell_voltages); i++){
    if (cell_voltages[i] < minCellVolt){
      minCellVolt = cell_voltages[i];
    }
  }

  charger_state = charge;
  balance_state = balance;

}
