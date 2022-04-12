//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   dialog.cpp - Windows 95 dialog handling implementation             //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _DIALOG_CPP
#include "dialog.h"
#include <stdio.h>
#include <commdlg.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////
// common dialog boxes

UINT APIENTRY OFNhook( HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam)
{
  if (uiMsg==WM_INITDIALOG) {
    hdlg=GetParent(hdlg);
    RECT rcOwner,rcDlg;

    GetWindowRect(GetParent(hdlg),&rcOwner);
    GetWindowRect(hdlg,&rcDlg);
    int x=(rcOwner.right+rcOwner.left-rcDlg.right+rcDlg.left)/2;
    int y=(rcOwner.bottom+rcOwner.top-rcDlg.bottom+rcDlg.top)/2;
    POINT p;
    p.x=p.y=0;
    ClientToScreen(hwndMain,&p);
    if (y<p.y) y=p.y;
    SetWindowPos(hdlg,HWND_TOP,x,y,0,0,SWP_NOSIZE);
    return 1;
  }
  return 0;
}
 
bool openfilename(char *filename,char *title,char *filters,char *defext,bool changedir)
{
  OPENFILENAME of;

  memset(&of,0,sizeof(of));
  of.lStructSize=sizeof(of);
  of.hwndOwner=hwndMain;
  of.hInstance=hinst;
  of.lpstrFilter=filters;
  of.nFilterIndex=1;
  of.lpstrFile=filename;
  of.lpstrTitle=title;
  of.nMaxFile=_MAX_PATH-1;
  of.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_ENABLEHOOK|OFN_EXPLORER;
  if (!changedir) of.Flags|=OFN_NOCHANGEDIR;
  of.nFileOffset=0;
  of.nFileExtension=0;
  of.lpstrDefExt=defext;
  of.lpfnHook=OFNhook;
  if (!GetOpenFileName(&of)) return false;
  filenameoffset=of.nFileOffset;
  filenameext=of.nFileExtension;
  return true;
}

bool savefilename(char *filename,char *title,char *filters,char *defext,bool changedir)
{
  OPENFILENAME of;

  memset(&of,0,sizeof(of));
  of.lStructSize=sizeof(of);
  of.hwndOwner=hwndMain;
  of.hInstance=hinst;
  of.lpstrFilter=filters;
  of.nFilterIndex=1;
  of.lpstrFile=filename;
  of.lpstrTitle=title;
  of.nMaxFile=_MAX_PATH-1;
  of.Flags=OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_ENABLEHOOK|OFN_EXPLORER;
  if (!changedir) of.Flags|=OFN_NOCHANGEDIR;
  of.nFileOffset=0;
  of.nFileExtension=0;
  of.lpstrDefExt=defext;
  of.lpfnHook=OFNhook;
  if (!GetSaveFileName(&of)) return false;
  filenameoffset=of.nFileOffset;
  filenameext=of.nFileExtension;
  return true;
}

bool choosecolor(COLORREF *color)
{
  static COLORREF cr[16];
  CHOOSECOLOR cc;

  memset(&cc,0,sizeof(cc));
  cc.lStructSize=sizeof(cc);
  cc.hwndOwner=hwndMain;
  cc.hInstance=0;
  cc.rgbResult=*color;
  cc.lpCustColors=cr;
  cc.Flags=CC_RGBINIT|CC_FULLOPEN;
  if (!ChooseColor(&cc)) return false;
  *color=cc.rgbResult;
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// general dialog management

static DLGPROC currentdlgproc;

static BOOL CALLBACK ModalDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  currentDlg=hwndDlg;
  return currentdlgproc(hwndDlg,uMsg,wParam,lParam);
}

int rundialog(int dialog,DLGPROC DialogProc)
{
  HWND prevdlg=currentDlg;
  DLGPROC prevdlgproc=currentdlgproc;
  currentdlgproc=DialogProc;
  int res=DialogBox(hinst,MAKEINTRESOURCE(dialog),hwndMain,ModalDialogProc);
  currentDlg=prevdlg;
  currentdlgproc=prevdlgproc;
  return res;
}

bool enddialog(int result)
{
  return EndDialog(currentDlg,result)!=0;
}

bool enablecontrol(HWND hwndDlg,int controlid,bool state)
{
  return !EnableWindow(GetDlgItem(hwndDlg,controlid),state);
}

bool enablecontrol(int controlid,bool state)
{
  return !EnableWindow(GetDlgItem(currentDlg,controlid),state);
}


///////////////////////////////////////////////////////////////////////////////////////
// dialog controls

///////////////////////////////////////////////////////////////////////////////////////
// edit box
int eb_settext(HWND hwndDlg,int controlid,char *text)
{
  return SendDlgItemMessage(hwndDlg,controlid,WM_SETTEXT,0,(LPARAM)text);
}

int eb_settext(int controlid,char *text)
{
  return SendDlgItemMessage(currentDlg,controlid,WM_SETTEXT,0,(LPARAM)text);
}

int eb_gettext(HWND hwndDlg,int controlid,char *text,int len)
{
  *(short *)text=len;
  int l=SendDlgItemMessage(hwndDlg,controlid,EM_GETLINE,0,(LPARAM)text);
  text[l]=0;
  return l;
}

int eb_gettext(int controlid,char *text,int len)
{
  *(short *)text=len;
  int l=SendDlgItemMessage(currentDlg,controlid,EM_GETLINE,0,(LPARAM)text);
  text[l]=0;
  return l;
}


int eb_setivalue(HWND hwndDlg,int controlid,int value)
{
  char text[20];
  sprintf(text,"%d",value);
  return SendDlgItemMessage(hwndDlg,controlid,WM_SETTEXT,0,(LPARAM)text);
}

int eb_setivalue(int controlid,int value)
{
  char text[20];
  sprintf(text,"%d",value);
  return SendDlgItemMessage(currentDlg,controlid,WM_SETTEXT,0,(LPARAM)text);
}

int eb_getivalue(HWND hwndDlg,int controlid)
{
  char text[20];
  *(short *)text=sizeof(text)-1;
  int l=SendDlgItemMessage(hwndDlg,controlid,EM_GETLINE,0,(LPARAM)text);
  text[l]=0;
  return atol(text);
}

int eb_getivalue(int controlid)
{
  char text[20];
  *(short *)text=sizeof(text)-1;
  int l=SendDlgItemMessage(currentDlg,controlid,EM_GETLINE,0,(LPARAM)text);
  text[l]=0;
  return atol(text);
}


int eb_setfvalue(HWND hwndDlg,int controlid,double value,int digits)
{
  char text[20];
  sprintf(text,"%.*f",digits,value);
  return SendDlgItemMessage(hwndDlg,controlid,WM_SETTEXT,0,(LPARAM)text);
}

int eb_setfvalue(int controlid,double value,int digits)
{
  char text[20];
  sprintf(text,"%.*f",digits,value);
  return SendDlgItemMessage(currentDlg,controlid,WM_SETTEXT,0,(LPARAM)text);
}

double eb_getfvalue(HWND hwndDlg,int controlid)
{
  char text[20];
  *(short *)text=sizeof(text)-1;
  int l=SendDlgItemMessage(hwndDlg,controlid,EM_GETLINE,0,(LPARAM)text);
  text[l]=0;
  return atof(text);
}

double eb_getfvalue(int controlid)
{
  char text[20];
  *(short *)text=sizeof(text)-1;
  int l=SendDlgItemMessage(currentDlg,controlid,EM_GETLINE,0,(LPARAM)text);
  text[l]=0;
  return atof(text);
}


///////////////////////////////////////////////////////////////////////////////////////
// check box
static WPARAM bsstate(bool fl)
{
  return fl?BST_CHECKED:BST_UNCHECKED;
}

int cb_setcheck(HWND hwndDlg,int controlid,bool state)
{
  return SendDlgItemMessage(hwndDlg,controlid,BM_SETCHECK,bsstate(state),0);
}

int cb_setcheck(int controlid,bool state)
{
  return SendDlgItemMessage(currentDlg,controlid,BM_SETCHECK,bsstate(state),0);
}

bool cb_getcheck(HWND hwndDlg,int controlid)
{
  int s=SendDlgItemMessage(hwndDlg,controlid,BM_GETCHECK,0,0);
  return (s==BST_CHECKED);
}

bool cb_getcheck(int controlid)
{
  int s=SendDlgItemMessage(currentDlg,controlid,BM_GETCHECK,0,0);
  return (s==BST_CHECKED);
}

///////////////////////////////////////////////////////////////////////////////////////
// combo box
void cb_reset(HWND hwndDlg,int controlid)
{
  SendDlgItemMessage(hwndDlg,controlid,CB_RESETCONTENT,0,0);
}

void cb_reset(int controlid)
{
  SendDlgItemMessage(currentDlg,controlid,CB_RESETCONTENT,0,0);
}

int cb_addstring(HWND hwndDlg,int controlid,char *text)
{
  return SendDlgItemMessage(hwndDlg,controlid,CB_ADDSTRING,0,(LPARAM)text);
}

int cb_addstring(int controlid,char *text)
{
  return SendDlgItemMessage(currentDlg,controlid,CB_ADDSTRING,0,(LPARAM)text);
}

int cb_insstring(HWND hwndDlg,int controlid,int index,char *text)
{
  return SendDlgItemMessage(hwndDlg,controlid,CB_INSERTSTRING,index,(LPARAM)text);
}

int cb_insstring(int controlid,int index,char *text)
{
  return SendDlgItemMessage(currentDlg,controlid,CB_INSERTSTRING,index,(LPARAM)text);
}

void cb_delstring(HWND hwndDlg,int controlid,int index)
{
  SendDlgItemMessage(hwndDlg,controlid,CB_DELETESTRING,index,0);
}

void cb_delstring(int controlid,int index)
{
  SendDlgItemMessage(currentDlg,controlid,CB_DELETESTRING,index,0);
}

void cb_setitemdata(HWND hwndDlg,int controlid,int index,int data)
{
  SendDlgItemMessage(hwndDlg,controlid,CB_SETITEMDATA,index,data);
}

void cb_setitemdata(int controlid,int index,int data)
{
  SendDlgItemMessage(currentDlg,controlid,CB_SETITEMDATA,index,data);
}

int cb_getitemdata(HWND hwndDlg,int controlid,int index)
{
  return SendDlgItemMessage(hwndDlg,controlid,CB_GETITEMDATA,index,0);
}

int cb_getitemdata(int controlid,int index)
{
  return SendDlgItemMessage(currentDlg,controlid,CB_GETITEMDATA,index,0);
}

void cb_setcurrent(HWND hwndDlg,int controlid,int n)
{
  SendDlgItemMessage(hwndDlg,controlid,CB_SETCURSEL,n,0);
}

void cb_setcurrent(int controlid,int n)
{
  SendDlgItemMessage(currentDlg,controlid,CB_SETCURSEL,n,0);
}

int cb_getcurrent(HWND hwndDlg,int controlid)
{
  return SendDlgItemMessage(hwndDlg,controlid,CB_GETCURSEL,0,0);
}

int cb_getcurrent(int controlid)
{
  return SendDlgItemMessage(currentDlg,controlid,CB_GETCURSEL,0,0);
}


///////////////////////////////////////////////////////////////////////////////////////
// list box
void lb_reset(HWND hwndDlg,int controlid)
{
  SendDlgItemMessage(hwndDlg,controlid,LB_RESETCONTENT,0,0);
}

void lb_reset(int controlid)
{
  SendDlgItemMessage(currentDlg,controlid,LB_RESETCONTENT,0,0);
}

int lb_addstring(HWND hwndDlg,int controlid,char *text)
{
  return SendDlgItemMessage(hwndDlg,controlid,LB_ADDSTRING,0,(LPARAM)text);
}

int lb_addstring(int controlid,char *text)
{
  return SendDlgItemMessage(currentDlg,controlid,LB_ADDSTRING,0,(LPARAM)text);
}

int lb_insstring(HWND hwndDlg,int controlid,int index,char *text)
{
  return SendDlgItemMessage(hwndDlg,controlid,LB_INSERTSTRING,index,(LPARAM)text);
}

int lb_insstring(int controlid,int index,char *text)
{
  return SendDlgItemMessage(currentDlg,controlid,LB_INSERTSTRING,index,(LPARAM)text);
}

void lb_delstring(HWND hwndDlg,int controlid,int index)
{
  SendDlgItemMessage(hwndDlg,controlid,LB_DELETESTRING,index,0);
}

void lb_delstring(int controlid,int index)
{
  SendDlgItemMessage(currentDlg,controlid,LB_DELETESTRING,index,0);
}

void lb_setitemdata(HWND hwndDlg,int controlid,int index,int data)
{
  SendDlgItemMessage(hwndDlg,controlid,LB_SETITEMDATA,index,data);
}

void lb_setitemdata(int controlid,int index,int data)
{
  SendDlgItemMessage(currentDlg,controlid,LB_SETITEMDATA,index,data);
}

int lb_getitemdata(HWND hwndDlg,int controlid,int index)
{
  return SendDlgItemMessage(hwndDlg,controlid,LB_GETITEMDATA,index,0);
}

int lb_getitemdata(int controlid,int index)
{
  return SendDlgItemMessage(currentDlg,controlid,LB_GETITEMDATA,index,0);
}

void lb_setcurrent(HWND hwndDlg,int controlid,int n)
{
  SendDlgItemMessage(hwndDlg,controlid,LB_SETCURSEL,n,0);
}

void lb_setcurrent(int controlid,int n)
{
  SendDlgItemMessage(currentDlg,controlid,LB_SETCURSEL,n,0);
}

int lb_getcurrent(HWND hwndDlg,int controlid)
{
  return SendDlgItemMessage(hwndDlg,controlid,LB_GETCURSEL,0,0);
}

int lb_getcurrent(int controlid)
{
  return SendDlgItemMessage(currentDlg,controlid,LB_GETCURSEL,0,0);
}

void lb_settabs(HWND hwndDlg,int controlid,int n,int *tabs)
{
  SendDlgItemMessage(hwndDlg,controlid,LB_SETTABSTOPS,n,(LPARAM)tabs);
}

void lb_settabs(int controlid,int n,int *tabs)
{
  SendDlgItemMessage(currentDlg,controlid,LB_SETTABSTOPS,n,(LPARAM)tabs);
}

///////////////////////////////////////////////////////////////////////////////////////
// scroll bar
void sb_setrange(HWND hwndDlg,int controlid,int min,int max)
{
  SendDlgItemMessage(hwndDlg,controlid,SBM_SETRANGE,min,max);
}

void sb_setrange(int controlid,int min,int max)
{
  SendDlgItemMessage(currentDlg,controlid,SBM_SETRANGE,min,max);
}

void sb_setpage(HWND hwndDlg,int controlid,int page,bool redraw)
{
  SCROLLINFO si;
  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE;
  si.nPage=page;
  SendDlgItemMessage(hwndDlg,controlid,SBM_SETSCROLLINFO,redraw,(LPARAM)&si);
}

void sb_setpage(int controlid,int page,bool redraw)
{
  SCROLLINFO si;
  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE;
  si.nPage=page;
  SendDlgItemMessage(currentDlg,controlid,SBM_SETSCROLLINFO,redraw,(LPARAM)&si);
}

int sb_getpage(HWND hwndDlg,int controlid)
{
  SCROLLINFO si;
  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE;
  SendDlgItemMessage(hwndDlg,controlid,SBM_GETSCROLLINFO,0,(LPARAM)&si);
  return si.nPage;
}

int sb_getpage(int controlid)
{
  SCROLLINFO si;
  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE;
  SendDlgItemMessage(currentDlg,controlid,SBM_GETSCROLLINFO,0,(LPARAM)&si);
  return si.nPage;
}

void sb_setpos(HWND hwndDlg,int controlid,int pos,bool redraw)
{
  SendDlgItemMessage(hwndDlg,controlid,SBM_SETPOS,pos,redraw);
}

void sb_setpos(int controlid,int pos,bool redraw)
{
  SendDlgItemMessage(currentDlg,controlid,SBM_SETPOS,pos,redraw);
}

int sb_getpos(HWND hwndDlg,int controlid)
{
  return SendDlgItemMessage(hwndDlg,controlid,SBM_GETPOS,0,0);
}

int sb_getpos(int controlid)
{
  return SendDlgItemMessage(currentDlg,controlid,SBM_GETPOS,0,0);
}

int sb_scroll(HWND hwndDlg,int controlid,WPARAM wParam,LPARAM lParam)
{
  int pos=sb_getpos(hwndDlg,controlid);
  int pagesize=sb_getpage(hwndDlg,controlid);
  switch(LOWORD(wParam)) {
    case SB_LINEDOWN:
      pos++;
      break;
    case SB_PAGEDOWN:
      pos+=pagesize;
      break;
    case SB_LINEUP:
      pos--;
      break;
    case SB_PAGEUP:
      pos-=pagesize;
      break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      pos=HIWORD(wParam);
      break;
  }
  sb_setpos(hwndDlg,controlid,pos);
  return sb_getpos(hwndDlg,controlid);
}

int sb_scroll(int controlid,WPARAM wParam,LPARAM lParam)
{
  int pos=sb_getpos(controlid);
  int pagesize=sb_getpage(controlid);
  switch(LOWORD(wParam)) {
    case SB_LINEDOWN:
      pos++;
      break;
    case SB_PAGEDOWN:
      pos+=pagesize;
      break;
    case SB_LINEUP:
      pos--;
      break;
    case SB_PAGEUP:
      pos-=pagesize;
      break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      pos=HIWORD(wParam);
      break;
  }
  sb_setpos(controlid,pos);
  return sb_getpos(controlid);
}

///////////////////////////////////////////////////////////////////////////////////////
// up-down control
void ud_setrange(HWND hwndDlg,int controlid,int min,int max)
{
  SendDlgItemMessage(hwndDlg,controlid,UDM_SETRANGE,0,(LPARAM)MAKELONG(max,min));
}

void ud_setrange(int controlid,int min,int max)
{
  SendDlgItemMessage(currentDlg,controlid,UDM_SETRANGE,0,(LPARAM)MAKELONG(max,min));
}

void ud_setpos(HWND hwndDlg,int controlid,int pos)
{
  SendDlgItemMessage(hwndDlg,controlid,UDM_SETPOS,0,(LPARAM)MAKELONG(pos,0));
}
void ud_setpos(int controlid,int pos)
{
  SendDlgItemMessage(currentDlg,controlid,UDM_SETPOS,0,(LPARAM)MAKELONG(pos,0));
}

int ud_getpos(HWND hwndDlg,int controlid)
{
  return SendDlgItemMessage(hwndDlg,controlid,UDM_GETPOS,0,0);
}
int ud_getpos(int controlid)
{
  return SendDlgItemMessage(currentDlg,controlid,UDM_GETPOS,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////
// progress bar
void pb_setrange(HWND hwndDlg,int controlid,int min,int max)
{
  SendDlgItemMessage(hwndDlg,controlid,PBM_SETRANGE,0,(LPARAM)MAKELONG(min,max));
}

void pb_setrange(int controlid,int min,int max)
{
  SendDlgItemMessage(currentDlg,controlid,PBM_SETRANGE,0,(LPARAM)MAKELONG(min,max));
}

void pb_setpos(HWND hwndDlg,int controlid,int pos)
{
  SendDlgItemMessage(hwndDlg,controlid,PBM_SETPOS,pos,0);
}

void pb_setpos(int controlid,int pos)
{
  SendDlgItemMessage(currentDlg,controlid,PBM_SETPOS,pos,0);
}

int pb_getpos(HWND hwndDlg,int controlid)
{
  return SendDlgItemMessage(hwndDlg,controlid,PBM_GETPOS,0,0);
}
int pb_getpos(int controlid)
{
  return SendDlgItemMessage(currentDlg,controlid,PBM_GETPOS,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////
// radio group
void rg_setstate(HWND hwndDlg,int controlid,int state)
{
  HWND item0=GetDlgItem(hwndDlg,controlid);
  HWND item=item0;
  for (int i=0;;i++) {
    SendMessage(item,BM_SETCHECK,bsstate(i==state),0);
    item=GetNextDlgGroupItem(hwndDlg,item,FALSE);
    if (!item || item==item0) return;
  }
}

void rg_setstate(int controlid,int state)
{
  HWND item0=GetDlgItem(currentDlg,controlid);
  HWND item=item0;
  for (int i=0;;i++) {
    SendMessage(item,BM_SETCHECK,bsstate(i==state),0);
    item=GetNextDlgGroupItem(currentDlg,item,FALSE);
    if (!item || item==item0) return;
  }
}

int rg_getstate(HWND hwndDlg,int controlid)
{
  HWND item0=GetDlgItem(currentDlg,controlid);
  HWND item=item0;
  for (int i=0;;i++) {
    if (SendMessage(item,BM_GETCHECK,0,0)==BST_CHECKED) return i;
    item=GetNextDlgGroupItem(hwndDlg,item,FALSE);
    if (!item&& item==item0) return -1;
  }
}

int rg_getstate(int controlid)
{
  HWND item0=GetDlgItem(currentDlg,controlid);
  HWND item=item0;
  for (int i=0;;i++) {
    if (SendMessage(item,BM_GETCHECK,0,0)==BST_CHECKED) return i;
    item=GetNextDlgGroupItem(currentDlg,item,FALSE);
    if (!item || item==item0) return -1;
  }
}
