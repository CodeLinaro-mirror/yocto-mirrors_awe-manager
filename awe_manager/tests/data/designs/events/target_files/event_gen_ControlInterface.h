/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 * 		This file is populated with symbol information only for modules
 *		that have an object ID of 30000 or greater assigned.
 *
 *          Generated on:  04-Nov-2024 23:33:28
 *
 ***************************************************************************/

#ifndef AWE_EVENT_GEN_CONTROLINTERFACE_H
#define AWE_EVENT_GEN_CONTROLINTERFACE_H

// ----------------------------------------------------------------------
//  [DCSourceInt]
// Newly created subsystem

#define AWE_EventTrigger_classID 0xBEEF0873
#define AWE_EventTrigger_ID 30000

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_EventTrigger_profileTime_HANDLE 0x07530007
#define AWE_EventTrigger_profileTime_MASK 0x00000080
#define AWE_EventTrigger_profileTime_SIZE 0x00000001

// int value - Data Value
// Default value: 0
// Range: 0 to 1
#define AWE_EventTrigger_value_HANDLE 0x07530008
#define AWE_EventTrigger_value_MASK 0x00000100
#define AWE_EventTrigger_value_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [SourceV2Int]
// Newly created subsystem

#define AWE_SourceV2ASCII_classID 0xBEEF08F7
#define AWE_SourceV2ASCII_ID 30002

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SourceV2ASCII_profileTime_HANDLE 0x07532007
#define AWE_SourceV2ASCII_profileTime_MASK 0x00000080
#define AWE_SourceV2ASCII_profileTime_SIZE 0x00000001

// int BlockSize - Block Size
#define AWE_SourceV2ASCII_BlockSize_HANDLE 0x07532008
#define AWE_SourceV2ASCII_BlockSize_MASK 0x00000100
#define AWE_SourceV2ASCII_BlockSize_SIZE 0x00000001

// int NumChannels - Number of Channels
#define AWE_SourceV2ASCII_NumChannels_HANDLE 0x07532009
#define AWE_SourceV2ASCII_NumChannels_MASK 0x00000200
#define AWE_SourceV2ASCII_NumChannels_SIZE 0x00000001

// int SampleRate - Sample Rate
#define AWE_SourceV2ASCII_SampleRate_HANDLE 0x0753200A
#define AWE_SourceV2ASCII_SampleRate_MASK 0x00000400
#define AWE_SourceV2ASCII_SampleRate_SIZE 0x00000001

// int isComplex - Is Complex
#define AWE_SourceV2ASCII_isComplex_HANDLE 0x0753200B
#define AWE_SourceV2ASCII_isComplex_MASK 0x00000800
#define AWE_SourceV2ASCII_isComplex_SIZE 0x00000001

// int format - Is interleaved Or deinterleaved
#define AWE_SourceV2ASCII_format_HANDLE 0x0753200C
#define AWE_SourceV2ASCII_format_MASK 0x00001000
#define AWE_SourceV2ASCII_format_SIZE 0x00000001

// int arrayHeap - Heap in which to allocate memory.
#define AWE_SourceV2ASCII_arrayHeap_HANDLE 0x0753200D
#define AWE_SourceV2ASCII_arrayHeap_MASK 0x00002000
#define AWE_SourceV2ASCII_arrayHeap_SIZE 0x00000001

// int value[5] - Array of integer data
// Default value:
//      1953719636
//      1936942413
//         6645601
//               0
//     -2147483648
// Range: unrestricted
#define AWE_SourceV2ASCII_value_HANDLE 0x8753200E
#define AWE_SourceV2ASCII_value_MASK 0x00004000
#define AWE_SourceV2ASCII_value_SIZE 0x00000005


// ----------------------------------------------------------------------
//  [Event]
// Newly created subsystem

#define AWE_Event1_classID 0xBEEF0DA6
#define AWE_Event1_ID 30003

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Event1_profileTime_HANDLE 0x07533007
#define AWE_Event1_profileTime_MASK 0x00000080
#define AWE_Event1_profileTime_SIZE 0x00000001

// int eventType - Event type, defined by this module instance
#define AWE_Event1_eventType_HANDLE 0x07533008
#define AWE_Event1_eventType_MASK 0x00000100
#define AWE_Event1_eventType_SIZE 0x00000001

// int triggerType - Trigger Type
// Default value: 0
// Range: 0 to 4
#define AWE_Event1_triggerType_HANDLE 0x07533009
#define AWE_Event1_triggerType_MASK 0x00000200
#define AWE_Event1_triggerType_SIZE 0x00000001

// int resetTriggerCnts - Used to reset the triggerCnt and 
//         failedTriggerCnt.
#define AWE_Event1_resetTriggerCnts_HANDLE 0x0753300A
#define AWE_Event1_resetTriggerCnts_MASK 0x00000400
#define AWE_Event1_resetTriggerCnts_SIZE 0x00000001

// int triggerBehavior - Controls whether the event is triggered 
//         immediately, or in deferred processing
// Default value: 0
// Range: 0 to 1
#define AWE_Event1_triggerBehavior_HANDLE 0x0753300B
#define AWE_Event1_triggerBehavior_MASK 0x00000800
#define AWE_Event1_triggerBehavior_SIZE 0x00000001

// int prevTriggerValue - Internal variable to store old trigger value
// Default value: 0
// Range: unrestricted
#define AWE_Event1_prevTriggerValue_HANDLE 0x0753300C
#define AWE_Event1_prevTriggerValue_MASK 0x00001000
#define AWE_Event1_prevTriggerValue_SIZE 0x00000001

// int isCallbackRegistered - Tracks whether there is an event callback 
//         registered on the target system
#define AWE_Event1_isCallbackRegistered_HANDLE 0x0753300D
#define AWE_Event1_isCallbackRegistered_MASK 0x00002000
#define AWE_Event1_isCallbackRegistered_SIZE 0x00000001

// uint triggerCnt - Counts the number of times the module has 
//         successfully triggered the event callback
#define AWE_Event1_triggerCnt_HANDLE 0x0753300E
#define AWE_Event1_triggerCnt_MASK 0x00004000
#define AWE_Event1_triggerCnt_SIZE 0x00000001

// uint failedTriggerCnt - Counts the number of times the event callback 
//         has failed to process the trigger
#define AWE_Event1_failedTriggerCnt_HANDLE 0x0753300F
#define AWE_Event1_failedTriggerCnt_MASK 0x00008000
#define AWE_Event1_failedTriggerCnt_SIZE 0x00000001

// int deferredProcessingActive - State of deferred processing
#define AWE_Event1_deferredProcessingActive_HANDLE 0x07533010
#define AWE_Event1_deferredProcessingActive_MASK 0x00010000
#define AWE_Event1_deferredProcessingActive_SIZE 0x00000001

// int deferredTriggerRequired - Internal flag used to handle deferred 
//         processing
#define AWE_Event1_deferredTriggerRequired_HANDLE 0x07533011
#define AWE_Event1_deferredTriggerRequired_MASK 0x00020000
#define AWE_Event1_deferredTriggerRequired_SIZE 0x00000001

// int moduleObjId - Custom objectID of this module. -1 if not in custom 
//         objectID range
#define AWE_Event1_moduleObjId_HANDLE 0x07533012
#define AWE_Event1_moduleObjId_MASK 0x00040000
#define AWE_Event1_moduleObjId_SIZE 0x00000001

// int deferredBuffer[0] - Captured payload for deferred processing
#define AWE_Event1_deferredBuffer_HANDLE 0x87533013
#define AWE_Event1_deferredBuffer_MASK 0x00080000
#define AWE_Event1_deferredBuffer_SIZE 0x00000000


// ----------------------------------------------------------------------
//  [DCSourceV2]
// Newly created subsystem

#define AWE_RMSThreshold_classID 0xBEEF0872
#define AWE_RMSThreshold_ID 30004

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_RMSThreshold_profileTime_HANDLE 0x07534007
#define AWE_RMSThreshold_profileTime_MASK 0x00000080
#define AWE_RMSThreshold_profileTime_SIZE 0x00000001

// float value - Data Value
// Default value: -10
// Range: -60 to 20
#define AWE_RMSThreshold_value_HANDLE 0x07534008
#define AWE_RMSThreshold_value_MASK 0x00000100
#define AWE_RMSThreshold_value_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [ScalerV2]
// Newly created subsystem

#define AWE_Scaler1_classID 0xBEEF0813
#define AWE_Scaler1_ID 30001

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Scaler1_profileTime_HANDLE 0x07531007
#define AWE_Scaler1_profileTime_MASK 0x00000080
#define AWE_Scaler1_profileTime_SIZE 0x00000001

// float gain - Gain in either linear or dB units.
// Default value: -24
// Range: -24 to 24
#define AWE_Scaler1_gain_HANDLE 0x07531008
#define AWE_Scaler1_gain_MASK 0x00000100
#define AWE_Scaler1_gain_SIZE 0x00000001

// float smoothingTime - Time constant of the smoothing process (0 = 
//         unsmoothed).
// Default value: 10
// Range: 0 to 1000
#define AWE_Scaler1_smoothingTime_HANDLE 0x07531009
#define AWE_Scaler1_smoothingTime_MASK 0x00000200
#define AWE_Scaler1_smoothingTime_SIZE 0x00000001

// int isDB - Selects between linear (=0) and dB (=1) operation
// Default value: 1
// Range: 0 to 1
#define AWE_Scaler1_isDB_HANDLE 0x0753100A
#define AWE_Scaler1_isDB_MASK 0x00000400
#define AWE_Scaler1_isDB_SIZE 0x00000001

// float targetGain - Target gain in linear units.
#define AWE_Scaler1_targetGain_HANDLE 0x0753100B
#define AWE_Scaler1_targetGain_MASK 0x00000800
#define AWE_Scaler1_targetGain_SIZE 0x00000001

// float currentGain - Instantaneous gain applied by the module.
#define AWE_Scaler1_currentGain_HANDLE 0x0753100C
#define AWE_Scaler1_currentGain_MASK 0x00001000
#define AWE_Scaler1_currentGain_SIZE 0x00000001

// float smoothingCoeff - Smoothing coefficient.
#define AWE_Scaler1_smoothingCoeff_HANDLE 0x0753100D
#define AWE_Scaler1_smoothingCoeff_MASK 0x00002000
#define AWE_Scaler1_smoothingCoeff_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [Event]
// Newly created subsystem

#define AWE_Event2_classID 0xBEEF0DA6
#define AWE_Event2_ID 31111

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_Event2_profileTime_HANDLE 0x07987007
#define AWE_Event2_profileTime_MASK 0x00000080
#define AWE_Event2_profileTime_SIZE 0x00000001

// int eventType - Event type, defined by this module instance
#define AWE_Event2_eventType_HANDLE 0x07987008
#define AWE_Event2_eventType_MASK 0x00000100
#define AWE_Event2_eventType_SIZE 0x00000001

// int triggerType - Trigger Type
// Default value: 2
// Range: 0 to 3
#define AWE_Event2_triggerType_HANDLE 0x07987009
#define AWE_Event2_triggerType_MASK 0x00000200
#define AWE_Event2_triggerType_SIZE 0x00000001

// int resetTriggerCnts - Used to reset the triggerCnt and 
//         failedTriggerCnt.
#define AWE_Event2_resetTriggerCnts_HANDLE 0x0798700A
#define AWE_Event2_resetTriggerCnts_MASK 0x00000400
#define AWE_Event2_resetTriggerCnts_SIZE 0x00000001

// int triggerBehavior - Controls whether the event is triggered 
//         immediately, or in deferred processing
// Default value: 0
// Range: 0 to 1
#define AWE_Event2_triggerBehavior_HANDLE 0x0798700B
#define AWE_Event2_triggerBehavior_MASK 0x00000800
#define AWE_Event2_triggerBehavior_SIZE 0x00000001

// int prevTriggerValue - Internal variable to store old trigger value
// Default value: 0
// Range: unrestricted
#define AWE_Event2_prevTriggerValue_HANDLE 0x0798700C
#define AWE_Event2_prevTriggerValue_MASK 0x00001000
#define AWE_Event2_prevTriggerValue_SIZE 0x00000001

// int isCallbackRegistered - Tracks whether there is an event callback 
//         registered on the target system
#define AWE_Event2_isCallbackRegistered_HANDLE 0x0798700D
#define AWE_Event2_isCallbackRegistered_MASK 0x00002000
#define AWE_Event2_isCallbackRegistered_SIZE 0x00000001

// uint triggerCnt - Counts the number of times the module has 
//         successfully triggered the event callback
#define AWE_Event2_triggerCnt_HANDLE 0x0798700E
#define AWE_Event2_triggerCnt_MASK 0x00004000
#define AWE_Event2_triggerCnt_SIZE 0x00000001

// uint failedTriggerCnt - Counts the number of times the event callback 
//         has failed to process the trigger
#define AWE_Event2_failedTriggerCnt_HANDLE 0x0798700F
#define AWE_Event2_failedTriggerCnt_MASK 0x00008000
#define AWE_Event2_failedTriggerCnt_SIZE 0x00000001

// int deferredProcessingActive - State of deferred processing
#define AWE_Event2_deferredProcessingActive_HANDLE 0x07987010
#define AWE_Event2_deferredProcessingActive_MASK 0x00010000
#define AWE_Event2_deferredProcessingActive_SIZE 0x00000001

// int deferredTriggerRequired - Internal flag used to handle deferred 
//         processing
#define AWE_Event2_deferredTriggerRequired_HANDLE 0x07987011
#define AWE_Event2_deferredTriggerRequired_MASK 0x00020000
#define AWE_Event2_deferredTriggerRequired_SIZE 0x00000001

// int moduleObjId - Custom objectID of this module. -1 if not in custom 
//         objectID range
#define AWE_Event2_moduleObjId_HANDLE 0x07987012
#define AWE_Event2_moduleObjId_MASK 0x00040000
#define AWE_Event2_moduleObjId_SIZE 0x00000001

// int deferredBuffer[0] - Captured payload for deferred processing
#define AWE_Event2_deferredBuffer_HANDLE 0x87987013
#define AWE_Event2_deferredBuffer_MASK 0x00080000
#define AWE_Event2_deferredBuffer_SIZE 0x00000000


#define AWE_OBJECT_FOUND 0

#endif // AWE_EVENT_GEN_CONTROLINTERFACE_H

