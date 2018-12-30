//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   dialog.h - Windows 95 dialog handling interface                    //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _DIALOG_H
#define _DIALOG_H

#include "win32.h"

#ifdef _DIALOG_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif


///////////////////////////////////////////////////////////////////////////////////////
// common dialog boxes

GLOBAL int filenameoffset;
GLOBAL int filenameext;
bool openfilename(char *filename,char *title="Open",char *filters="All Files\0*.*\0",char *defext="",bool changedir=false);
bool savefilename(char *filename,char *title="Save",char *filters="All Files\0*.*\0",char *defext="",bool changedir=false);
bool choosecolor(COLORREF *color);

///////////////////////////////////////////////////////////////////////////////////////
// general dialog management
GLOBAL HWND currentDlg;

int rundialog(int dialog,DLGPROC DialogProc);
bool enddialog(int result);

bool enablecontrol(HWND hwndDlg,int controlid,bool state);
bool enablecontrol(int controlid,bool state);

///////////////////////////////////////////////////////////////////////////////////////
// dialog controls
///////////////////////////////////////////////////////////////////////////////////////
// edit box
int eb_settext(HWND hwndDlg,int controlid,char *text);
int eb_settext(int controlid,char *text);
int eb_gettext(HWND hwndDlg,int controlid,char *text,int len);
int eb_gettext(int controlid,char *text,int len);

int eb_setivalue(HWND hwndDlg,int controlid,int value);
int eb_setivalue(int controlid,int value);
int eb_getivalue(HWND hwndDlg,int controlid);
int eb_getivalue(int controlid);

int eb_setfvalue(HWND hwndDlg,int controlid,double value,int digits=2);
int eb_setfvalue(int controlid,double value,int digits=2);
double eb_getfvalue(HWND hwndDlg,int controlid);
double eb_getfvalue(int controlid);

///////////////////////////////////////////////////////////////////////////////////////
// check box
int cb_setcheck(HWND hwndDlg,int controlid,bool state);
int cb_setcheck(int controlid,bool state);
bool cb_getcheck(HWND hwndDlg,int controlid);
bool cb_getcheck(int controlid);

///////////////////////////////////////////////////////////////////////////////////////
// combo box
void cb_reset(HWND hwndDlg,int controlid);
void cb_reset(int controlid);

int cb_addstring(HWND hwndDlg,int controlid,char *text);
int cb_addstring(int controlid,char *text);
int cb_insstring(HWND hwndDlg,int controlid,int index,char *text);
int cb_insstring(int controlid,int index,char *text);
void cb_delstring(HWND hwndDlg,int controlid,int index);
void cb_delstring(int controlid,int index);

void cb_setitemdata(HWND hwndDlg,int controlid,int index,int data);
void cb_setitemdata(int controlid,int index,int data);
int cb_getitemdata(HWND hwndDlg,int controlid,int index);
int cb_getitemdata(int controlid,int index);

void cb_setcurrent(HWND hwndDlg,int controlid,int n);
void cb_setcurrent(int controlid,int n);
int cb_getcurrent(HWND hwndDlg,int controlid);
int cb_getcurrent(int controlid);

///////////////////////////////////////////////////////////////////////////////////////
// list box
void lb_reset(HWND hwndDlg,int controlid);
void lb_reset(int controlid);

int lb_addstring(HWND hwndDlg,int controlid,char *text);
int lb_addstring(int controlid,char *text);
int lb_insstring(HWND hwndDlg,int controlid,int index,char *text);
int lb_insstring(int controlid,int index,char *text);
void lb_delstring(HWND hwndDlg,int controlid,int index);
void lb_delstring(int controlid,int index);

void lb_setitemdata(HWND hwndDlg,int controlid,int index,int data);
void lb_setitemdata(int controlid,int index,int data);
int lb_getitemdata(HWND hwndDlg,int controlid,int index);
int lb_getitemdata(int controlid,int index);

void lb_setcurrent(HWND hwndDlg,int controlid,int n);
void lb_setcurrent(int controlid,int n);
int lb_getcurrent(HWND hwndDlg,int controlid);
int lb_getcurrent(int controlid);

void lb_settabs(HWND hwndDlg,int controlid,int n,int *tabs);
void lb_settabs(int controlid,int n,int *tabs);

///////////////////////////////////////////////////////////////////////////////////////
// scroll bar
void sb_setrange(HWND hwndDlg,int controlid,int min,int max);
void sb_setrange(int controlid,int min,int max);

void sb_setpage(HWND hwndDlg,int controlid,int page,bool redraw=true);
void sb_setpage(int controlid,int page,bool redraw=true);
int sb_getpage(HWND hwndDlg,int controlid);
int sb_getpage(int controlid);

void sb_setpos(HWND hwndDlg,int controlid,int pos,bool redraw=true);
void sb_setpos(int controlid,int pos,bool redraw=true);
int sb_getpos(HWND hwndDlg,int controlid);
int sb_getpos(int controlid);

int sb_scroll(HWND hwndDlg,int controlid,WPARAM wParam,LPARAM lParam);
int sb_scroll(int controlid,WPARAM wParam,LPARAM lParam);

///////////////////////////////////////////////////////////////////////////////////////
// up-down control
void ud_setrange(HWND hwndDlg,int controlid,int min,int max);
void ud_setrange(int controlid,int min,int max);

void ud_setpos(HWND hwndDlg,int controlid,int pos);
void ud_setpos(int controlid,int pos);
int ud_getpos(HWND hwndDlg,int controlid);
int ud_getpos(int controlid);

///////////////////////////////////////////////////////////////////////////////////////
// progress bar
void pb_setrange(HWND hwndDlg,int controlid,int min,int max);
void pb_setrange(int controlid,int min,int max);

void pb_setpos(HWND hwndDlg,int controlid,int pos);
void pb_setpos(int controlid,int pos);
int pb_getpos(HWND hwndDlg,int controlid);
int pb_getpos(int controlid);

///////////////////////////////////////////////////////////////////////////////////////
// radio group
void rg_setstate(HWND hwndDlg,int controlid,int state);
void rg_setstate(int controlid,int state);
int rg_getstate(HWND hwndDlg,int controlid);
int rg_getstate(int controlid);

///////////////////////////////////////////////////////////////////////////////////////
#undef GLOBAL

#endif
