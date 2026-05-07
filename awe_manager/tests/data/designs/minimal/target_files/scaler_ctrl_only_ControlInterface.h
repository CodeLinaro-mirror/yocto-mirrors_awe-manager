/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 * 		This file is populated with symbol information only for modules
 *		that have an object ID of 30000 or greater assigned.
 *
 *          Generated on:  13-May-2024 10:22:51
 *
 ***************************************************************************/

#ifndef AWE_SCALER_CTRL_ONLY_CONTROLINTERFACE_H
#define AWE_SCALER_CTRL_ONLY_CONTROLINTERFACE_H

// ----------------------------------------------------------------------
//  [ScalerV2]
// Newly created subsystem

#define AWE_Scaler1_classID 0xBEEF0813
#define AWE_Scaler1_ID 30000

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Scaler1_profileTime_HANDLE 0x07530007
#define AWE_Scaler1_profileTime_MASK 0x00000080
#define AWE_Scaler1_profileTime_SIZE 0x00000001

// float gain - Gain in either linear or dB units.
// Default value: 0
// Range: -24 to 24
#define AWE_Scaler1_gain_HANDLE 0x07530008
#define AWE_Scaler1_gain_MASK 0x00000100
#define AWE_Scaler1_gain_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process (0 = 
//         unsmoothed).
// Default value: 10
// Range: 0 to 1000
#define AWE_Scaler1_smoothingTime_HANDLE 0x07530009
#define AWE_Scaler1_smoothingTime_MASK 0x00000200
#define AWE_Scaler1_smoothingTime_SIZE 0x00000001

// int isDB - Selects between linear (=0) and dB (=1) operation
// Default value: 1
// Range: 0 to 1
#define AWE_Scaler1_isDB_HANDLE 0x0753000A
#define AWE_Scaler1_isDB_MASK 0x00000400
#define AWE_Scaler1_isDB_SIZE 0x00000001

// float targetGain - Target gain in linear units.
#define AWE_Scaler1_targetGain_HANDLE 0x0753000B
#define AWE_Scaler1_targetGain_MASK 0x00000800
#define AWE_Scaler1_targetGain_SIZE 0x00000001

// float currentGain - Instantaneous gain applied by the module.
#define AWE_Scaler1_currentGain_HANDLE 0x0753000C
#define AWE_Scaler1_currentGain_MASK 0x00001000
#define AWE_Scaler1_currentGain_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_Scaler1_smoothingCoeff_HANDLE 0x0753000D
#define AWE_Scaler1_smoothingCoeff_MASK 0x00002000
#define AWE_Scaler1_smoothingCoeff_SIZE 0x00000001


#define AWE_OBJECT_FOUND 0

#endif // AWE_SCALER_CTRL_ONLY_CONTROLINTERFACE_H

