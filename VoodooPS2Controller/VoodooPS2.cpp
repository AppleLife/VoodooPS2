#ifndef TIGER
#if 1 //def SNOW_LEO
/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*! @file       VoodooACPIPS2Nub.cpp
 @abstract   VoodooACPIPS2Nub class implementation
 @discussion
 Implements the ACPI PS/2 nub for ApplePS2Controller.kext.
 Reverse-engineered from the Darwin 8 binary ACPI kext.
 Copyright 2007 David Elliott
 */

#include "VoodooPS2.h"

#if 0
#define DEBUG_LOG(args...)  IOLog(args)
#else
#define DEBUG_LOG(args...)
#endif

static IOPMPowerState myTwoStates[2] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

#define super IOPlatformDevice

OSDefineMetaClassAndStructors(VoodooACPIPS2Nub, IOPlatformDevice);

// FIXME: We could simply ask for the PE rather than importing the global
// from AppleACPIPlatformExpert.kext
// extern IOPlatformExpert *gAppleACPIPlatformExpert;

bool VoodooACPIPS2Nub::start(IOService *provider)
{
    if (!super::start(provider)) return false;
	
    DEBUG_LOG("VoodooACPIPS2Nub::start: provider=%p\n", provider);
	
    /* Initialize our interrupt controller/specifier i-vars */
    m_interruptControllers = OSArray::withCapacity(2);
    m_interruptSpecifiers = OSArray::withCapacity(2);
    if(m_interruptControllers == NULL || m_interruptSpecifiers == NULL)
        return false;
	
    /* Merge in the keyboard (primary) provider interrupt properties */
    mergeInterruptProperties(provider, LEGACY_KEYBOARD_IRQ);
	
    /* Initialize and register our power management properties */
	PMinit();
    registerPowerDriver(this, myTwoStates, 2);
    provider->joinPMtree(this);
	
    /* Find the mouse provider */
    m_mouseProvider = findMouseDevice();
    if(m_mouseProvider != NULL)
    {
        DEBUG_LOG("VoodooACPIPS2Nub::start: Found mouse PNP device\n");
        if(attach(m_mouseProvider))
        {
            mergeInterruptProperties(m_mouseProvider, LEGACY_MOUSE_IRQ);
            if(m_mouseProvider->inPlane(gIOPowerPlane))
            {
                m_mouseProvider->joinPMtree(this);
            }
        }
    }
	
    /* Set our interrupt properties in the IOReigstry */
    if(m_interruptControllers->getCount() != 0 && m_interruptSpecifiers->getCount() != 0)
    {
        setProperty(gIOInterruptControllersKey, m_interruptControllers);
        setProperty(gIOInterruptSpecifiersKey, m_interruptSpecifiers);
    }
	
    /* Release the arrays we allocated.  Our properties dictionary has them retained */
    m_interruptControllers->release();
    m_interruptControllers = NULL;
    m_interruptSpecifiers->release();
    m_interruptSpecifiers = NULL;
	
    /* Make ourselves the ps2controller nub and register so ApplePS2Controller can find us. */
    setName("ps2controller");
    registerService();
	
    DEBUG_LOG("VoodooACPIPS2Nub::start: startup complete\n");
	
    return true;
}

IOService *VoodooACPIPS2Nub::findMouseDevice()
{
    OSObject *prop = getProperty("MouseNameMatch");
    /* Search from the root of the ACPI plane for the mouse PNP nub */
    IORegistryIterator *i = IORegistryIterator::iterateOver(gIOACPIPlane, kIORegistryIterateRecursively);
    IORegistryEntry *entry;
    if(i != NULL)
    {
        while(entry = i->getNextObject())
        {
            if(entry->compareNames(prop))
                break;
        }
        i->release();
    }
    else
        entry = NULL;
    return OSDynamicCast(IOService, entry);
}

void VoodooACPIPS2Nub::mergeInterruptProperties(IOService *pnpProvider, long)
{
    /* Make sure we're called from within start() where these i-vars are valid */
    if(m_interruptControllers == NULL || m_interruptSpecifiers == NULL)
        return;
	
    /*  Get the interrupt controllers/specifiers arrays from the provider, and make sure they
     *  exist and contain at least one entry.  We assume they contain exactly one entry.
     */
    OSArray *controllers = OSDynamicCast(OSArray,pnpProvider->getProperty(gIOInterruptControllersKey));
    OSArray *specifiers = OSDynamicCast(OSArray,pnpProvider->getProperty(gIOInterruptSpecifiersKey));
    if(controllers == NULL || specifiers == NULL)
        return;
    if(controllers->getCount() == 0 || specifiers->getCount() == 0)
        return;
	
    /* Append the first object of each array into our own respective array */
    m_interruptControllers->setObject(controllers->getObject(0));
    m_interruptSpecifiers->setObject(specifiers->getObject(0));
}

IOReturn VoodooACPIPS2Nub::registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refCon)
{
    if(source == LEGACY_KEYBOARD_IRQ)
        return super::registerInterrupt(0, target, handler, refCon);
    else if(source == LEGACY_MOUSE_IRQ)
        return super::registerInterrupt(1, target, handler, refCon);
    else
        return kIOReturnBadArgument;
}

IOReturn VoodooACPIPS2Nub::unregisterInterrupt(int source)
{
    if(source == LEGACY_KEYBOARD_IRQ)
        return super::unregisterInterrupt(0);
    else if(source == LEGACY_MOUSE_IRQ)
        return super::unregisterInterrupt(1);
    else
        return kIOReturnBadArgument;
}

IOReturn VoodooACPIPS2Nub::getInterruptType(int source, int *interruptType)
{
    if(source == LEGACY_KEYBOARD_IRQ)
        return super::getInterruptType(0, interruptType);
    else if(source == LEGACY_MOUSE_IRQ)
        return super::getInterruptType(1, interruptType);
    else
        return kIOReturnBadArgument;
}

IOReturn VoodooACPIPS2Nub::enableInterrupt(int source)
{
    if(source == LEGACY_KEYBOARD_IRQ)
        return super::enableInterrupt(0);
    else if(source == LEGACY_MOUSE_IRQ)
        return super::enableInterrupt(1);
    else
        return kIOReturnBadArgument;
}

IOReturn VoodooACPIPS2Nub::disableInterrupt(int source)
{
    if(source == LEGACY_KEYBOARD_IRQ)
        return super::disableInterrupt(0);
    else if(source == LEGACY_MOUSE_IRQ)
        return super::disableInterrupt(1);
    else
        return kIOReturnBadArgument;
}

bool VoodooACPIPS2Nub::compareName( OSString * name, OSString ** matched ) const
{
	//    return gAppleACPIPlatformExpert->compareNubName( this, name, matched );
    return( this->IORegistryEntry::compareName( name, matched ));
}

IOReturn VoodooACPIPS2Nub::getResources( void )
{
	//    return gAppleACPIPlatformExpert->getNubResources(this);
    return( kIOReturnSuccess );
}
#else
#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IODeviceTreeSupport.h>
#include "VoodooPS2.h"
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#define kextname "PS2Controller"
#define dbg(args...)    do { kprintf(kextname ": DEBUG " args); IOLog(kextname ": DEBUG " args); IOSleep(50); } while(0)
#define err(args...)    do { IOLog(kextname ": ERROR " args); IOSleep(50); } while(0)


#define info(args...)   do { kprintf(kextname ": " args); IOLog(kextname ": " args); } while(0)
#define chkpoint(arg)   kprintf(kextname " %s : %s\n", __FUNCTION__, arg)
#define fail(arg)  do { kprintf(kextname ": %s(%d) - fail '%s'\n", __FUNCTION__, __LINE__, arg); printf(kextname ": %s(%d) - fail '%s'\n", __FUNCTION__, __LINE__, arg); goto fail; } while(0)
#define RELEASE(x) do { if(x) { (x)->release(); (x) = 0; } } while(0)

// Use to return our platform
#define platform IOService::getPlatform()

// Define my superclass
#define super IOService

OSDefineMetaClassAndStructors( driver, IOService )	;


bool driver::create_ps2_device(char *devName, OSDictionary *devProp) {
 IOPlatformDevice *dev = new IOPlatformDevice;
 IOReturn ret;
 IOInterruptController *ic;
 //uint32_t interrupts[2] = { 0x01, 0x0c };	// PS2 interrupts
 
 /* Initialize our interrupt controller/specifier i-vars */
 OSArray *controllers = OSArray::withCapacity(2);
 if (!controllers) fail("!controllers");
 
 controllers->setObject(0,OSSymbol::withCString("io-apic-0"));
 controllers->setObject(1,OSSymbol::withCString("io-apic-0"));

 if (dev == NULL) fail("dev==NULL");
 if (dev->init(devProp) == false) fail("dev->init");

 dev->setName(devName);
 dev->setProperty("IOInterruptControllers",controllers);
 
 // Attach it to the platform 
 dev->attach(platform);

 ret=dev->lookupInterrupt(0,true,&ic);
 if (ret!=kIOReturnSuccess) fail("lookupInterrupt 0 failed");
 ret=dev->lookupInterrupt(1,true,&ic);
 if (ret!=kIOReturnSuccess) err("lookupInterrupt 1 failed");

 dev->registerService();
 
 info("Created dev /%s\n",devName);
 
 RELEASE(dev);
 RELEASE(controllers);
 return(true);

 fail:
 RELEASE(dev);
 return(false);
}


bool driver::init(OSDictionary *dict) {
 return(super::init(dict));
}

void driver::free(void) {
 chkpoint("entered");
 super::free();
}

IOService *driver::probe(IOService *provider, SInt32 *score) {
 chkpoint("entered");
 return(super::probe(provider, score));
}

bool driver::start(IOService *provider) {
 OSString *devName;
 OSDictionary *devProp;

 chkpoint("entered");
 if (!super::start(provider)) fail("super::start");

 devName=(OSString*)OSString::withCStringNoCopy("ps2controller");
 devProp=(OSDictionary*)getProperty("PS2_properties");
 if (!devProp) fail("No PS2_properties defined");

 create_ps2_device((char*)devName->getCStringNoCopy(),devProp);

 return(true);

 fail:
 return(false);
}
 
void driver::stop(IOService *provider) {
 super::stop(provider);
}
#endif
#endif
