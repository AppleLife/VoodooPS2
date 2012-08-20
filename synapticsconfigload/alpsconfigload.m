#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOKitLib.h>

int main (int argc, char * const argv[]) {
	io_service_t io_service;
	FILE *f;
	int siz, nkeys;
	UInt8 *buf; 
	CFDataRef dat;
	CFDictionaryRef plist;
	CFStringRef *keys;
	NSString *tmp1, *tmp2;
	CFTypeRef *vals;
	int i;
	
	io_service = IOServiceGetMatchingService(0, IOServiceMatching("ApplePS2APLSGlidePoint"));
	if (!io_service)
	{
		printf ("No ApplePS2APLSGlidePoint found\n");
		return 1;
	}
	
	f=fopen ([tmp2=[NSHomeDirectory() stringByAppendingString:tmp1
					= [NSString stringWithUTF8String: "/Library/Preferences/org.voodoo.APLSGlidePoint.plist"]] UTF8String], "rb");
	CFRelease(tmp1);
	CFRelease(tmp2);
	
	if (!f)
	{
		printf ("Couldn't open config.plist\n");
		return 1;
	}
	fseek (f, 0, SEEK_END);
	siz=ftell (f);
	buf=(UInt8 *)malloc (siz);
	if (!buf)
	{
		printf ("Couldn't allocate space\n");
		return 1;
	}
	fseek (f, 0, SEEK_SET);
	fread (buf, 1, siz, f);
	if (!(dat=CFDataCreate (kCFAllocatorDefault, buf, siz)))
	{
		printf ("Couldn't allocate space\n");
		return 1;
	}
	
	if (!(plist=(CFDictionaryRef)CFPropertyListCreateFromXMLData (kCFAllocatorDefault, dat, kCFPropertyListImmutable, NULL)))
	{
		printf ("Error parsing plist\n");
		return 1;
	}
	
	nkeys = CFDictionaryGetCount(plist);
	keys = (CFStringRef *) malloc (nkeys*sizeof (CFStringRef));
	vals = (CFTypeRef *) malloc (nkeys*sizeof (CFTypeRef));
	if (!keys || !vals)
	{
		printf ("Couldn't allocate space\n");
		free (buf);
		return 1;
	}
	
	CFDictionaryGetKeysAndValues (plist, (const void **)keys, vals);
	for (i=0;i<nkeys;i++)
		IORegistryEntrySetCFProperty(io_service, keys[i], vals[i]);
	CFRelease (plist);
	CFRelease (dat);
	free (buf);
	free (keys);
	free (vals);
    return 0;
}
