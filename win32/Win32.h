//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   win32.h - Windows 95 main window and common controls interface     //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _WIN32_H
#define _WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>

#ifdef _WIN32_CPP
#define GLOBAL
#else
#define GLOBAL extern
#endif

// missing from watcom's commctrl.h
#ifndef PGF_CALCWIDTH
# define PGF_CALCWIDTH       1
# define PGF_CALCHEIGHT      2
#endif

GLOBAL HWND hwndMain;
GLOBAL HINSTANCE hinst;

///////////////////////////////////////////////////////////////////////////////////////
// general window management
bool createmainwindow(char *appname,HINSTANCE _hinst,int menu=0,int icon=0,HBRUSH brush=0,int cursor=0);
bool settitle(char *title);
void getmainwindowsize(int *top,int *left,int *width,int *height);
int mainwindowloop(int accelerator=0,void (*idle)( void )=NULL);
LRESULT CALLBACK defwindowproc( HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

// user defined functions
extern long windowproc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
extern bool oncommand(WPARAM wParam,LPARAM lParam);


///////////////////////////////////////////////////////////////////////////////////////
// common controls
///////////////////////////////////////////////////////////////////////////////////////
// status bar
GLOBAL HWND hwndStatus;
#define IDC_STATUSBAR 'SBAR'

bool createstatusbar(int n=0,int *pos=0);
void setstatustext(char *text,int i=0);
void showstatusbar(bool state);

///////////////////////////////////////////////////////////////////////////////////////
// tool bar
#define MAXTOOLBAR 32
#define MAXTOOLBARS 8

#define TBSTYLE_CHECKED 0x0080

// info for each toolbar button.
struct Ttoolbar {
  // TBSTYLE_BUTTON,TBSTYLE_CHECK,TBSTYLE_CHECK,TBSTYLE_CHECKGROUP,TBSTYLE_SEP,TBSTYLE_DROPDOWN
  int style;
  // unique command. if style is TBSTYLE_DROPDOWN and there should not be arrow button, must be <0
  int command;
  // tip text
  char *tip;
  // text under the bitmap
  char *text;
  // menu handle for dropdown buttons
  HMENU menu;
};

GLOBAL HWND hwndRebar;
#define IDC_REBAR 'RBAR'

bool createrebar( void );
HWND createtoolbar(int n,Ttoolbar *tb,int toolbarbmp,bool flatfl,bool pagerfl);
bool addtorebar(HWND control,int width,int height);
bool showtoolbar(bool state,int index=-1);

void enablecommand(int commandid,bool state);
void checkbutton(int commandid,bool state);

///////////////////////////////////////////////////////////////////////////////////////



#undef GLOBAL

#endif
