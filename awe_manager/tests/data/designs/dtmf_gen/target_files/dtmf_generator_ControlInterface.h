/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 * 		This file is populated with symbol information only for modules
 *		that have an object ID of 30000 or greater assigned.
 *
 *          Generated on:  08-May-2024 08:35:27
 *
 ***************************************************************************/

#ifndef AWE_DTMF_GENERATOR_CONTROLINTERFACE_H
#define AWE_DTMF_GENERATOR_CONTROLINTERFACE_H

// ----------------------------------------------------------------------
//  [SineSmoothedGen]
// Top Level System

#define AWE_Sine_Column_classID 0xBEEF0879
#define AWE_Sine_Column_ID 30002

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Sine_Column_profileTime_HANDLE 0x07532007
#define AWE_Sine_Column_profileTime_MASK 0x00000080
#define AWE_Sine_Column_profileTime_SIZE 0x00000001

// float freq - Frequency of the sine wave, in Hz.
// Default value: 697
// Range: 0.01 to 24000
#define AWE_Sine_Column_freq_HANDLE 0x07532008
#define AWE_Sine_Column_freq_MASK 0x00000100
#define AWE_Sine_Column_freq_SIZE 0x00000001

// float smoothingTime - Time constant of the frequency adjustment 
//         operation, in msec.
// Default value: 10
// Range: 0 to 1000
#define AWE_Sine_Column_smoothingTime_HANDLE 0x07532009
#define AWE_Sine_Column_smoothingTime_MASK 0x00000200
#define AWE_Sine_Column_smoothingTime_SIZE 0x00000001

// float startPhase - Starting phase of the sine wave, in degrees.
// Default value: 0
// Range: 0 to 360
#define AWE_Sine_Column_startPhase_HANDLE 0x0753200A
#define AWE_Sine_Column_startPhase_MASK 0x00000400
#define AWE_Sine_Column_startPhase_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_Sine_Column_smoothingCoeff_HANDLE 0x0753200B
#define AWE_Sine_Column_smoothingCoeff_MASK 0x00000800
#define AWE_Sine_Column_smoothingCoeff_SIZE 0x00000001

// float phase - Instantanteous phase and also starting phase.
// Default value: 0
// Range: unrestricted
#define AWE_Sine_Column_phase_HANDLE 0x0753200C
#define AWE_Sine_Column_phase_MASK 0x00001000
#define AWE_Sine_Column_phase_SIZE 0x00000001

// float phaseIncTarget - Target for the sample to sample phase 
//         increment. Essentially the target frequency.
#define AWE_Sine_Column_phaseIncTarget_HANDLE 0x0753200D
#define AWE_Sine_Column_phaseIncTarget_MASK 0x00002000
#define AWE_Sine_Column_phaseIncTarget_SIZE 0x00000001

// float phaseInc - Instantaneous sample to sample phase increment.
#define AWE_Sine_Column_phaseInc_HANDLE 0x0753200E
#define AWE_Sine_Column_phaseInc_MASK 0x00004000
#define AWE_Sine_Column_phaseInc_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [SineSmoothedGen]
// Top Level System

#define AWE_Sine_Row_classID 0xBEEF0879
#define AWE_Sine_Row_ID 30003

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Sine_Row_profileTime_HANDLE 0x07533007
#define AWE_Sine_Row_profileTime_MASK 0x00000080
#define AWE_Sine_Row_profileTime_SIZE 0x00000001

// float freq - Frequency of the sine wave, in Hz.
// Default value: 1209
// Range: 0.01 to 24000
#define AWE_Sine_Row_freq_HANDLE 0x07533008
#define AWE_Sine_Row_freq_MASK 0x00000100
#define AWE_Sine_Row_freq_SIZE 0x00000001

// float smoothingTime - Time constant of the frequency adjustment 
//         operation, in msec.
// Default value: 10
// Range: 0 to 1000
#define AWE_Sine_Row_smoothingTime_HANDLE 0x07533009
#define AWE_Sine_Row_smoothingTime_MASK 0x00000200
#define AWE_Sine_Row_smoothingTime_SIZE 0x00000001

// float startPhase - Starting phase of the sine wave, in degrees.
// Default value: 0
// Range: 0 to 360
#define AWE_Sine_Row_startPhase_HANDLE 0x0753300A
#define AWE_Sine_Row_startPhase_MASK 0x00000400
#define AWE_Sine_Row_startPhase_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_Sine_Row_smoothingCoeff_HANDLE 0x0753300B
#define AWE_Sine_Row_smoothingCoeff_MASK 0x00000800
#define AWE_Sine_Row_smoothingCoeff_SIZE 0x00000001

// float phase - Instantanteous phase and also starting phase.
// Default value: 0
// Range: unrestricted
#define AWE_Sine_Row_phase_HANDLE 0x0753300C
#define AWE_Sine_Row_phase_MASK 0x00001000
#define AWE_Sine_Row_phase_SIZE 0x00000001

// float phaseIncTarget - Target for the sample to sample phase 
//         increment. Essentially the target frequency.
#define AWE_Sine_Row_phaseIncTarget_HANDLE 0x0753300D
#define AWE_Sine_Row_phaseIncTarget_MASK 0x00002000
#define AWE_Sine_Row_phaseIncTarget_SIZE 0x00000001

// float phaseInc - Instantaneous sample to sample phase increment.
#define AWE_Sine_Row_phaseInc_HANDLE 0x0753300E
#define AWE_Sine_Row_phaseInc_MASK 0x00004000
#define AWE_Sine_Row_phaseInc_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [ScalerNV2]
// Top Level System

#define AWE_sys_volume_classID 0xBEEF0814
#define AWE_sys_volume_ID 30001

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_sys_volume_profileTime_HANDLE 0x07531007
#define AWE_sys_volume_profileTime_MASK 0x00000080
#define AWE_sys_volume_profileTime_SIZE 0x00000001

// float masterGain - Overall gain to apply.
// Default value: -19.0044
// Range: -24 to 24
#define AWE_sys_volume_masterGain_HANDLE 0x07531008
#define AWE_sys_volume_masterGain_MASK 0x00000100
#define AWE_sys_volume_masterGain_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process (0 = 
//         unsmoothed).
// Default value: 10
// Range: 0 to 1000
#define AWE_sys_volume_smoothingTime_HANDLE 0x07531009
#define AWE_sys_volume_smoothingTime_MASK 0x00000200
#define AWE_sys_volume_smoothingTime_SIZE 0x00000001

// int isDB - Selects between linear (=0) and dB (=1) operation
// Default value: 1
// Range: 0 to 1
#define AWE_sys_volume_isDB_HANDLE 0x0753100A
#define AWE_sys_volume_isDB_MASK 0x00000400
#define AWE_sys_volume_isDB_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_sys_volume_smoothingCoeff_HANDLE 0x0753100B
#define AWE_sys_volume_smoothingCoeff_MASK 0x00000800
#define AWE_sys_volume_smoothingCoeff_SIZE 0x00000001

// float trimGain[16] - Array of trim gains, one per channel
// Default value:
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
//     0
// Range: -24 to 24
#define AWE_sys_volume_trimGain_HANDLE 0x8753100C
#define AWE_sys_volume_trimGain_MASK 0x00001000
#define AWE_sys_volume_trimGain_SIZE 0x00000010

// float targetGain[16] - Computed target gains in linear units
#define AWE_sys_volume_targetGain_HANDLE 0x8753100D
#define AWE_sys_volume_targetGain_MASK 0x00002000
#define AWE_sys_volume_targetGain_SIZE 0x00000010

// float currentGain[16] - Instanteous gains.  These ramp towards 
//         targetGain
#define AWE_sys_volume_currentGain_HANDLE 0x8753100E
#define AWE_sys_volume_currentGain_MASK 0x00004000
#define AWE_sys_volume_currentGain_SIZE 0x00000010


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

#endif // AWE_DTMF_GENERATOR_CONTROLINTERFACE_H

