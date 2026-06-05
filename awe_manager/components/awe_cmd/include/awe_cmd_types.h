/* MIT License
**
** Copyright (c) 2026 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/


#ifndef INCLUSION_GUARD_AWECMD_TYPES_H
#define INCLUSION_GUARD_AWECMD_TYPES_H

#ifndef UINT32
    typedef unsigned int UINT32;
#endif
#ifndef INT32
    typedef int INT32;
#endif
#ifndef INT64
    typedef long long INT64;
#endif

// todo: redefinition from AWECore Errors.h! remove
#define E_SUCCESS (0)

/**
 * AWE event structure; contains meta information about an event, like type, category, size of payload data, etc.
 * The actual data of the event is contained in a separate payload buffer, which is passed to the event listener
 * function together with this header.
 */
typedef struct aweevent_header {
    UINT32 instanceId;        // <** index of the AWE instance (aka CPU core index) (0-15) that has generated this event
    UINT32 objectId;          // <** event module objectId;
    UINT32 classId;           // <** event module classid, 0 or don't care for system events (eventCategory != 0),
    UINT32 eventType;         // <** event type as defined in event module
    UINT32 eventCategory;     // <** 0 = user/event 1 = system
    UINT32 dataSize;          // <** size of payload data in Bytes
    INT64 timeStamp;          // <** systick
} aweevent_header;

#define aweevent_header_SIZE_IN_WORDS sizeof(aweevent_header) / sizeof(UINT32)

#endif // INCLUSION_GUARD_AWECMD_TYPES_H
