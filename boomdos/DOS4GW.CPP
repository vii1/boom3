//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   dos4gw.cpp - DPMI support implementation                           //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _DOS4GW_CPP
#include "dos4gw.h"

// DOS4GW registers
struct Tdos4gwregs {
  long edi;
  long esi;
  long ebp;
  long qqq;
  long ebx;
  long edx;
  long ecx;
  long eax;
  word flags;
  word es;
  word ds;
  word fs;
  word gs;
  word ip;
  word cs;
  word sp;
  word ss;
};

// converts real mode addres to flat mode address
void *realtoflat(void *a)
{
  return (void *)((word)a+((((dword)a)>>12)&(~15)));
}

int dosalloc_asm(dword size,void *adr,word *selector);

// allocates DOS memory block (in the first 1MB)
bool dosalloc(dword size,void *adr,word *selector)
{
  bool result=false;
  _asm {
    mov  ebx,size
    dec  ebx
    shr  ebx,4
    inc  ebx
    cmp  ebx,65536
    jae  qq
    mov  eax,100h
    int  31h
    jc   qq
    and  eax,65535
    shl  eax,4
    mov  esi,adr
    mov  [esi],eax
    mov  edi,selector
    mov  [edi],dx
    mov  result,1
  qq:
  }
  return result;
}

// frees DOS memory block
bool dosfree(word selector)
{
  byte result;
  _asm {
    mov  dx,selector
    mov  eax,101h
    int  31h
    lahf
    mov  result,ah
  }
  return (result&1)==0;
}

// executes 16 bit real mode interrupt
bool int16(int int_no,REGPACKX *r)
{
  Tdos4gwregs regs;

  regs.eax=r->eax;
  regs.ebx=r->ebx;
  regs.ecx=r->ecx;
  regs.edx=r->edx;
  regs.esi=r->esi;
  regs.edi=r->edi;
  regs.ebp=r->ebp;
  regs.flags=0;
  regs.ds=r->ds;
  regs.es=r->es;
  regs.fs=r->fs;
  regs.gs=r->gs;
  regs.ss=0;
  regs.sp=0;
  regs.ip=0;
  byte result;
  _asm {
    mov  ebx,int_no
    mov  dx,ss
    lea  edi,regs
    push es
    mov  es,dx
    mov  eax,300h
    xor  ecx,ecx
    int  31h
    lahf
    pop  es
    mov  result,ah
  }
  if (result&1) return false;
  r->eax=regs.eax;
  r->ebx=regs.ebx;
  r->ecx=regs.ecx;
  r->edx=regs.edx;
  r->esi=regs.esi;
  r->edi=regs.edi;
  r->ebp=regs.ebp;
  r->flags=regs.flags;
  r->ds=regs.ds;
  r->es=regs.es;
  r->fs=regs.fs;
  r->gs=regs.gs;
  return true;
}

//converts physical address to linear address
bool getlinear(dword phys,dword size,byte **linear)
{
  bool result=false;
  _asm {
    mov  ecx,phys
    mov  edi,size
    mov  edx,linear
    mov  eax,800h
    shld ebx,ecx,16
    shld esi,edi,16
    int  31h
    jc   qq
    shl  ebx,16
    mov  bx,cx
    mov  [edx],ebx
    mov  result,1
  qq:
  }
  return result;
}

//frees a linear address space
bool freelinear(byte *linear)
{
  byte result;
  _asm {
    mov  ecx,linear
    mov  eax,801h
    shld ebx,ecx,16
    int  31h
    lahf
    mov  result,ah
  }
  return (result&1)==0;
}

// locks a region in the memory
bool lockregion(void *address,dword length)
{
  byte result;
  _asm {
    mov  ecx,address
    mov  edi,length
    mov  eax,600h
    shld ebx,ecx,16
    shld esi,edi,16
    int  31h
    lahf
    mov  result,ah
  }
  return (result&1)==0;
}
