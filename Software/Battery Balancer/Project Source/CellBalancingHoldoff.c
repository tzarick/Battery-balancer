#include <msp430.h> 
//#include <stdlib.h>

/*Cell balancing Holdoff block rough draft code segment for battery balancer, Buckeye Current- 1/12/17
 * by: Tommy Zarick
 * main.c
 */
int main(void) {
  WDTCTL = WDTPW | WDTHOLD;// Stop watchdog timer
  
  CellBalancingHoldoff(balanceCommand, balanceRelaxationTime); // call fxn or whatever

  return 0;
}


// function defs
bool CellBalancingHoldoff(balanceCommand, balanceRelaxationTime) {
  const int relaxationConst = 1; // Could just use 1 as a magic number instead of using a var, prolly
  const int step = 5; // This is what the simulink shows but why is it 5?
  bool cellVoltageValidity = false;

  if (Integrator(balanceCommand, relaxationConst, step) >= balanceRelaxationTime) {
    cellVoltageValidity = true; //is this supposed to be a boolean or a number value???
  }
  return cellVoltageValidity;
}

double Integrator(balanceCommand, relaxationConstant, step) {
  // some math function goes here
}
