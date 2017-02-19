/*
 * CAN.h
 *
 *  Created on: Jan 26, 2016
 *      Author: Sean Harrington
 */

#ifndef BATTERY_BALANCER_OLD_PROJECT_INCLUDES_CAN_H_
#define BATTERY_BALANCER_OLD_PROJECT_INCLUDES_CAN_H_

//---------------------------------------------------------------------------------
// Common Includes

#include "Common_Includes.h"


//---------------------------------------------------------------------------------
// Public variables

// Define table of valid CAN IDs battery balancer is looking to receive

#define CELLS_1_TO_4_ID			783
#define CELLS_5_TO_8_ID			784
#define CELLS_9_TO_12_ID		785
#define CELLS_13_TO_16_ID		786
#define CELLS_17_TO_20_ID		787
#define CELLS_21_TO_24_ID		788
#define CELLS_25_TO_28_ID		789
#define CELLS_29_TO_32_ID		790
#define CELLS_33_TO_36_ID		791
#define CELLS_37_TO_40_ID		792
#define CELLS_41_TO_44_ID		793
#define CELLS_45_TO_46_ID		794
#define CELLS_47_TO_50_ID		795
#define CELLS_51_TO_54_ID		796
#define CELLS_55_TO_58_ID		797
#define CELLS_59_TO_62_ID		798
#define CELLS_63_TO_66_ID		799
#define CELLS_67_TO_70_ID		800
#define CELLS_71_TO_74_ID		801
#define CELLS_75_TO_78_ID		802
#define CELLS_79_TO_82_ID		803
#define CELLS_83_TO_86_ID		804
#define CELLS_87_TO_90_ID		805
#define CELLS_91_TO_94_ID		806
#define CELLS_95_TO_98_ID		807
#define CELLS_99_TO_102_ID		808
#define CELLS_103_TO_106_ID		809
#define CELLS_107_TO_110_ID		810
#define CELLS_111_TO_114_ID		811
#define CELLS_115_TO_118_ID		812
#define CELLS_119_TO_122_ID		813
#define CELLS_123_TO_126_ID		814
#define CELLS_127_TO_130_ID		815
#define CELLS_131_TO_134_ID		816

typedef struct
{
	uint8_t CellSel1;
	uint8_t CellSel2;
	uint8_t CellSel3;
	uint8_t CellSel4;
} cells_t;

typedef struct
{
	Uint32 ID1;
	Uint32 ID2;
	cells_t Active_Cells;
	Bool ID1_Active;
} can_bim_mailbox_t;




//---------------------------------------------------------------------------------

Void CAN_Init();

// Setup current as ECANA_INT1 like MCN_Software
Void CAN_Receive_Interrupt();

Void SendCAN();

#endif /* BATTERY_BALANCER_OLD_PROJECT_INCLUDES_CAN_H_ */
