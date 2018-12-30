//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   win32.cpp - Windows 95 main window and common controls implementation  //
//                                                                          //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////////

#define _WIN32_CPP
#include "win32.h"
#include <commctrl.h>
#include <stdio.h>


static HWND toolbars[MAXTOOLBARS];

///////////////////////////////////////////////////////////////////////////////////////
// general window management

static long toolbarnotify(NMHDR *hdr);
static long pagernotify(NMHDR *hdr);

LRESULT CALLBACK WindowProc( HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  static int winwidth=0,winheight=0;
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_SIZE:
      if (lParam) {
        winwidth=LOWORD(lParam);
        winheight=HIWORD(lParam);
      }
      else lParam=MAKELONG(winwidth,winheight);
      if (hwndStatus) SendMessage(hwndStatus,WM_SIZE,wParam,lParam);
      if (hwndRebar) SendMessage(hwndRebar,WM_SIZE,wParam,lParam);
      else if (toolbars[0]) SendMessage(toolbars[0],WM_SIZE,wParam,lParam);
      break;
    case WM_NOTIFY:
      {
        NMHDR *hdr=(NMHDR *)lParam;
        switch (hdr->code) {
          case TBN_DROPDOWN:
          case TTN_GETDISPINFO:
            return toolbarnotify(hdr);
          case PGN_CALCSIZE:
          case PGN_SCROLL:
            return pagernotify(hdr);
          case RBN_HEIGHTCHANGE:
            SendMessage(hwndMain,WM_SIZE,SIZE_RESTORED,MAKELONG(winwidth,winheight));
            break;
        }
      }
      break;
    case WM_COMMAND:
      if (oncommand(wParam,lParam)) return 0;
      break;
  }
  return windowproc(hwnd,uMsg,wParam,lParam);
}

bool createmainwindow(char *appname,HINSTANCE _hinst,int menu,int icon,HBRUSH brush,int cursor)
{
  WNDCLASS cls;

  hinst=_hinst;
  if (menu) cls.lpszMenuName=MAKEINTRESOURCE(menu);
  else cls.lpszMenuName =NULL;

  if (!icon) cls.hIcon=LoadIcon(0,IDI_APPLICATION);
  else cls.hIcon=LoadIcon(hinst,MAKEINTRESOURCE(icon));

  if (!cursor) cls.hCursor=LoadCursor(0,IDC_ARROW);
  else cls.hCursor=LoadCursor(hinst,MAKEINTRESOURCE(cursor));

  cls.lpszClassName=appname;
  cls.hbrBackground=brush;
  cls.hInstance=hinst;
  cls.style=CS_VREDRAW | CS_HREDRAW;
  cls.lpfnWndProc=(WNDPROC)WindowProc;
  cls.cbClsExtra=0;
  cls.cbWndExtra=0;
  if (!RegisterClass(&cls)) return false;

  hwndMain=CreateWindow(appname,appname,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
                        CW_USEDEFAULT,CW_USEDEFAULT,0,0,hinst,0);
  if (!hwndMain) return false;
  ShowWindow(hwndMain,SW_SHOWNORMAL);
  UpdateWindow(hwndMain);
  hwndStatus=NULL;
  INITCOMMONCONTROLSEX initcc;
  initcc.dwSize=sizeof(initcc);
  initcc.dwICC=ICC_BAR_CLASSES|ICC_COOL_CLASSES|ICC_PAGESCROLLER_CLASS;
  InitCommonControlsEx(&initcc);
  return true;
}

bool settitle(char *title)
{
  return SetWindowText(hwndMain,title)!=0;
}

void getmainwindowsize(int *top,int *left,int *width,int *height)
{
  RECT r;
  GetClientRect(hwndMain,&r);
  int w=r.right;
  int h=r.bottom;
  int t=0;
  int l=0;
  POINT p={0,0};
  ClientToScreen(hwndMain,&p);
  if (hwndStatus && (GetWindowLong(hwndStatus,GWL_STYLE)&WS_VISIBLE)) {
    GetWindowRect(hwndStatus,&r);
    h-=r.bottom-r.top+1;
  }
  HWND toolbar=hwndRebar?hwndRebar:toolbars[0];
  if (toolbar && (GetWindowLong(toolbar,GWL_STYLE)&WS_VISIBLE)) {
    GetWindowRect(toolbar,&r);
    t+=r.bottom-r.top+1;
  }
  w-=l;
  h-=t;
  if (w<0) w=0;
  if (h<0) h=0;
  *top=t;
  *left=l;
  *width=w;
  *height=h;
}

int mainwindowloop(int accelerator,void (*idle)( void ))
{
  MSG msg;

  HACCEL hAccel;
  if (accelerator) hAccel=LoadAccelerators(hinst,MAKEINTRESOURCE(accelerator));
  if (idle) {
    while (1) {
      if (PeekMessage(&msg,0,0,0,PM_REMOVE)) {
        if (msg.message==WM_QUIT) break;
        if (!accelerator || !TranslateAccelerator(hwndMain, hAccel, &msg)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      else idle();
    }
  }
  else {
    while (GetMessage(&msg,0,0,0)) {
      if (!accelerator || !TranslateAccelerator(hwndMain, hAccel, &msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
  return msg.wParam;
}

///////////////////////////////////////////////////////////////////////////////////////
// common controls
///////////////////////////////////////////////////////////////////////////////////////
// status bar

bool createstatusbar(int n,int *pos)
{
  hwndStatus=CreateStatusWindow(WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP,"",hwndMain,IDC_STATUSBAR);
  if (!hwndStatus) return false;
  if (n) SendMessage(hwndStatus,SB_SETPARTS,n,(LPARAM)pos);
  else SendMessage(hwndStatus,SB_SIMPLE,TRUE,0);
  ShowWindow(hwndStatus,SW_SHOW);
  return true;
}

void setstatustext(char *text,int i)
{
  SendMessage(hwndStatus,SB_SETTEXT,i,(LPARAM)text);
}

void showstatusbar(bool state)
{
  if (hwndStatus) {
    long style=GetWindowLong(hwndStatus,GWL_STYLE);
    if (state) style|=WS_VISIBLE;
    else style&=~WS_VISIBLE;
    SetWindowLong(hwndStatus,GWL_STYLE,style);
    SendMessage(hwndMain,WM_SIZE,0,0);
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// tool bar

static Ttoolbar *toolbardata[MAXTOOLBARS];
static int nbuttons[MAXTOOLBARS];
static int ntoolbars=0;

static long toolbarnotify(NMHDR *hdr)
{
  int n=hdr->idFrom-1;
  switch (hdr->code) {
    case TBN_DROPDOWN:
    {
      NMTOOLBAR *toolbarstr=(NMTOOLBAR *)hdr;
      RECT rc;
      TPMPARAMS tpm;
      int id=toolbarstr->iItem;
      SendMessage(toolbars[n], TB_GETRECT, id,(LPARAM)&rc);
      id=SendMessage(toolbars[n], TB_COMMANDTOINDEX, id,0);
      MapWindowPoints(toolbars[n],  HWND_DESKTOP, (LPPOINT)&rc, 2);                         
      tpm.cbSize = sizeof(TPMPARAMS);
      tpm.rcExclude.top    = rc.top;
      tpm.rcExclude.left   = rc.left;
      tpm.rcExclude.bottom = rc.bottom;
      tpm.rcExclude.right  = rc.right;
      if (toolbardata[n][id].menu)
        TrackPopupMenuEx(toolbardata[n][id].menu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,
                       rc.left, rc.bottom, hwndMain, &tpm);         
      return FALSE;
    }
    case TTN_GETDISPINFO:
    {
      NMTTDISPINFO *tti=(NMTTDISPINFO *)hdr;
      TOOLINFO ti;
      ti.cbSize=sizeof(ti);
      SendMessage(hdr->hwndFrom,TTM_GETCURRENTTOOL,0,(LPARAM)&ti);
      int id=SendMessage(ti.hwnd, TB_COMMANDTOINDEX,hdr->idFrom,0);
      for (int i=0;i<ntoolbars;i++)
        if (toolbars[i]==ti.hwnd) {
          tti->lpszText=toolbardata[i][id].tip;
          tti->hinst=0;
          tti->uFlags=0;
          return TRUE;
        }
    }
    return FALSE;
  }
  return FALSE;
}

HWND createtoolbar(int n,Ttoolbar *tb,int toolbarbmp,bool flatfl,bool pagerfl)
{
  if (n>MAXTOOLBAR) return NULL;
  if (ntoolbars>MAXTOOLBARS) return NULL;
  if (ntoolbars>1 && !hwndRebar) return NULL;

  int nbmp=0;
  int nstr=0;
  HBITMAP bmp=LoadBitmap(hinst,MAKEINTRESOURCE(toolbarbmp));
  BITMAP bmpinfo;
  GetObject(bmp,sizeof(bmpinfo),&bmpinfo);
  DeleteObject(bmp);

  TBBUTTON buttons[MAXTOOLBAR];
  int i;
  for (i=0;i<n;i++) {
    buttons[i].fsState=TBSTATE_ENABLED|(tb[i].style==TBSTYLE_CHECKED?TBSTATE_CHECKED:0);
    if (tb[i].style==TBSTYLE_CHECKED) tb[i].style=TBSTYLE_CHECK;
    buttons[i].iBitmap=(tb[i].style&TBSTYLE_SEP)?0:nbmp++;
    buttons[i].idCommand=tb[i].command;
    buttons[i].fsStyle=tb[i].style;
    buttons[i].dwData=0;
    buttons[i].iString=tb[i].text?nstr++:-1;
  }
  int style=WS_CHILD|TBSTYLE_TOOLTIPS;
  if (flatfl) style|=TBSTYLE_FLAT;
  if (!hwndRebar) style|=TBSTYLE_WRAPABLE;
  else style|=CCS_NODIVIDER;

  HWND parent=hwndRebar?hwndRebar:hwndMain;
  HWND toolbar=CreateToolbarEx(parent,style,ntoolbars+1,nbmp,hinst,toolbarbmp,buttons,n,
                0,0,bmpinfo.bmWidth/nbmp,bmpinfo.bmHeight,sizeof(TBBUTTON));
  if (!toolbar) return NULL;

  bool arrowfl=false;
  for (i=0;i<n;i++) {
    if (tb[i].text) {
      char text[100];
      strcpy(text,tb[i].text);
      text[strlen(text)+1]=0;
      SendMessage(toolbar,TB_ADDSTRING,0,(LPARAM)text);
    }
    if (tb[i].command>0 && tb[i].style&TBSTYLE_DROPDOWN) arrowfl=true;
  }
  if (arrowfl) SendMessage(toolbar,TB_SETEXTENDEDSTYLE,0,TBSTYLE_EX_DRAWDDARROWS);
  SendMessage(toolbar,TB_AUTOSIZE,0,0);

  if (hwndRebar) {
    SIZE size;
    RECT rc;
    SetWindowLong(toolbar,GWL_STYLE,style|CCS_NORESIZE);
    GetWindowRect(toolbar,&rc);
    SendMessage(toolbar,TB_GETMAXSIZE,0,(LPARAM)&size);
    if (pagerfl) {
      HWND pager=CreateWindow(WC_PAGESCROLLER,"",WS_CHILD|WS_VISIBLE|PGS_HORZ|CCS_NORESIZE,
                  0,0,0,0,hwndRebar,(HMENU)IDC_REBAR,hinst,0);
      SetParent(toolbar,pager);
      SendMessage(pager,PGM_SETCHILD,0,(LPARAM)toolbar);
      addtorebar(pager,size.cx+22,rc.bottom-rc.top);
    }
    else {
      addtorebar(toolbar,size.cx+22,rc.bottom-rc.top);
    }
  }
  ShowWindow(toolbar,SW_SHOW);

  toolbars[ntoolbars]=toolbar;
  toolbardata[ntoolbars]=tb;
  nbuttons[ntoolbars]=n;
  ntoolbars++;
  return toolbar;
}

bool createrebar( void )
{
  hwndRebar=CreateWindowEx(WS_EX_TOOLWINDOW,REBARCLASSNAME,"",
              RBS_AUTOSIZE|WS_CHILD|WS_VISIBLE|CCS_TOP|RBS_VARHEIGHT|
              WS_BORDER|RBS_BANDBORDERS,
              0,0,0,0,hwndMain,0,hinst,0);
  if (!hwndRebar) return false;
  REBARINFO rbi;
  rbi.cbSize=sizeof(REBARINFO);
  rbi.fMask=0;
  rbi.himl=(HIMAGELIST)NULL;
  SendMessage(hwndRebar,RB_SETBARINFO,0,(LPARAM)&rbi);
  ShowWindow(hwndRebar,SW_SHOW);
  return true;
}

static HWND pagerchild;

static BOOL CALLBACK EnumChildProc(HWND hwnd,LPARAM lParam)
{
  pagerchild=hwnd;
  return FALSE;
}

static long pagernotify(NMHDR *hdr)
{
  switch (hdr->code) {
    case PGN_CALCSIZE:
      {
        NMPGCALCSIZE *pagerstr=(NMPGCALCSIZE *)hdr;
        if (pagerstr->dwFlag==PGF_CALCWIDTH) {
          pagerchild=NULL;
          EnumChildWindows(hdr->hwndFrom,EnumChildProc,0);
          if (pagerchild) {
            SIZE size;
            SendMessage(pagerchild,TB_GETMAXSIZE,0,(LPARAM)&size);
            pagerstr->iWidth=size.cx;
          }
        }
      }
      break;
    case PGN_SCROLL:
      ((NMPGSCROLL *)hdr)->iScroll=20;
      break;
  }
  return FALSE;
}

bool addtorebar(HWND control,int width,int height)
{
  REBARBANDINFO rbBand;
  rbBand.cbSize=sizeof(REBARBANDINFO);
  rbBand.fMask=RBBIM_STYLE|RBBIM_CHILD|RBBIM_CHILDSIZE|RBBIM_SIZE|RBBIM_HEADERSIZE;
  rbBand.fStyle=RBBS_CHILDEDGE|RBBS_VARIABLEHEIGHT;

  rbBand.hwndChild=control;
  rbBand.cxMinChild=10;
  rbBand.cyMinChild=height;
  rbBand.cyChild=height;
  rbBand.cx=width;
  rbBand.cxHeader=16;
  rbBand.cyMaxChild=height;
  rbBand.cyIntegral=height;
  if (!SendMessage(hwndRebar,RB_INSERTBAND,(WPARAM)-1,(LPARAM)&rbBand)) return false;
  return true;
}

void enablecommand(int commandid,bool state)
{
  for (int t=0;t<ntoolbars;t++) {
    int bstate=SendMessage(toolbars[t],TB_GETSTATE,commandid,0);
    if (bstate!=-1) {
      if (state) bstate|=TBSTATE_ENABLED;
      else bstate&=~TBSTATE_ENABLED;
      SendMessage(toolbars[t],TB_SETSTATE,commandid,MAKELONG(bstate,0));
      break;
    }
  }
  EnableMenuItem(GetMenu(hwndMain),commandid,state?MF_ENABLED:MF_GRAYED);
}

void checkbutton(int commandid,bool state)
{
  for (int t=0;t<ntoolbars;t++) {
    int bstate=SendMessage(toolbars[t],TB_GETSTATE,commandid,0);
    if (bstate!=-1) {
      if (state) bstate|=TBSTATE_CHECKED;
      else bstate&=~TBSTATE_CHECKED;
      SendMessage(toolbars[t],TB_SETSTATE,commandid,MAKELONG(bstate,0));
      break;
    }
  }
}

bool showtoolbar(bool state,int index)
{
  if (index<0) {
    HWND toolbar=hwndRebar?hwndRebar:toolbars[0];
    if (toolbar) {
      long style=GetWindowLong(toolbar,GWL_STYLE);
      if (state) style|=WS_VISIBLE;
      else style&=~WS_VISIBLE;
      SetWindowLong(toolbar,GWL_STYLE,style);
      SendMessage(hwndMain,WM_SIZE,0,0);
      return true;
    }
    return false;
  }
  else if (hwndRebar && index<ntoolbars) {
    for (int i=0;;i++) {
      REBARBANDINFO rbi;
      rbi.cbSize=sizeof(rbi);
      rbi.fMask=RBBIM_CHILD;
      if (!SendMessage(hwndRebar,RB_GETBANDINFO,i,(LPARAM)&rbi)) return false;
      if (rbi.hwndChild==toolbars[index]) {
        SendMessage(hwndRebar,RB_SHOWBAND,i,state);
        SendMessage(hwndMain,WM_SIZE,0,0);
        return true;
      }
      pagerchild=NULL;
      EnumChildWindows(rbi.hwndChild,EnumChildProc,0);
      if (pagerchild==toolbars[index]) {
        SendMessage(hwndRebar,RB_SHOWBAND,i,state);
        SendMessage(hwndMain,WM_SIZE,0,0);
        return true;
      }
    }
  }
  return false;
}
