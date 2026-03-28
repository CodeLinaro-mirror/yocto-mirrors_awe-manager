/****************************************************************************
 *
 *		Target Tuning Symbol File
 *		-------------------------
 *
 * 		This file is populated with symbol information only for modules
 *		that have an object ID of 30000 or greater assigned.
 *
 *          Generated on:  11-Nov-2024 22:33:20
 *
 ***************************************************************************/

#ifndef AWE_SET_GET_CONTROLINTERFACE_H
#define AWE_SET_GET_CONTROLINTERFACE_H

// ----------------------------------------------------------------------
//  [Source]
// Top Level System

#define AWE_SourceFloat_10_classID 0xBEEF086F
#define AWE_SourceFloat_10_ID 30000

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SourceFloat_10_profileTime_HANDLE 0x07530007
#define AWE_SourceFloat_10_profileTime_MASK 0x00000080
#define AWE_SourceFloat_10_profileTime_SIZE 0x00000001

// float value[10] - Array of interleaved audio data.
// Default value:
//     1           2         3.5           0           0           0           0         3.5           2           1
// Range: unrestricted
#define AWE_SourceFloat_10_value_HANDLE 0x87530008
#define AWE_SourceFloat_10_value_MASK 0x00000100
#define AWE_SourceFloat_10_value_SIZE 0x0000000A


// ----------------------------------------------------------------------
//  [Sink]
// Top Level System

#define AWE_SinkFloat_10_classID 0xBEEF086D
#define AWE_SinkFloat_10_ID 30001

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SinkFloat_10_profileTime_HANDLE 0x07531007
#define AWE_SinkFloat_10_profileTime_MASK 0x00000080
#define AWE_SinkFloat_10_profileTime_SIZE 0x00000001

// int enable - To enable or disable the plotting.
// Default value: 0
// Range: unrestricted
#define AWE_SinkFloat_10_enable_HANDLE 0x07531008
#define AWE_SinkFloat_10_enable_MASK 0x00000100
#define AWE_SinkFloat_10_enable_SIZE 0x00000001

// float value[10] - Captured values.
#define AWE_SinkFloat_10_value_HANDLE 0x87531009
#define AWE_SinkFloat_10_value_MASK 0x00000200
#define AWE_SinkFloat_10_value_SIZE 0x0000000A

// float yRange[2] - Y-axis range.
// Default value:
//     -5  5
// Range: unrestricted
#define AWE_SinkFloat_10_yRange_HANDLE 0x8753100A
#define AWE_SinkFloat_10_yRange_MASK 0x00000400
#define AWE_SinkFloat_10_yRange_SIZE 0x00000002


// ----------------------------------------------------------------------
//  [Source]
// Top Level System

#define AWE_SourceFloat_1_classID 0xBEEF086F
#define AWE_SourceFloat_1_ID 30002

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SourceFloat_1_profileTime_HANDLE 0x07532007
#define AWE_SourceFloat_1_profileTime_MASK 0x00000080
#define AWE_SourceFloat_1_profileTime_SIZE 0x00000001

// float value[1] - Array of interleaved audio data.
// Default value:
//     5
// Range: unrestricted
#define AWE_SourceFloat_1_value_HANDLE 0x87532008
#define AWE_SourceFloat_1_value_MASK 0x00000100
#define AWE_SourceFloat_1_value_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [Sink]
// Top Level System

#define AWE_SinkFloat_1_classID 0xBEEF086D
#define AWE_SinkFloat_1_ID 30003

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SinkFloat_1_profileTime_HANDLE 0x07533007
#define AWE_SinkFloat_1_profileTime_MASK 0x00000080
#define AWE_SinkFloat_1_profileTime_SIZE 0x00000001

// int enable - To enable or disable the plotting.
// Default value: 0
// Range: unrestricted
#define AWE_SinkFloat_1_enable_HANDLE 0x07533008
#define AWE_SinkFloat_1_enable_MASK 0x00000100
#define AWE_SinkFloat_1_enable_SIZE 0x00000001

// float value[1] - Captured values.
#define AWE_SinkFloat_1_value_HANDLE 0x87533009
#define AWE_SinkFloat_1_value_MASK 0x00000200
#define AWE_SinkFloat_1_value_SIZE 0x00000001

// float yRange[2] - Y-axis range.
// Default value:
//     -5  5
// Range: unrestricted
#define AWE_SinkFloat_1_yRange_HANDLE 0x8753300A
#define AWE_SinkFloat_1_yRange_MASK 0x00000400
#define AWE_SinkFloat_1_yRange_SIZE 0x00000002


// ----------------------------------------------------------------------
//  [SourceInt]
// Top Level System

#define AWE_SourceInt_1_classID 0xBEEF0870
#define AWE_SourceInt_1_ID 30006

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SourceInt_1_profileTime_HANDLE 0x07536007
#define AWE_SourceInt_1_profileTime_MASK 0x00000080
#define AWE_SourceInt_1_profileTime_SIZE 0x00000001

// int value[1] - Array of interleaved audio data
// Default value:
//     57005
// Range: unrestricted
#define AWE_SourceInt_1_value_HANDLE 0x87536008
#define AWE_SourceInt_1_value_MASK 0x00000100
#define AWE_SourceInt_1_value_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [SinkInt]
// Top Level System

#define AWE_SinkInt_1_classID 0xBEEF086E
#define AWE_SinkInt_1_ID 30004

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SinkInt_1_profileTime_HANDLE 0x07534007
#define AWE_SinkInt_1_profileTime_MASK 0x00000080
#define AWE_SinkInt_1_profileTime_SIZE 0x00000001

// int value[1] - Captured values
#define AWE_SinkInt_1_value_HANDLE 0x87534008
#define AWE_SinkInt_1_value_MASK 0x00000100
#define AWE_SinkInt_1_value_SIZE 0x00000001


// ----------------------------------------------------------------------
//  [SourceInt]
// Top Level System

#define AWE_SourceInt_10_classID 0xBEEF0870
#define AWE_SourceInt_10_ID 30007

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SourceInt_10_profileTime_HANDLE 0x07537007
#define AWE_SourceInt_10_profileTime_MASK 0x00000080
#define AWE_SourceInt_10_profileTime_SIZE 0x00000001

// int value[10] - Array of interleaved audio data
// Default value:
//     10    170  57005      0      0      0      0     10    170  57005
// Range: unrestricted
#define AWE_SourceInt_10_value_HANDLE 0x87537008
#define AWE_SourceInt_10_value_MASK 0x00000100
#define AWE_SourceInt_10_value_SIZE 0x0000000A


// ----------------------------------------------------------------------
//  [SinkInt]
// Top Level System

#define AWE_SinkInt_10_classID 0xBEEF086E
#define AWE_SinkInt_10_ID 30005

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SinkInt_10_profileTime_HANDLE 0x07535007
#define AWE_SinkInt_10_profileTime_MASK 0x00000080
#define AWE_SinkInt_10_profileTime_SIZE 0x00000001

// int value[10] - Captured values
#define AWE_SinkInt_10_value_HANDLE 0x87535008
#define AWE_SinkInt_10_value_MASK 0x00000100
#define AWE_SinkInt_10_value_SIZE 0x0000000A


// ----------------------------------------------------------------------
//  [SourceInt]
// Top Level System

#define AWE_SourceInt_2000_classID 0xBEEF0870
#define AWE_SourceInt_2000_ID 30008

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SourceInt_2000_profileTime_HANDLE 0x07538007
#define AWE_SourceInt_2000_profileTime_MASK 0x00000080
#define AWE_SourceInt_2000_profileTime_SIZE 0x00000001

// int value[2000] - Array of interleaved audio data
// Default value:
//     57005
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//       194
//       195
//       196
//       197
//       198
//       199
//       200
//     57005
//       202
//       203
//       204
//       205
//       206
//       207
//       208
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//       234
//       235
//       236
//       237
//       238
//       239
//       240
//     57005
//       242
//       243
//       244
//       245
//       246
//       247
//       248
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//       258
//       259
//       260
//       261
//       262
//       263
//       264
//       265
//       266
//       267
//       268
//       269
//       270
//       271
//       272
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//     57005
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//         0
//      1998
//      1999
//      2000
// Range: unrestricted
#define AWE_SourceInt_2000_value_HANDLE 0x87538008
#define AWE_SourceInt_2000_value_MASK 0x00000100
#define AWE_SourceInt_2000_value_SIZE 0x000007D0


// ----------------------------------------------------------------------
//  [SinkInt]
// Top Level System

#define AWE_SinkInt_2000_classID 0xBEEF086E
#define AWE_SinkInt_2000_ID 30009

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SinkInt_2000_profileTime_HANDLE 0x07539007
#define AWE_SinkInt_2000_profileTime_MASK 0x00000080
#define AWE_SinkInt_2000_profileTime_SIZE 0x00000001

// int value[2000] - Captured values
#define AWE_SinkInt_2000_value_HANDLE 0x87539008
#define AWE_SinkInt_2000_value_MASK 0x00000100
#define AWE_SinkInt_2000_value_SIZE 0x000007D0


// ----------------------------------------------------------------------
//  [MixerV3Fract32]
// Top Level System

#define AWE_MixerFract_classID 0xBEEF08A6
#define AWE_MixerFract_ID 30010

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_MixerFract_profileTime_HANDLE 0x0753A007
#define AWE_MixerFract_profileTime_MASK 0x00000080
#define AWE_MixerFract_profileTime_SIZE 0x00000001

// int maxNonZero - Maximum number of non zero coefficients.
#define AWE_MixerFract_maxNonZero_HANDLE 0x0753A008
#define AWE_MixerFract_maxNonZero_MASK 0x00000100
#define AWE_MixerFract_maxNonZero_SIZE 0x00000001

// int postShift - Number of bits to shift
#define AWE_MixerFract_postShift_HANDLE 0x0753A009
#define AWE_MixerFract_postShift_MASK 0x00000200
#define AWE_MixerFract_postShift_SIZE 0x00000001

// float gainScale - Scale coefficients based on postShift.
#define AWE_MixerFract_gainScale_HANDLE 0x0753A00A
#define AWE_MixerFract_gainScale_MASK 0x00000400
#define AWE_MixerFract_gainScale_SIZE 0x00000001

// int numIn - Number of input channels.
#define AWE_MixerFract_numIn_HANDLE 0x0753A00B
#define AWE_MixerFract_numIn_MASK 0x00000800
#define AWE_MixerFract_numIn_SIZE 0x00000001

// float gain[512] - Linear gain.
// Default value:
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
//     0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
// Range: -60 to 20
#define AWE_MixerFract_gain_HANDLE 0x8753A00C
#define AWE_MixerFract_gain_MASK 0x00001000
#define AWE_MixerFract_gain_SIZE 0x00000200

// fract32 nonZeroGainFract32[512] - Instanteous fract32 gain being 
//         applied.
#define AWE_MixerFract_nonZeroGainFract32_HANDLE 0x8753A00D
#define AWE_MixerFract_nonZeroGainFract32_MASK 0x00002000
#define AWE_MixerFract_nonZeroGainFract32_SIZE 0x00000200


// ----------------------------------------------------------------------
//  [SinkFract32]
// Top Level System

#define AWE_SinkFract_MixOut_classID 0xBEEF08C5
#define AWE_SinkFract_MixOut_ID 30011

// int profileTime - 24.8 fixed point filtered execution time. Must be pumped 1000 times to get to within .1% accuracy
#define AWE_SinkFract_MixOut_profileTime_HANDLE 0x0753B007
#define AWE_SinkFract_MixOut_profileTime_MASK 0x00000080
#define AWE_SinkFract_MixOut_profileTime_SIZE 0x00000001

// int enable - To Enable or disable the plotting
// Default value: 0
// Range: unrestricted
#define AWE_SinkFract_MixOut_enable_HANDLE 0x0753B008
#define AWE_SinkFract_MixOut_enable_MASK 0x00000100
#define AWE_SinkFract_MixOut_enable_SIZE 0x00000001

// fract32 value[32] - Captured values
#define AWE_SinkFract_MixOut_value_HANDLE 0x8753B009
#define AWE_SinkFract_MixOut_value_MASK 0x00000200
#define AWE_SinkFract_MixOut_value_SIZE 0x00000020

// float yRange[2] - Yaxis Range
// Default value:
//     -5  5
// Range: unrestricted
#define AWE_SinkFract_MixOut_yRange_HANDLE 0x8753B00A
#define AWE_SinkFract_MixOut_yRange_MASK 0x00000400
#define AWE_SinkFract_MixOut_yRange_SIZE 0x00000002


#define AWE_OBJECT_FOUND 0

#endif // AWE_SET_GET_CONTROLINTERFACE_H

