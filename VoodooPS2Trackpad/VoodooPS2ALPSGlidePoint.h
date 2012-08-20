/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.2 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _APPLEPS2SYNAPTICSTOUCHPAD_H
#define _APPLEPS2SYNAPTICSTOUCHPAD_H

#include "ApplePS2MouseDevice.h"
#include <IOKit/hidsystem/IOHIPointing.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ApplePS2ALPSGlidePoint Class Declaration
//


typedef struct ALPSStatus {
	UInt8 byte0;
	UInt8 byte1;
	UInt8 byte2;
} ALPSStatus_t;

#define SCROLL_NONE  0
#define SCROLL_HORIZ 1
#define SCROLL_VERT  2

class ApplePS2ALPSGlidePoint : public IOHIPointing 
{
	OSDeclareDefaultStructors( ApplePS2ALPSGlidePoint );

private:
    ApplePS2MouseDevice * _device;
    UInt32                _interruptHandlerInstalled:1;
    UInt32                _powerControlHandlerInstalled:1;
    UInt8                 _packetBuffer[6];
    UInt32                _packetByteCount;
    IOFixed               _resolution;
    UInt16                _touchPadVersion;
    UInt8                 _touchPadModeByte;

	bool				  _dragging;
	bool				  _edgehscroll;
	bool				  _edgevscroll;
    UInt32                _edgeaccell;
    double                _edgeaccellvalue;
	bool				  _draglock;
	AbsoluteTime		_time;
//from synaptic
	int z_finger;
	int divisor;
	int ledge;
	int redge;
	int tedge;
	int bedge;
	int vscrolldivisor, hscrolldivisor, cscrolldivisor;
	int ctrigger;
	int centerx;
	int centery;
	int lastx, lasty;
	int xrest, yrest, scrollrest;
	int inited;
	uint64_t maxtaptime;
	uint64_t touchtime;
	uint64_t untouchtime;
	uint64_t maxdragtime;
	int hsticky,vsticky, wsticky, tapstable;
	int wlimit, wvdivisor, whdivisor;
	int xmoved,ymoved,xscrolled, yscrolled;
	bool clicking;
	bool dragging;
	bool draglock;
	bool hscroll, scroll;
	bool wasdouble;
	bool rtap;
	
	
private:
    SInt32				  _xpos, _xscrollpos;
    SInt32				  _ypos, _yscrollpos;
    SInt32				  _zpos, _zscrollpos;
    int                   _xdiffold, _ydiffold;
    short                 _scrolling;
	int					_movedelay;
	bool				_absolute; 
	enum {MODE_NOTOUCH, MODE_MOVE, MODE_VSCROLL, MODE_HSCROLL, MODE_CSCROLL, MODE_MTOUCH, 
		MODE_PREDRAG, MODE_DRAG, MODE_DRAGNOTOUCH, MODE_DRAGLOCK} touchmode;

    
protected:
	virtual void   dispatchRelativePointerEventWithPacket( UInt8 * packet,
                                                           UInt32  packetSize );
	virtual void   dispatchAbsolutePointerEventWithPacket(UInt8 *packet,UInt32 packetSize);
	virtual void   getModel(ALPSStatus_t *e6,ALPSStatus_t *e7);
	virtual void   setAbsoluteMode();
	virtual bool   setECMode(bool enable);
	virtual void	setMisc( UInt16 val );
	
	virtual void	AlpsECNibble(PS2Request * request, int * index, uint8_t nibble);
	virtual int		AlpsECWrite(uint16_t addr, uint8_t value);
	
	virtual void   getStatus(ALPSStatus_t *status);
	virtual int    insideScrollArea(int x,int y);

    virtual void   setCommandByte( UInt8 setBits, UInt8 clearBits );
	virtual void   setSampleRateAndResolution(uint8_t rate, uint8_t res );

	virtual void   setTapEnable( bool enable );
    virtual void   setTouchPadEnable( bool enable );
#if _NO_TOUCHPAD_ENABLE_
    virtual UInt32 getTouchPadData( UInt8 dataSelector );
    virtual bool   setTouchPadModeByte( UInt8 modeByteValue,
                                        bool  enableStreamMode = false );
#endif
	virtual void   free();
	virtual void   interruptOccurred( UInt8 data );
    virtual void   setDevicePowerState(UInt32 whatToDo);

protected:
	virtual IOItemCount buttonCount();
	virtual IOFixed     resolution();

public:
    virtual bool init( OSDictionary * properties );
    virtual ApplePS2ALPSGlidePoint * probe( IOService * provider,
                                               SInt32 *    score );
    
    virtual bool start( IOService * provider );
    virtual void stop( IOService * provider );
    
    virtual UInt32 deviceType();
    virtual UInt32 interfaceID();

	virtual IOReturn setParamProperties( OSDictionary * dict );
};

const int cmds[] = {
	kDP_SetMousePoll,   
	kDP_SetDefaults,
	kDP_SetMouseScaling2To1,
	kDP_SetMouseSampleRate,
	kDP_SetMouseSampleRate,
	kDP_SetMouseSampleRate, //5
	kDP_SetMouseSampleRate, 
	kDP_SetMouseSampleRate,
	kDP_SetMouseSampleRate,
	kDP_SetMouseSampleRate,
	kDP_GetMouseInformation, //10
	kDP_SetMouseResolution,
	kDP_SetMouseResolution,
	kDP_SetMouseResolution,
	kDP_SetMouseResolution,
	kDP_SetMouseScaling1To1 }; //15
const unsigned char params[] = {
	0xff, 0xff, 0xff, 10, 20, 40, 60, 80, 100, 200, 0xff, 0,  1,  2,  3, 0xff };
//  F0,   F6,   E7,   F3, F3, F3, F3, F3, F3,  F3,  E9,   E8, E8, E8, E8, E6
//   0     1    2      3   4   5   6   7   8    9    a     b   c   d   e   f
#endif /* _APPLEPS2SYNAPTICSTOUCHPAD_H */
