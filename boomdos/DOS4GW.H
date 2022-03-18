//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   dos4gw.h - DPMI support interface                                  //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _DOS4GW_H
#define _DOS4GW_H

#include <i86.h>
#include "engine/types.h"

// converts real mode addres to flat mode address
void *realtoflat(void *a);
// allocates DOS memory block (in the first 1MB)
bool dosalloc(dword size,void *adr,word *selector);
// frees DOS memory block
bool dosfree(word selector);
// executes 16 bit real mode interrupt
bool int16(int int_no,REGPACKX *r);
//converts physical address to linear address
bool getlinear(dword phys,dword size,byte **linear);
//frees a linear address space
bool freelinear(byte *linear);
// locks a region in the memory
bool lockregion(void *address,dword length);

#endif
