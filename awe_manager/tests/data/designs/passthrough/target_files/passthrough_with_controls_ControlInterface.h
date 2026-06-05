/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 * 		This file is populated with symbol information only for modules
 *		that have an object ID of 30000 or greater assigned.
 *
 *          Generated on:  06-May-2024 12:36:37
 *
 ***************************************************************************/

#ifndef AWE_PASSTHROUGH_WITH_CONTROLS_CONTROLINTERFACE_H
#define AWE_PASSTHROUGH_WITH_CONTROLS_CONTROLINTERFACE_H

// ----------------------------------------------------------------------
// SubSys1 [ScalerNV2]
// Hierarchical subsystem

#define AWE_sys_volume_classID 0xBEEF0814
#define AWE_sys_volume_ID 30033

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_sys_volume_profileTime_HANDLE 0x07551007
#define AWE_sys_volume_profileTime_MASK 0x00000080
#define AWE_sys_volume_profileTime_SIZE 0x00000001

// float masterGain - Overall gain to apply.
// Default value: -19.0044
// Range: -24 to 24
#define AWE_sys_volume_masterGain_HANDLE 0x07551008
#define AWE_sys_volume_masterGain_MASK 0x00000100
#define AWE_sys_volume_masterGain_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process (0 = 
//         unsmoothed).
// Default value: 10
// Range: 0 to 1000
#define AWE_sys_volume_smoothingTime_HANDLE 0x07551009
#define AWE_sys_volume_smoothingTime_MASK 0x00000200
#define AWE_sys_volume_smoothingTime_SIZE 0x00000001

// int isDB - Selects between linear (=0) and dB (=1) operation
// Default value: 1
// Range: 0 to 1
#define AWE_sys_volume_isDB_HANDLE 0x0755100A
#define AWE_sys_volume_isDB_MASK 0x00000400
#define AWE_sys_volume_isDB_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_sys_volume_smoothingCoeff_HANDLE 0x0755100B
#define AWE_sys_volume_smoothingCoeff_MASK 0x00000800
#define AWE_sys_volume_smoothingCoeff_SIZE 0x00000001

// float trimGain[2] - Array of trim gains, one per channel
// Default value:
//     0
//     0
// Range: -24 to 24
#define AWE_sys_volume_trimGain_HANDLE 0x8755100C
#define AWE_sys_volume_trimGain_MASK 0x00001000
#define AWE_sys_volume_trimGain_SIZE 0x00000002

// float targetGain[2] - Computed target gains in linear units
#define AWE_sys_volume_targetGain_HANDLE 0x8755100D
#define AWE_sys_volume_targetGain_MASK 0x00002000
#define AWE_sys_volume_targetGain_SIZE 0x00000002

// float currentGain[2] - Instanteous gains.  These ramp towards 
//         targetGain
#define AWE_sys_volume_currentGain_HANDLE 0x8755100E
#define AWE_sys_volume_currentGain_MASK 0x00004000
#define AWE_sys_volume_currentGain_SIZE 0x00000002


// ----------------------------------------------------------------------
//  [SecondOrderFilterSmoothed]
// Top Level System

#define AWE_MYNAME_classID 0xBEEF0807
#define AWE_MYNAME_ID 30003

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_MYNAME_profileTime_HANDLE 0x07533007
#define AWE_MYNAME_profileTime_MASK 0x00000080
#define AWE_MYNAME_profileTime_SIZE 0x00000001

// int filterType - Selects the type of filter that is implemented by 
//         the module: Bypass=0, Gain=1, Butter1stLPF=2, Butter2ndLPF=3, 
//         Butter1stHPF=4, Butter2ndHPF=5, Allpass1st=6, Allpass2nd=7, 
//         Shelf2ndLow=8, Shelf2ndLowQ=9, Shelf2ndHigh=10, Shelf2ndHighQ=11, 
//         PeakEQ=12, Notch=13, Bandpass=14, Bessel1stLPF=15, Bessel1stHPF=16, 
//         AsymShelf1stLow=17, AsymShelf1stHigh=18, SymShelf1stLow=19, 
//         SymShelf1stHigh=20, VariableQLPF=21, VariableQHPF=22 Resonant=23.
// Default value: 0
// Range: 0 to 23
#define AWE_MYNAME_filterType_HANDLE 0x07533008
#define AWE_MYNAME_filterType_MASK 0x00000100
#define AWE_MYNAME_filterType_SIZE 0x00000001

// float freq - Cutoff frequency of the filter, in Hz.
// Default value: 250
// Range: 10 to 20000.  Step size = 0.1
#define AWE_MYNAME_freq_HANDLE 0x07533009
#define AWE_MYNAME_freq_MASK 0x00000200
#define AWE_MYNAME_freq_SIZE 0x00000001

// float gain - Amount of boost or cut to apply, in dB if applicable.
// Default value: 0
// Range: -24 to 24.  Step size = 0.1
#define AWE_MYNAME_gain_HANDLE 0x0753300A
#define AWE_MYNAME_gain_MASK 0x00000400
#define AWE_MYNAME_gain_SIZE 0x00000001

// float Q - Specifies the Q of the filter, if applicable.
// Default value: 1
// Range: 0 to 20.  Step size = 0.1
#define AWE_MYNAME_Q_HANDLE 0x0753300B
#define AWE_MYNAME_Q_MASK 0x00000800
#define AWE_MYNAME_Q_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process.
// Default value: 10
// Range: 0 to 1000.  Step size = 1
#define AWE_MYNAME_smoothingTime_HANDLE 0x0753300C
#define AWE_MYNAME_smoothingTime_MASK 0x00001000
#define AWE_MYNAME_smoothingTime_SIZE 0x00000001

// int updateActive - Specifies whether the filter coefficients are 
//         updating (=1) or fixed (=0).
// Default value: 1
// Range: 0 to 1
#define AWE_MYNAME_updateActive_HANDLE 0x0753300D
#define AWE_MYNAME_updateActive_MASK 0x00002000
#define AWE_MYNAME_updateActive_SIZE 0x00000001

// float b0 - Desired first numerator coefficient.
#define AWE_MYNAME_b0_HANDLE 0x0753300E
#define AWE_MYNAME_b0_MASK 0x00004000
#define AWE_MYNAME_b0_SIZE 0x00000001

// float b1 - Desired second numerator coefficient.
#define AWE_MYNAME_b1_HANDLE 0x0753300F
#define AWE_MYNAME_b1_MASK 0x00008000
#define AWE_MYNAME_b1_SIZE 0x00000001

// float b2 - Desired third numerator coefficient.
#define AWE_MYNAME_b2_HANDLE 0x07533010
#define AWE_MYNAME_b2_MASK 0x00010000
#define AWE_MYNAME_b2_SIZE 0x00000001

// float a1 - Desired second denominator coefficient.
#define AWE_MYNAME_a1_HANDLE 0x07533011
#define AWE_MYNAME_a1_MASK 0x00020000
#define AWE_MYNAME_a1_SIZE 0x00000001

// float a2 - Desired third denominator coefficient.
#define AWE_MYNAME_a2_HANDLE 0x07533012
#define AWE_MYNAME_a2_MASK 0x00040000
#define AWE_MYNAME_a2_SIZE 0x00000001

// float current_b0 - Instantaneous first numerator coefficient.
#define AWE_MYNAME_current_b0_HANDLE 0x07533013
#define AWE_MYNAME_current_b0_MASK 0x00080000
#define AWE_MYNAME_current_b0_SIZE 0x00000001

// float current_b1 - Instantaneous second numerator coefficient.
#define AWE_MYNAME_current_b1_HANDLE 0x07533014
#define AWE_MYNAME_current_b1_MASK 0x00100000
#define AWE_MYNAME_current_b1_SIZE 0x00000001

// float current_b2 - Instantaneous third numerator coefficient.
#define AWE_MYNAME_current_b2_HANDLE 0x07533015
#define AWE_MYNAME_current_b2_MASK 0x00200000
#define AWE_MYNAME_current_b2_SIZE 0x00000001

// float current_a1 - Instantaneous second denominator coefficient.
#define AWE_MYNAME_current_a1_HANDLE 0x07533016
#define AWE_MYNAME_current_a1_MASK 0x00400000
#define AWE_MYNAME_current_a1_SIZE 0x00000001

// float current_a2 - Instantaneous third denominator coefficient.
#define AWE_MYNAME_current_a2_HANDLE 0x07533017
#define AWE_MYNAME_current_a2_MASK 0x00800000
#define AWE_MYNAME_current_a2_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient. This is computed based 
//         on the smoothingTime, sample rate, and block size of the module.
#define AWE_MYNAME_smoothingCoeff_HANDLE 0x07533018
#define AWE_MYNAME_smoothingCoeff_MASK 0x01000000
#define AWE_MYNAME_smoothingCoeff_SIZE 0x00000001

// float state[4] - State variables. 2 per channel.
#define AWE_MYNAME_state_HANDLE 0x87533019
#define AWE_MYNAME_state_MASK 0x02000000
#define AWE_MYNAME_state_SIZE 0x00000004


// ----------------------------------------------------------------------
//  [MuteSmoothed]
// Top Level System

#define AWE_FullMute_classID 0xBEEF081C
#define AWE_FullMute_ID 30000

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_FullMute_profileTime_HANDLE 0x07530007
#define AWE_FullMute_profileTime_MASK 0x00000080
#define AWE_FullMute_profileTime_SIZE 0x00000001

// int isMuted - Boolean that controls muting/unmuting.
// Default value: 0
// Range: 0 to 1
#define AWE_FullMute_isMuted_HANDLE 0x07530008
#define AWE_FullMute_isMuted_MASK 0x00000100
#define AWE_FullMute_isMuted_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process
// Default value: 10
// Range: 0 to 1000
#define AWE_FullMute_smoothingTime_HANDLE 0x07530009
#define AWE_FullMute_smoothingTime_MASK 0x00000200
#define AWE_FullMute_smoothingTime_SIZE 0x00000001

// float currentGain - Instantaneous gain applied by the module.  This 
//         is also the starting gain of the module.
#define AWE_FullMute_currentGain_HANDLE 0x0753000A
#define AWE_FullMute_currentGain_MASK 0x00000400
#define AWE_FullMute_currentGain_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_FullMute_smoothingCoeff_HANDLE 0x0753000B
#define AWE_FullMute_smoothingCoeff_MASK 0x00000800
#define AWE_FullMute_smoothingCoeff_SIZE 0x00000001

// float gain - Target gain.
#define AWE_FullMute_gain_HANDLE 0x0753000C
#define AWE_FullMute_gain_MASK 0x00001000
#define AWE_FullMute_gain_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [RMSFract32]
// Top Level System

#define AWE_RMSCheck_classID 0xBEEF08AE
#define AWE_RMSCheck_ID 30005

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_RMSCheck_profileTime_HANDLE 0x07535007
#define AWE_RMSCheck_profileTime_MASK 0x00000080
#define AWE_RMSCheck_profileTime_SIZE 0x00000001

// float smoothingTime - Time interval over which to smooth the 
//         measurement
// Default value: 1000
// Range: 0 to 10000
#define AWE_RMSCheck_smoothingTime_HANDLE 0x07535008
#define AWE_RMSCheck_smoothingTime_MASK 0x00000100
#define AWE_RMSCheck_smoothingTime_SIZE 0x00000001

// fract32 instantaneousValue - Instantaneous (unsmoothed) output value
#define AWE_RMSCheck_instantaneousValue_HANDLE 0x07535009
#define AWE_RMSCheck_instantaneousValue_MASK 0x00000200
#define AWE_RMSCheck_instantaneousValue_SIZE 0x00000001

// fract32 filteredValue - Smoothed output value
#define AWE_RMSCheck_filteredValue_HANDLE 0x0753500A
#define AWE_RMSCheck_filteredValue_MASK 0x00000400
#define AWE_RMSCheck_filteredValue_SIZE 0x00000001

// fract32 a1 - a1 coefficient of 1st order smoothing filter
#define AWE_RMSCheck_a1_HANDLE 0x0753500B
#define AWE_RMSCheck_a1_MASK 0x00000800
#define AWE_RMSCheck_a1_SIZE 0x00000001

// fract32 b0 - b0 coefficient of 1st order smoothing filter
#define AWE_RMSCheck_b0_HANDLE 0x0753500C
#define AWE_RMSCheck_b0_MASK 0x00001000
#define AWE_RMSCheck_b0_SIZE 0x00000001

// fract32 b1 - b1 coefficient of 1st order smoothing filter
#define AWE_RMSCheck_b1_HANDLE 0x0753500D
#define AWE_RMSCheck_b1_MASK 0x00002000
#define AWE_RMSCheck_b1_SIZE 0x00000001

// fract32 xNm1 - Delayed input sample
#define AWE_RMSCheck_xNm1_HANDLE 0x0753500E
#define AWE_RMSCheck_xNm1_MASK 0x00004000
#define AWE_RMSCheck_xNm1_SIZE 0x00000001

// fract32 yNm1 - Delayed output sample
#define AWE_RMSCheck_yNm1_HANDLE 0x0753500F
#define AWE_RMSCheck_yNm1_MASK 0x00008000
#define AWE_RMSCheck_yNm1_SIZE 0x00000001


#define AWE_OBJECT_FOUND 0

#endif // AWE_PASSTHROUGH_WITH_CONTROLS_CONTROLINTERFACE_H

