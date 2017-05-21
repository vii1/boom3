//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Editor                                                      //
//                                                                      //
//   boomedit.cpp - editor's main file: Windows 95 user interface       //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _BOOMEDIT_CPP
#include "boomedit.h"
#include "boom.h"
#include "draw.h"
#include "geom.h"
#include "player.h"
#include "draw2d.h"
#include "resource.h"
#include "select.h"
#include "contour.h"
#include "vertex.h"
#include "sector.h"
#include "cluster.h"
#include "dialog.h"
#include "write.h"
#include "read.h"
#include "texture.h"
#include "readbmp.h"

#include <stdio.h>
#include <crtdbg.h>

///////////////////////////////////////////////////////////////////////////////////////
// local variables

#define DSCALE 1.1 // step for changing the scale

// drag mode for the left mouse button
#define drVERTEX  1
#define drBOX     2
#define drFLOOR   3
#define drCEILING 4
static int msdrag=0;

#define drXY      1
#define drXZ      2
#define drYZ      3
// drag view for the left mouse button
static int dragview1=0;
// drag view for the right mouse button
static int dragview2=0;

static HWND hwndSector;
static HWND hwndClusters;
static HWND hwndTextures;
static int dialogx,dialogy;

static int dragposx,dragposy; // current drag position
static RECT selbox; // current vertex selection box
static coord3d editx0,edity0,editz0; // current center of the views in map coordinates

static bool viewxyfl=true,viewxzfl=true,viewyzfl=true,view3dfl=true; // flags for each view
// flags for the status bar and the toolbars
static bool statusbarfl=true,filetoolbarfl=true,viewtoolbarfl=true,edittoolbarfl=true;
// sector about to be joined
static Tsector *joinsc;
// palette is empty
static bool nopalettefl;

// timeout in miliseconds for the messages in the status bar
#define TIMEOUT 2000
static int messagetimeout=0;

// current edit mode
#define mSELECT       0
#define mCONTOUR      1
#define mVERTEX       2
#define mJOIN         3
#define mSELCONTOUR   4
#define mSELECTSP     5
#define mSELECTCP     6
static int mode;

// currently edited contour
static Tnode *contour,*lastc;

///////////////////////////////////////////////////////////////////////////////////////

static void delcontour( void );

// sets the current edit mode
static bool setmode(int m)
{
  if (m==mode) return true;
  switch (mode) {
    case mCONTOUR:
      delcontour();
      break;
  }
  if (m==mSELECT && !messagetimeout) setmessage(NULL);
  mode=m;
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// drawing functions

// draws the vertex selection box
void drawselbox(HDC hdc)
{
  if (msdrag==drBOX) Rectangle(hdc,selbox.left,selbox.top,selbox.right,selbox.bottom);
}

// draws the currently edited contour
void drawcontour(HDC hdc)
{
  if (contour) {
    MoveTo(hdc,xymap2scrx(contour->x),xymap2scry(contour->y));
    for (Tnode *c=contour->next;c;c=c->next)
      LineTo(hdc,xymap2scrx(c->x),xymap2scry(c->y));
    if (!lastc) LineTo(hdc,xymap2scrx(contour->x),xymap2scry(contour->y));
  }
}

// bitmap for the 3D view
static HBITMAP hbmp=NULL;
static byte *hbmpbuffer;

// initializes the 3D view
static void drawinit(int w,int h)
{
  if (hbmp) DeleteObject(hbmp),hbmp=NULL;
  BITMAPINFO *pbmi=(BITMAPINFO *)malloc(sizeof(BITMAPINFO)+255*sizeof(RGBQUAD));
  for (int i=0;i<256;i++) {
    pbmi->bmiColors[i].rgbRed=palette[i][0];
    pbmi->bmiColors[i].rgbGreen=palette[i][1];
    pbmi->bmiColors[i].rgbBlue=palette[i][2];
  }
  pbmi->bmiHeader.biSize=sizeof(pbmi->bmiHeader);
  pbmi->bmiHeader.biWidth=w;
  pbmi->bmiHeader.biHeight=-h;
  pbmi->bmiHeader.biPlanes=1;
  pbmi->bmiHeader.biBitCount=8;
  pbmi->bmiHeader.biCompression=BI_RGB;
  pbmi->bmiHeader.biSizeImage=0;
  pbmi->bmiHeader.biXPelsPerMeter=0;
  pbmi->bmiHeader.biYPelsPerMeter=0;
  pbmi->bmiHeader.biClrUsed=0;
  pbmi->bmiHeader.biClrImportant=0;
  hbmp=CreateDIBSection(NULL,pbmi,DIB_RGB_COLORS,(void **)&hbmpbuffer,NULL,0);
  memset(hbmpbuffer,0,w*h);
  free(pbmi);
  draw_init(hbmpbuffer,w,h);
}

// draws the 3D view
static void draw3dview(HDC hdc,int x,int y)
{
  HDC hsrc=CreateCompatibleDC(hdc);
  SelectObject(hsrc,hbmp);
  BitBlt(hdc,x,y,scr_dx,scr_dy,hsrc,0,0,SRCCOPY);
  DeleteDC(hsrc);
}

// frees the allocated memory for the 3D view
void drawdone( void )
{
  if (hbmp) DeleteObject(hbmp),hbmp=NULL;
  draw_done();
}

// updates the mouse cursor image
static void updatecursor( void )
{
  if (dragview2) SetCursor(LoadCursor(0,IDC_SIZEALL));
  else SetCursor(LoadCursor(0,IDC_CROSS));
}

///////////////////////////////////////////////////////////////////////////////////////
// various updates

// checks if the point is inside the rectangle
static bool inrect(int x,int y,RECT *r)
{
  return (x>r->left && x<r->right && y>r->top && y<r->bottom);
}

// updates the coordinates on the status bar
static void updatecoord(LPARAM coord)
{
  char s[256]="";
  int x=LOWORD(coord);
  int y=HIWORD(coord);
  if (dragview1==drXY || (dragview1==0 && inrect(x,y,&viewxy))) {
    sprintf(s,"X=%d",(int)xyscr2mapx(x));
    setstatustext(s,0);
    sprintf(s,"Y=%d",(int)xyscr2mapy(y));
    setstatustext(s,1);
  }
  else if (dragview1==drXZ || (dragview1==0 && inrect(x,y,&viewxz))) {
    sprintf(s,"X=%d",(int)xzscr2mapx(x));
    setstatustext(s,0);
    sprintf(s,"Z=%d",(int)xzscr2mapz(y));
    setstatustext(s,1);
  }
  else if (dragview1==drYZ || (dragview1==0 && inrect(x,y,&viewyz))) {
    sprintf(s,"Y=%d",(int)yzscr2mapy(x));
    setstatustext(s,0);
    sprintf(s,"Z=%d",(int)yzscr2mapz(y));
    setstatustext(s,1);
  }
  else {
    setstatustext("",0);
    setstatustext("",1);
  }
}

// updates the views
static void updateviewxy( void )
{
  InvalidateRect(hwndMain,&viewxy,0);
}

static void updateviewxz( void )
{
  InvalidateRect(hwndMain,&viewxz,0);
}

static void updateviewyz( void )
{
  InvalidateRect(hwndMain,&viewyz,0);
}

static void updateview3d( void )
{
  InvalidateRect(hwndMain,&view3d,0);
}

static void updateview2d( void )
{
  updateviewxy();
  updateviewxz();
  updateviewyz();
}

static void updateviewall( void )
{
  updateviewxy();
  updateviewxz();
  updateviewyz();
  updateview3d();
}

static void updateview0( void )
{
  viewxyx0=editx0-vieww1/(2*scale);
  viewxyy0=edity0-viewh1/(2*scale);
  viewxzx0=editx0-vieww2/(2*scale);
  viewxzz0=editz0-viewh2/(2*scale);
  viewyzy0=edity0-vieww3/(2*scale);
  viewyzz0=editz0-viewh3/(2*scale);
  updateview2d();
}

// updates the size of the views
static void calculatesize( void )
{
  int top,left,width,height;
  getmainwindowsize(&top,&left,&width,&height);
  dialogy=top;
  dialogx=width;
  RECT r;

  if (hwndSector) {
    GetWindowRect(hwndSector,&r);
    dialogx=width-(r.right-r.left);
    MoveWindow(hwndSector,dialogx,dialogy-1,r.right-r.left,r.bottom-r.top,TRUE);
    dialogy+=r.bottom-r.top;
  }

  if (hwndClusters) {
    GetWindowRect(hwndClusters,&r);
    dialogx=width-(r.right-r.left);
    MoveWindow(hwndClusters,dialogx,dialogy-1,r.right-r.left,r.bottom-r.top,TRUE);
    dialogy+=r.bottom-r.top;
  }

  if (hwndTextures) {
    GetWindowRect(hwndTextures,&r);
    dialogx=width-(r.right-r.left);
    MoveWindow(hwndTextures,dialogx,dialogy-1,r.right-r.left,r.bottom-r.top,TRUE);
    dialogy+=r.bottom-r.top;
  }
  if (dialogy==top) dialogy=0;
  width=dialogx;

  int w2=width/2;
  int h2=height/2;

  if (viewxyfl) {
    viewxy.left=left;
    viewxy.top=top;
    viewxy.right=left+(view3dfl?w2:width);
    viewxy.bottom=top+((viewxzfl || viewyzfl)?h2:height);
    if (viewxy.right<viewxy.left) viewxy.right=viewxy.left;
    if (viewxy.bottom<viewxy.top) viewxy.bottom=viewxy.top;
    vieww1=viewxy.right-viewxy.left;
    viewh1=viewxy.bottom-viewxy.top;
  }
  else viewxy.left=viewxy.right=viewxy.bottom=viewxy.top=viewh1=0;

  if (viewxzfl) {
    viewxz.left=left;
    viewxz.top=top+((viewxyfl || view3dfl)?(h2+1):0);
    viewxz.right=left+((viewyzfl)?w2:width);
    viewxz.bottom=top+height;
    if (viewxz.right<viewxz.left) viewxz.right=viewxz.left;
    if (viewxz.bottom<viewxz.top) viewxz.bottom=viewxz.top;
    vieww2=viewxz.right-viewxz.left;
    viewh2=viewxz.bottom-viewxz.top;
  }
  else viewxz.left=viewxz.right=viewxz.bottom=viewxz.top=viewh2=0;

  if (viewyzfl) {
    viewyz.left=left+(viewxzfl?(w2+1):0);
    viewyz.top=top+((viewxyfl || view3dfl)?(h2+1):0);
    viewyz.right=left+width;
    viewyz.bottom=top+height;
    if (viewyz.right<viewyz.left) viewyz.right=viewyz.left;
    if (viewyz.bottom<viewyz.top) viewyz.bottom=viewyz.top;
    viewh3=viewyz.bottom-viewyz.top;
    vieww3=width-viewyz.left;
  }
  else viewyz.left=viewyz.right=viewyz.bottom=viewyz.top=viewh3=0;

  if (view3dfl) {
    view3d.left=(viewxyfl)?(w2+1):0;
    view3d.top=top;
    view3d.right=left+width;
    view3d.bottom=top+((viewxzfl || viewyzfl)?h2:height);
    if (view3d.right<view3d.left) view3d.right=view3d.left;
    if (view3d.bottom<view3d.top) view3d.bottom=view3d.top;
    drawinit(view3d.right-view3d.left+1,view3d.bottom-view3d.top+1);
    setscreensize(view3d.right-view3d.left+1,view3d.bottom-view3d.top+1);
  }
  else view3d.left=view3d.right=view3d.bottom=view3d.top=0;

  updateview0();
  InvalidateRect(hwndMain,NULL,0);
  UpdateWindow(hwndMain);
}

// sets the message in the status bar
void setmessage(char *s,bool timeout)
{
  if (s) setstatustext(s,2);
  else setstatustext("",2);
  int time=timeGetTime();
  if (timeout) messagetimeout=time+TIMEOUT;
  else messagetimeout=0;
}

// enables/disables the buttons on the toolbar and menu items
static void updatecommands( void )
{
  enablecommand(ID_TOGGLEXY,!viewxyfl || viewxzfl || viewyzfl || view3dfl);
  enablecommand(ID_TOGGLEXZ,viewxyfl || !viewxzfl || viewyzfl || view3dfl);
  enablecommand(ID_TOGGLEYZ,viewxyfl || viewxzfl || !viewyzfl || view3dfl);
  enablecommand(ID_TOGGLE3D,viewxyfl || viewxzfl || viewyzfl || !view3dfl);
  checkbutton(ID_TOGGLEXY,viewxyfl);
  checkbutton(ID_TOGGLEXZ,viewxzfl);
  checkbutton(ID_TOGGLEYZ,viewyzfl);
  checkbutton(ID_TOGGLE3D,view3dfl);
}

///////////////////////////////////////////////////////////////////////////////////////
// dialog support

//// About
// dialog function for about box
BOOL CALLBACK AboutProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_INITDIALOG:
      return 1;
    case WM_COMMAND:
      if (HIWORD(wParam)==BN_CLICKED)
        switch (LOWORD(wParam)) {
          case IDCANCEL:
            enddialog(0);
            return 1;
        }
  }
  return 0;
}

static void updatemappingdlg(int p);

//// Sector Info
// update the sector info window
static void updatesectordlg( void )
{
  updatemappingdlg(-1);
  if (!hwndSector) return;
  if (cursector) {
    eb_setivalue(hwndSector,IDC_EDITLINES,cursector->linesnum);
    Tcluster *c,*c0=findcluster(cursector);
    int i;
    for (c=map->clusters,i=0;c!=c0;c=c->next,i++);
    eb_setivalue(hwndSector,IDC_CLUSTER,i);
  }
  else {
    eb_settext(hwndSector,IDC_EDITLINES,"");
    eb_settext(hwndSector,IDC_CLUSTER,"");
  }
}

// dialog function for the sector info window
BOOL CALLBACK SectorInfoProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_INITDIALOG:
      {
        RECT r;
        GetWindowRect(hwndDlg,&r);
        MoveWindow(hwndDlg,-1000,-1000,r.right-r.left,r.bottom-r.top,0);
      }
      return 1;
  }
  return 0;
}

//// Clusters
// updates the clusters window
static void updateclustersdlg(HWND hwndDlg,int n)
{
  if (hwndDlg) {
    int i;
    Tcluster *c;
    lb_reset(hwndDlg,IDC_CLUSTERS);
    for (i=0,c=map->clusters;c;c=c->next,i++) {
      char s[20];
      s[0]='v';
      sprintf(s+c->visible,"\tCLUSTER %d",i);
      lb_addstring(hwndDlg,IDC_CLUSTERS,s);
      lb_setitemdata(hwndDlg,IDC_CLUSTERS,i,(int)c);
    }
    lb_setcurrent(hwndDlg,IDC_CLUSTERS,n);
  }
}

// dialog function for the clusters window
BOOL CALLBACK ClustersProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_INITDIALOG:
      {
        int tabs[1]={10};
        lb_settabs(hwndDlg,IDC_CLUSTERS,1,tabs);
        updateclustersdlg(hwndDlg,0);
        RECT r;
        GetWindowRect(hwndDlg,&r);
        MoveWindow(hwndDlg,-1000,-1000,r.right-r.left,r.bottom-r.top,0);
        return 1;
      }
    case WM_COMMAND:
      if (LOWORD(wParam)==IDC_CLUSTERS && HIWORD(wParam)==LBN_DBLCLK) {
        int i=lb_getcurrent(hwndDlg,IDC_CLUSTERS);
        Tcluster *c=(Tcluster *)lb_getitemdata(hwndDlg,IDC_CLUSTERS,i);
        if (c->visible) hidecluster(c);
        else showcluster(c);
        updateview2d();
        char s[20];
        s[0]='v';
        sprintf(s+c->visible,"\tCLUSTER %d",i);
        lb_delstring(hwndDlg,IDC_CLUSTERS,i);
        lb_insstring(hwndDlg,IDC_CLUSTERS,i,s);
        lb_setitemdata(hwndDlg,IDC_CLUSTERS,i,(int)c);
        lb_setcurrent(hwndDlg,IDC_CLUSTERS,i);
      }          
      break;
  }
  return 0;
}

//// Sector Properties
// dialog function for the sector properties dialog
BOOL CALLBACK SectorProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  static int cluster;
  int l;
  switch (uMsg) {
    case WM_INITDIALOG:
      {
        eb_setfvalue(IDC_EDITZFA,cursector->zfa,3);
        eb_setfvalue(IDC_EDITZFB,cursector->zfb,3);
        eb_setfvalue(IDC_EDITZFC,cursector->zfc,3);
        eb_setfvalue(IDC_EDITZCA,cursector->zca,3);
        eb_setfvalue(IDC_EDITZCB,cursector->zcb,3);
        eb_setfvalue(IDC_EDITZCC,cursector->zcc,3);
        Tcluster *c,*c0=findcluster(cursector);
        for (c=map->clusters,cluster=0;c!=c0;c=c->next,cluster++);
        eb_setivalue(IDC_EDITCLUSTER,cluster);
        updateviewall();
        return 1;
      }
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK:
          cursector->zfa=eb_getfvalue(IDC_EDITZFA);
          cursector->zfb=eb_getfvalue(IDC_EDITZFB);
          cursector->zfc=eb_getfvalue(IDC_EDITZFC);
          cursector->zca=eb_getfvalue(IDC_EDITZCA);
          cursector->zcb=eb_getfvalue(IDC_EDITZCB);
          cursector->zcc=eb_getfvalue(IDC_EDITZCC);
          cursector->changeheight();
          l=eb_getivalue(IDC_EDITCLUSTER);
          if (l!=cluster && l<map->clustersnum) {
            Tcluster *c;
            for (c=map->clusters;l;l--,c=c->next);
            changecluster(cursector,c);
          }
          selectsector(cursector);
          setdirty(true);
          enddialog(0);
          return 1;
        case IDCANCEL:
          enddialog(0);
          return 1;
      }
  }
  return 0;
}

//// New Sector
// dialog function for the new sector dialog
BOOL CALLBACK NewSectorProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_INITDIALOG:
      {
        int tabs[1]={10};
        lb_settabs(hwndDlg,IDC_CLUSTERS,1,tabs);
        updateclustersdlg(hwndDlg,0);
        eb_setfvalue(IDC_EDITFLOOR,defaultfloorz,3);
        eb_setfvalue(IDC_EDITCEILING,defaultceilingz,3);
        return 1;
      }
    case WM_COMMAND:
      if (LOWORD(wParam)==IDC_CLUSTERS && HIWORD(wParam)==LBN_DBLCLK) {
        int i=lb_getcurrent(hwndDlg,IDC_CLUSTERS);
        Tcluster *c=(Tcluster *)lb_getitemdata(hwndDlg,IDC_CLUSTERS,i);
        enddialog((int)c);
        return 1;
      }
      else if (HIWORD(wParam)==BN_CLICKED)
        switch (LOWORD(wParam)) {
          case IDCANCEL:
            enddialog(0);
            return 1;
          case IDOK:
            {
              int i=lb_getcurrent(hwndDlg,IDC_CLUSTERS);
              if (i<0 || i>=map->clustersnum) return 1;
              Tcluster *c=(Tcluster *)lb_getitemdata(hwndDlg,IDC_CLUSTERS,i);
              defaultfloorz=eb_getfvalue(IDC_EDITFLOOR);
              defaultceilingz=eb_getfvalue(IDC_EDITCEILING);
              enddialog((int)c);
              setdirty(true);
              return 1;
            }
          case IDC_NEWCLUSTER:
            newcluster();
            updateclustersdlg(hwndDlg,map->clustersnum-1);
            updateclustersdlg(hwndClusters,map->clustersnum-1);
            break;
        }
      break;
  }
  return 0;
}

// edits the properties of the new sector
// returns the parent cluster
Tcluster *newsector( void )
{
  lastc=NULL;
  InvalidateRect(hwndMain,&viewxy,0);
  UpdateWindow(hwndMain);
  return (Tcluster *)rundialog(IDD_NEWSECTOR,NewSectorProc);
}

//// Textures
static int curtexture=0;
static HBITMAP bmptxt=NULL;
// updates the textures dialog
static void updatetxtdlg( void )
{
  EnableWindow(GetDlgItem(currentDlg,IDC_ADDTXT),ntextures<MAX_TEXTURES);
  EnableWindow(GetDlgItem(currentDlg,IDC_REPLACETXT),ntextures>0);
  EnableWindow(GetDlgItem(currentDlg,IDC_DELTXT),ntextures>0);
  if (!ntextures) {
    eb_settext(IDC_TXTNAME,"");
    eb_settext(IDC_TXTSIZE,"");
    SendDlgItemMessage(currentDlg,IDC_TXTBITMAP,STM_SETIMAGE,IMAGE_BITMAP,0);
    return;
  }
  eb_settext(IDC_TXTNAME,textures[curtexture].name);
  char s[20];
  sprintf(s,"%dx%d",textures[curtexture].wd,textures[curtexture].he);
  eb_settext(IDC_TXTSIZE,s);
  if (bmptxt) DeleteObject(bmptxt);
  BITMAPINFO *pbmi=(BITMAPINFO *)malloc(sizeof(BITMAPINFO)+255*sizeof(RGBQUAD));
  for (int c=0;c<256;c++) {
    pbmi->bmiColors[c].rgbRed=palette[c][0];
    pbmi->bmiColors[c].rgbGreen=palette[c][1];
    pbmi->bmiColors[c].rgbBlue=palette[c][2];
  }
  pbmi->bmiHeader.biSize=sizeof(pbmi->bmiHeader);
  pbmi->bmiHeader.biWidth=textures[curtexture].wd;
  pbmi->bmiHeader.biHeight=-textures[curtexture].he;
  pbmi->bmiHeader.biPlanes=1;
  pbmi->bmiHeader.biBitCount=8;
  pbmi->bmiHeader.biCompression=BI_RGB;
  pbmi->bmiHeader.biSizeImage=0;
  pbmi->bmiHeader.biXPelsPerMeter=0;
  pbmi->bmiHeader.biYPelsPerMeter=0;
  pbmi->bmiHeader.biClrUsed=0;
  pbmi->bmiHeader.biClrImportant=0;
  void *data;
  bmptxt=CreateDIBSection(NULL,pbmi,DIB_RGB_COLORS,&data,NULL,0);
  memcpy(data,textures[curtexture].datap,textures[curtexture].wd*textures[curtexture].he);
  SendDlgItemMessage(currentDlg,IDC_TXTBITMAP,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bmptxt);
  free(pbmi);
}

// sets the system palette
static void setpalette(byte *pal)
{
  for (int i=0;i<256;i++,pal+=4) {
    palette[i][0]=pal[2];
    palette[i][1]=pal[1];
    palette[i][2]=pal[0];
  }
  setdirty(true);
  nopalettefl=false;
  calculatesize();
}

// reads palette from a file
static bool readpalette(char *fn)
{
  byte *pal;
  if (!read_pal(fn,&pal)) return false;
  setpalette(pal);
  free(pal);
  return true;
}

// replaces one texture with another
static bool replacetexture(int txt,char *bmp)
{
  byte *data;
  byte *pal;
  int w,h;
  if (!read_bmp(bmp,&data,&pal,&w,&h)) return false;
  setdirty(true);
  if (nopalettefl) setpalette(pal);
  free(pal);
  bool res=changetexture(txt,w,h,bmp+filenameoffset,data);
  free(data);
  return res;
}

// adds a new texture
static bool addtexture(char *bmp)
{
  byte *data;
  byte *pal;
  int w,h;
  if (!read_bmp(bmp,&data,&pal,&w,&h)) return false;
  setdirty(true);
  if (nopalettefl) setpalette(pal);
  free(pal);
  bool res=newtexture(w,h,bmp+filenameoffset,data);
  free(data);
  return res;
}

// saves a texture to bitmap
static void savetexture(int txt,char *bmp)
{
  int w=textures[txt].wd;
  int h=textures[txt].he;
  int w1=(w+3)&~3;
  int size=w1*h;
  byte *data=(byte *)malloc(size);
  if (!data) return;

  int offs=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD);
  byte *p1=data+w1*(h-1);
  byte *p2=textures[txt].datap;
  for (int y=0;y<h;y++,p1-=w1,p2+=w)
    memcpy(p1,p2,w);

  BITMAPFILEHEADER bfh={'MB',size+offs,0,0,offs};
  BITMAPINFOHEADER bih={sizeof(bih),w,h,1,8,0,size,0,0,256,256};

  RGBQUAD pal[256];
  for (int i=0;i<256;i++) {
    pal[i].rgbRed=palette[i][0];
    pal[i].rgbGreen=palette[i][1];
    pal[i].rgbBlue=palette[i][2];
    pal[i].rgbReserved=0;
  }

  FILE *f=fopen(bmp,"wb");
  if (f) {
    fwrite(&bfh,sizeof(bfh),1,f);
    fwrite(&bih,sizeof(bih),1,f);
    fwrite(&pal,sizeof(pal),1,f);
    fwrite(data,size,1,f);
    fclose(f);
  }
  else MessageBox(hwndMain,"Error writing bitmap file","BOOM Edit",MB_OK);

  free(data);
}

// dialog function for the textures dialog
BOOL CALLBACK TexturesProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  char bmp[_MAX_PATH]="";
  switch (uMsg) {
    case WM_INITDIALOG:
      if (curtexture>=ntextures) curtexture=0;
      updatetxtdlg();
      {
        sb_setrange(IDC_TXTSCROLL,0,ntextures-1);
        sb_setpage(IDC_TXTSCROLL,1);
        sb_setpos(IDC_TXTSCROLL,curtexture);
      }
      return 1;
    case WM_VSCROLL:
      curtexture=sb_scroll(IDC_TXTSCROLL,wParam,lParam);
      updatetxtdlg();
      break;
    case WM_COMMAND:
      if (HIWORD(wParam)==BN_CLICKED)
        switch (LOWORD(wParam)) {
          case IDC_REPLACETXT:
            if (openfilename(bmp,"Replace Texture","Bitmap files\0*.bmp","bmp")) {
              if (replacetexture(curtexture,bmp)) updatetxtdlg();
              else MessageBox(hwndMain,"Fail to replace the texture","BOOM Editor",MB_OK);
            }
            SetFocus(hwndDlg);
            break;
          case IDC_ADDTXT:
            if (openfilename(bmp,"Add Texture","Bitmap files\0*.bmp","bmp")) {
              if (addtexture(bmp)) {
                curtexture=ntextures-1;
                updatetxtdlg();
                sb_setrange(IDC_TXTSCROLL,0,ntextures-1);
                sb_setpos(IDC_TXTSCROLL,curtexture);
              }
              else MessageBox(hwndMain,"Fail to add the texture","BOOM Editor",MB_OK);
            }
            SetFocus(hwndDlg);
            break;
          case IDC_DELTXT:
            if (!textureused(curtexture) || MessageBox(hwndMain,"The texture is in use. Are you sure you want to delete it?","BOOM Editor",MB_YESNO)==IDYES) {
              setdirty(true);
              deltexture(curtexture);
              sb_setrange(IDC_TXTSCROLL,0,ntextures-1);
              if (curtexture>=ntextures) {
                curtexture--;
              }
              updatetxtdlg();
              sb_setpos(IDC_TXTSCROLL,curtexture);
            }
            SetFocus(hwndDlg);
            break;
          case IDC_SAVETXT:
            strcpy(bmp,textures[curtexture].name);
            if (savefilename(bmp,"Save Texture to Bitmap","Bitmap files\0*.bmp","bmp")) {
              savetexture(curtexture,bmp);
            }
            SetFocus(hwndDlg);
            break;
          case IDCANCEL:
            enddialog(0);
            return 1;
          case IDOK:
            enddialog(1);
            return 1;
        }
      break;
  }
  return 0;
}

//// Texture Mapping
static Twall *getwall(int k)
{
  Twall *w;
  int i;
  for (i=0,w=*curline->walls;w;w=w->next)
    if (is_solid(w)) {
      if (i==k) return w;
      i++;
    }
  return NULL;
}

// updates the texture window
static void updatemappingdlg(int p)
{
  if (hwndTextures) {
    if (p==-1) {
      lb_reset(hwndTextures,IDC_POLYGONS);
      bool state=false;
      if (cursector) {
        lb_addstring(hwndTextures,IDC_POLYGONS,"floor");
        lb_addstring(hwndTextures,IDC_POLYGONS,"ceiling");
        lb_setcurrent(hwndTextures,IDC_POLYGONS,0);
        p=0;
        state=true;
      }
      else if (curline) {
        Twall *w;
        int i;
        for (i=0,w=*curline->walls;w;w=w->next) {
          if (!is_solid(w)) continue;
          char s[20];
          sprintf(s,"wall %d",i+1);
          lb_addstring(hwndTextures,IDC_POLYGONS,s);
          i++;
        }
        if (i) state=true;
        p=0;
        lb_setcurrent(hwndTextures,IDC_POLYGONS,0);
      }
      enablecontrol(hwndTextures,IDC_POLYGONS,state);
      if (!state) eb_settext(hwndTextures,IDC_MAP,"");
    }
    if (cursector) {
      if (p==0) eb_settext(hwndTextures,IDC_MAP,textures[cursector->tfloor].name);
      else if (p==1) eb_settext(hwndTextures,IDC_MAP,textures[cursector->tceiling].name);
    }
    else if (curline) {
      Twall *w=getwall(p);
      if (w) eb_settext(hwndTextures,IDC_MAP,textures[w->texture].name);
      else eb_settext(hwndTextures,IDC_MAP,"");
    }
  }
}

// dialog function for the textures window
BOOL CALLBACK TextureMappingProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_INITDIALOG:
      hwndTextures=hwndDlg;
      updatemappingdlg(-1);
      return 1;
    case WM_COMMAND:
      if (LOWORD(wParam)==IDC_POLYGONS && HIWORD(wParam)==LBN_DBLCLK) {
        int p=lb_getcurrent(hwndTextures,IDC_POLYGONS);
        int *texture=NULL;
        if (cursector) {
          if (p==0) texture=&(cursector->tfloor);
          else if (p==1) texture=&(cursector->tceiling);
        }
        else if (curline) {
          Twall *w=getwall(p);
          if (w && is_solid(w)) texture=&(w->texture);
        }
        if (!texture) break;
        curtexture=*texture;
        if (rundialog(IDD_CHANGETEXTURE,TexturesProc)) {
          setdirty(true);
          *texture=curtexture;
        }
        updatemappingdlg(p);
      }
      if (LOWORD(wParam)==IDC_POLYGONS && HIWORD(wParam)==LBN_SELCHANGE)
        updatemappingdlg(lb_getcurrent(hwndTextures,IDC_POLYGONS));
      break;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// new contour editing

// deletes the currently edited contour
static void delcontour( void )
{
  Tnode *n;
  for (;contour;contour=n) {
    n=contour->next;
    free(contour);
  }
}

// deletes a node from the contour
static void delnode(bool last)
{
  if (contour && contour->next) {
    Tnode *c=contour;
    for (;c->next->next;c=c->next);
    if (!last) {
      c->x=c->next->x;
      c->y=c->next->y;
    }
    free(c->next);
    c->next=NULL;
    lastc=c;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
// File operaions

// current filename
static char filename[_MAX_PATH];
// "dirty" flag (the map is changed)
static bool dirtyfl=false;

// sets the title of the main window
static void settitlename( void )
{
  char s[256]="BOOM Editor - [";
  if (filename[0]) strcat(s,filename);
  else strcat(s,"untitled");
  if (dirtyfl) strcat(s," *");
  strcat(s,"]");
  settitle(s);
}

// saves the map in file
static void savemap( void )
{
  compactverts();
  write_start(filename);
  map->save();
  savetextures();
  write_end();
  setdirty(false);
}

// saves the map in a different file
static bool filesaveas( void )
{
  if (savefilename(filename,"Save Map","BOOM Map Files\0*.boo\0","boo")) {
    savemap();
    settitlename();
    return true;
  }
  return false;
}

// saves the map in the current file
static bool filesave( void )
{
  if (!map) return true;
  if (*filename) {
    savemap();
    return true;
  }
  else return filesaveas();
}

// saves the map on exit
static bool confirmsave( void )
{
  int m=MessageBox(hwndMain,"Save changes to the current map?","BOOM Editor",MB_YESNOCANCEL|MB_ICONEXCLAMATION);
  if (m==IDCANCEL) return false;
  if (m==IDNO) return true;
  return filesave();
}

// creates an empty map
static void filenew( void )
{
  if (dirtyfl && !confirmsave()) return;
  *filename=0;
  setdirty(false);
  settitlename();
  map_init(NULL);
  cursector=NULL;
  curline=NULL;
  deselectvertices();
  newcluster();
  updateclustersdlg(hwndClusters,0);
  updatesectordlg();
  calculatesize();
  freetextures();
  defaulttexture();
  defwalltxt=0;
  deffloortxt=0;
  defceiltxt=0;
  nopalettefl=true;
}

// opens a map
static void fileopen( void )
{
  if (dirtyfl && !confirmsave()) return;
  if (openfilename(filename,"Open Map","BOOM Map Files\0*.boo\0","boo")) {
    if (map_init(filename)) {
      setdirty(false);
      cursector=NULL;
      curline=NULL;
      deselectvertices();
      settitlename();
      init_clusters();
      initverts();
      nopalettefl=false;
      calculatesize();
    }
    else {
      MessageBox(hwndMain,"Error loading map file","Boom Edit",MB_OK);
      filenew();
    }
  }
}

// set the "dirty" flag of the map
void setdirty(bool state)
{
  if (dirtyfl!=state) {
    dirtyfl=state;
    settitlename();
  }
  else dirtyfl=state;
}

///////////////////////////////////////////////////////////////////////////////////////
// Windows messages

// WM_COMMAND handling
bool oncommand(WPARAM wParam,LPARAM lParam)
{
  switch (LOWORD(wParam)) {
    // file commands
    case ID_FILENEW:
      filenew();
      break;
    case ID_FILEOPEN:
      fileopen();
      break;
    case ID_FILESAVE:
      filesave();
      break;
    case ID_FILESAVEAS:
      filesaveas();
      break;
    case ID_FILEEXIT:
      if (!dirtyfl || confirmsave()) DestroyWindow(hwndMain);
      break;

    // zooms
    case ID_ZOOMIN:
      {
        if (scale<1) scale*=DSCALE;
        updateview0();
        break;
      }
    case ID_ZOOMOUT:
      {
        if (scale>0.001) scale/=DSCALE;
        updateview0();
        break;
      }
    case ID_ZOOMGRIDIN:
      if (gridsize>4) gridsize/=2;
      updateview2d();
      break;
    case ID_ZOOMGRIDOUT:
      if (gridsize<1024) gridsize*=2;
      updateview2d();
      break;

    // toggle different user interface components
    case ID_TOGGLEGRID:
      gridfl=!gridfl;
      CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEGRID,gridfl?MF_CHECKED:MF_UNCHECKED);
      checkbutton(ID_TOGGLEGRID,gridfl);
      updateview2d();
      break;
    case ID_VIEWSTATUSBAR:
      statusbarfl=!statusbarfl;
      showstatusbar(statusbarfl);
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWSTATUSBAR,statusbarfl?MF_CHECKED:MF_UNCHECKED);
      break;
    case ID_FILETOOLBAR:
      filetoolbarfl=!filetoolbarfl;
      showtoolbar(filetoolbarfl,0);
      CheckMenuItem(GetMenu(hwndMain),ID_FILETOOLBAR,filetoolbarfl?MF_CHECKED:MF_UNCHECKED);
      break;
    case ID_VIEWTOOLBAR:
      viewtoolbarfl=!viewtoolbarfl;
      showtoolbar(viewtoolbarfl,1);
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWTOOLBAR,viewtoolbarfl?MF_CHECKED:MF_UNCHECKED);
      break;
    case ID_EDITTOOLBAR:
      edittoolbarfl=!edittoolbarfl;
      showtoolbar(edittoolbarfl,2);
      CheckMenuItem(GetMenu(hwndMain),ID_EDITTOOLBAR,edittoolbarfl?MF_CHECKED:MF_UNCHECKED);
      break;
    case ID_TOGGLEXY:
      if (!viewxyfl || viewxzfl || viewyzfl || view3dfl) {
        viewxyfl=!viewxyfl;
        CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEXY,viewxyfl?MF_CHECKED:MF_UNCHECKED);
        updatecommands();
        calculatesize();
      }
      break;
    case ID_TOGGLEXZ:
      if (viewxyfl || !viewxzfl || viewyzfl || view3dfl) {
        viewxzfl=!viewxzfl;
        CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEXZ,viewxzfl?MF_CHECKED:MF_UNCHECKED);
        updatecommands();
        calculatesize();
      }
      break;
    case ID_TOGGLEYZ:
      if (viewxyfl || viewxzfl || !viewyzfl || view3dfl) {
        viewyzfl=!viewyzfl;
        CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEYZ,viewyzfl?MF_CHECKED:MF_UNCHECKED);
        updatecommands();
        calculatesize();
      }
      break;
    case ID_TOGGLE3D:
      if (viewxyfl || viewxzfl || viewyzfl || !view3dfl) {
        view3dfl=!view3dfl;
        CheckMenuItem(GetMenu(hwndMain),ID_TOGGLE3D,view3dfl?MF_CHECKED:MF_UNCHECKED);
        updatecommands();
        calculatesize();
      }
      break;
    case ID_VIEWSECTOR:
      if (hwndSector) {
        SetFocus(hwndMain);
        DestroyWindow(hwndSector);
        hwndSector=NULL;
      }
      else {
        hwndSector=CreateDialog(hinst,MAKEINTRESOURCE(IDD_SECTORINFO),hwndMain,(DLGPROC)SectorInfoProc);
        updatesectordlg();
        SetFocus(hwndMain);
      }
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWSECTOR,hwndSector?MF_CHECKED:MF_UNCHECKED);
      checkbutton(ID_VIEWSECTOR,hwndSector!=0);
      calculatesize();
      break;
    case ID_VIEWCLUSTERS:
      if (hwndClusters) {
        SetFocus(hwndMain);
        DestroyWindow(hwndClusters);
        hwndClusters=NULL;
      }
      else {
        hwndClusters=CreateDialog(hinst,MAKEINTRESOURCE(IDD_CLUSTERS),hwndMain,(DLGPROC)ClustersProc);
      }
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWCLUSTERS,hwndClusters?MF_CHECKED:MF_UNCHECKED);
      checkbutton(ID_VIEWCLUSTERS,hwndClusters!=0);
      calculatesize();
      break;
    case ID_VIEWTEXTURES:
      if (hwndTextures) {
        SetFocus(hwndMain);
        DestroyWindow(hwndTextures);
        hwndTextures=NULL;
      }
      else {
        CreateDialog(hinst,MAKEINTRESOURCE(IDD_MAPPING),hwndMain,(DLGPROC)TextureMappingProc);
        SetFocus(hwndMain);
      }
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWTEXTURES,hwndTextures?MF_CHECKED:MF_UNCHECKED);
      checkbutton(ID_VIEWTEXTURES,hwndTextures!=0);
      calculatesize();
      break;
    case ID_VIEWSTARTPOINT:
      startpfl=!startpfl;
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWSTARTPOINT,startpfl?MF_CHECKED:MF_UNCHECKED);
      updateview2d();
      break;
    case ID_VIEWCURRENTPOINT:
      currentpfl=!currentpfl;
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWCURRENTPOINT,currentpfl?MF_CHECKED:MF_UNCHECKED);
      updateview2d();
      break;

    // starts a new contour
    case ID_NEWCONTOUR:
      if (setmode(mCONTOUR)) {
        delcontour();
        contour=(Tnode *)malloc(sizeof(Tnode));
        contour->next=NULL;
        contour->x=0;
        contour->y=0;
        lastc=contour;
        setmessage("Enter the points of the new contour. <Enter> to finish, <Esc> to cancel");
      }
      break;

    // starts a new vertex input
    case ID_NEWVERTEX:
      setmode(mVERTEX);
      setmessage("Point the line to be split, <Esc> to cancel");
      break;

    // starts a sector joining
    case ID_JOINSECTORS:
      setmode(mJOIN);
      joinsc=cursector;
      if (cursector) setmessage("Select the second sector, <Esc> to cancel");
      else  setmessage("Select the first sector, <Esc> to cancel");
      break;

    // deletes the current sector
    case ID_DELETESECTOR:
      if (cursector) {
        delsector(map,findcluster(cursector),cursector);
        setdirty(true);
        cursector=NULL;
        updateviewall();
        updatesectordlg();
        setmessage("Sector deleted",true);
      }
      break;

    // starts a sector insertion
    case ID_INSERTSECTOR:
      setmode(mSELCONTOUR);
      setmessage("Select the contour to convert to sector, <Esc> to cancel");
      break;

    // edits the current sector properties
    case ID_SECTOREDIT:
      if (cursector) {
        rundialog(IDD_SECTOR,SectorProc);
        updatesectordlg();
        updateviewall();
      }
      break;

    // edits the textures
    case ID_TEXTURES:
      rundialog(IDD_TEXTURES,TexturesProc);
      break;

    // edits the current palette
    case ID_PALETTE:
      {
        char name[_MAX_PATH]="";
        if (openfilename(name,"Get palette from","Bitmap files\0*.bmp","bmp"))
          if (!readpalette(name)) {
            MessageBox(hwndMain,"Error loading palette","Boom Edit",MB_OK);
          }
      }
      break;

    // sets the default wall texture
    case ID_SETDEFWALL:
      curtexture=defwalltxt;
      if (rundialog(IDD_CHANGETEXTURE,TexturesProc)) defwalltxt=curtexture;
      break;

    // sets the default floor texture
    case ID_SETDEFFLOOR:
      curtexture=deffloortxt;
      if (rundialog(IDD_CHANGETEXTURE,TexturesProc)) deffloortxt=curtexture;
      break;

    // sets the default ceiling texture
    case ID_SETDEFCEILING:
      curtexture=defceiltxt;
      if (rundialog(IDD_CHANGETEXTURE,TexturesProc)) defceiltxt=curtexture;
      break;

    // creates a new cluster
    case ID_NEWCLUSTER:
      newcluster();
      setdirty(true);
      updateclustersdlg(hwndClusters,0);
      break;

    // starts editing the curent position
    case ID_EDITCURRENTPOINT:
      setmode(mSELECTCP);
      currentpfl=true;
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWCURRENTPOINT,currentpfl?MF_CHECKED:MF_UNCHECKED);
      updateview2d();
      setmessage("Enter the new current position, <Esc> to cancel");
      break;

    // starts editing the start position
    case ID_EDITSTARTPOINT:
      startpfl=true;
      CheckMenuItem(GetMenu(hwndMain),ID_VIEWSTARTPOINT,startpfl?MF_CHECKED:MF_UNCHECKED);
      updateview2d();
      setmode(mSELECTSP);
      setmessage("Enter the new start position, <Esc> to cancel");
      break;
    // centers the 2D view according the current position
    case ID_GOTOCURRENTPOSITION:
      editx0=view.x;
      edity0=view.y;
      editz0=view.z;
      updateview0();
      break;

    // shows the about dialog box
    case ID_ABOUT:
      rundialog(IDD_ABOUT,AboutProc);
      break;
    default: return false;
  }
  return true;
}

// Windows message handling
long windowproc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  static coord3d oldx0,oldy0,oldz0;

  switch (uMsg) {
    case WM_CHAR:
      // keyboard input handling
      switch (wParam) {
        case 8: // backspace
          if (mode==mCONTOUR) {
            delnode(false);
            updateview2d();
          }
          break;
        case 13: // enter
          if (mode==mCONTOUR) {
            delnode(true);
            addcontour(contour);
            delcontour();
            updatesectordlg();
            updateview2d();
            setdirty(true);
            setmode(mSELECT);
          }
          break;
        case 27: // escape
          switch (mode) {
            case mCONTOUR:
            case mJOIN:
            case mSELCONTOUR:
            case mVERTEX:
            case mSELECTSP:
            case mSELECTCP:
              updateview2d();
              setmode(mSELECT);
              break;
          }
          break;
      }
      break;

    // draws the main window
    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        HDC hdc=BeginPaint(hwnd,&ps);
        if (scale>0 && map) {
          drawframe(hdc);
          if (viewxyfl) drawxy(hdc);
          if (viewxzfl) drawxz(hdc);
          if (viewyzfl) drawyz(hdc);
          if (view3dfl) draw3dview(hdc,view3d.left,view3d.top);
        }
        if (dialogy) {
          RECT r;
          GetClientRect(hwndMain,&r);
          r.left=dialogx;
          r.top=dialogy-1;
          FillRect(hdc,&r,(HBRUSH)GetStockObject(GRAY_BRUSH));
        }
        EndPaint(hwnd,&ps);
        break;
      }

    // left mouse button click
    case WM_LBUTTONDOWN:
      SetFocus(hwndMain);
      if (dragview2) break;
      dragposx=LOWORD(lParam);
      dragposy=HIWORD(lParam);
      updateviewall();
      // if the click is in XY view
      if (inrect(dragposx,dragposy,&viewxy)) {
        dragview1=drXY;
        switch (mode) {
          // creates a new vertex
          case mVERTEX:
            {
              Tvertex *v=insertvertex(xyscr2mapx(dragposx),xyscr2mapy(dragposy));
              setmode(mSELECT);
              if (v) {
                setdirty(true);
                deselectvertices();
                selectvertex(v);
                cursector=NULL;
                curline=NULL;
                updatesectordlg();
                setmessage("Vertex inserted",true);
              }
              msdrag=drVERTEX;
              break;
            }

          // adds a vertex to the contour
          case mCONTOUR:
            lastc->next=(Tnode *)malloc(sizeof(Tnode));
            lastc->x=snaptogrid(xyscr2mapx(dragposx));
            lastc->y=snaptogrid(xyscr2mapy(dragposy));
            lastc->next->x=lastc->x;
            lastc->next->y=lastc->y;
            lastc=lastc->next;
            lastc->next=NULL;
            break;

          // selects the sectors to be joined
          case mJOIN:
            if (selectsector(dragposx,dragposy)) {
              if (!joinsc) {
                joinsc=cursector;
                setmessage("Select the second sector, <Esc> to cancel");
              }
              else if (cursector!=joinsc) {
                setdirty(true);
                setmode(mSELECT);
                if (joinsectors(cursector,joinsc))
                  setmessage("Sectors joined",true);
                else setmessage("Sectors belong to different clusters!",true);
                updatesectordlg();
              }
            }
            break;

          // selects a contour
          case mSELCONTOUR:
            setmode(mSELECT);
            if (selcontour(xyscr2mapx(dragposx),xyscr2mapy(dragposy))) {
              updatesectordlg();
            }
            else setmessage(NULL);
            break;

          // select (default mode)
          case mSELECT:
            if (wParam&MK_SHIFT) {
              msdrag=drBOX;
              selbox.left=dragposx-viewxy.left;
              selbox.top=dragposy-viewxy.top;
              selbox.right=dragposx-viewxy.left;
              selbox.bottom=dragposy-viewxy.top;
            }
            else {
              int x=dragposx;
              int y=dragposy;
              if (wParam&MK_CONTROL) {
                selectvertex(x,y);
                updatesectordlg();
              }
              else if (vertexselected(x,y)) {
                msdrag=drVERTEX;
              }
              else {
                deselectvertices();
                if (!selectvertex(x,y)) {
                  if (!selectline(x,y)) {
                    selectsector(x,y);
                  }
                }
                updatesectordlg();
              }
            }
            break;

          // enters the start position
          case mSELECTSP:
            {
              map->start_x=snaptogrid(xyscr2mapx(dragposx));
              map->start_y=snaptogrid(xyscr2mapy(dragposy));
              map->start_z=0;
              setdirty(true);
              Tsector *s=findsector(dragposx,dragposy);
              if (s) map->start_z=s->getzf(map->start_x,map->start_y)+100;
              view.x=map->start_x;
              view.y=map->start_y;
              view.z=map->start_z;
              setmode(mSELECT);
              break;
            }

          // enters the current position
          case mSELECTCP:
            {
              view.x=snaptogrid(xyscr2mapx(dragposx));
              view.y=snaptogrid(xyscr2mapy(dragposy));
              view.z=0;
              Tsector *s=findsector(dragposx,dragposy);
              if (s) view.z=s->getzf(view.x,view.y)+100;
              setmode(mSELECT);
              break;
            }
        }
      }
      // if the click is in XZ view or YZ view tries to drag the floor or ceiling
      else if (inrect(dragposx,dragposy,&viewxz)) {
        dragview1=drXZ;
        if (cursector) {
          if (selectfloorxz(dragposx,dragposy)) msdrag=drFLOOR;
          else if (selectceilingxz(dragposx,dragposy)) msdrag=drCEILING;
        }
      }
      else if (inrect(dragposx,dragposy,&viewyz)) {
        dragview1=drYZ;
        if (cursector) {
          if (selectflooryz(dragposx,dragposy)) msdrag=drFLOOR;
          else if (selectceilingyz(dragposx,dragposy)) msdrag=drCEILING;
        }
      }
      updatecoord(lParam);
      break;

    // left mouse button released (after drag)
    case WM_LBUTTONUP:
      updateviewall();
      switch (msdrag) {
        case drVERTEX:
          updateverts();
          break;
        case drFLOOR:
        case drCEILING:
          updateflcl();
          updatesectordlg();
          break;
        case drBOX:
          if (wParam&MK_CONTROL) {
            selectvertexbox(&selbox);
          }
          else {
            deselectvertices();
            selectvertexbox(&selbox);
          }
          break;
      }
      msdrag=0;
      dragview1=0;
      updatecoord(lParam);
      break;

    // right mouse button pressed (begins drag)
    case WM_RBUTTONDOWN:
      SetFocus(hwndMain);
      if (msdrag) break;
      dragposx=LOWORD(lParam);
      dragposy=HIWORD(lParam);
      oldx0=editx0;
      oldy0=edity0;
      oldz0=editz0;
      if (inrect(dragposx,dragposy,&viewxy)) dragview2=drXY;
      else if (inrect(dragposx,dragposy,&viewxz)) dragview2=drXZ;
      else if (inrect(dragposx,dragposy,&viewyz)) dragview2=drYZ;
      else dragview2=0;
      updatecursor();
      updatecoord(lParam);
      break;

    // right mouse button released (ends drag)
    case WM_RBUTTONUP:
      dragview2=0;
      updatecoord(lParam);
      updatecursor();
      break;

    // mouse pointer moved
    case WM_MOUSEMOVE:
      {
        updatecursor();
        int x=LOWORD(lParam);
        int y=HIWORD(lParam);
        switch (mode) {
          // moves the last vertex of the contour (rubber line)
          case mCONTOUR:
            lastc->x=snaptogrid(xyscr2mapx(x));
            lastc->y=snaptogrid(xyscr2mapy(y));
            updateviewxy();
            break;

          // select mode
          case mSELECT:
            if (msdrag && !(wParam&MK_LBUTTON)) {
              PostMessage(hwnd,WM_LBUTTONUP,wParam,lParam);
              break;
            }
            if (dragview2 && !(wParam&MK_RBUTTON)) {
              PostMessage(hwnd,WM_RBUTTONUP,wParam,lParam);
              break;
            }
            coord3d dx=(x-dragposx)/scale;
            coord3d dy=(dragposy-y)/scale;
            // if left button is pressed, drags the selected object
            switch (msdrag) {
              case drVERTEX:
                dragvertex(dx,dy);
                setdirty(true);
                updateviewall();
                break;
              case drBOX:
                selbox.right=x-viewxy.left;
                selbox.bottom=y-viewxy.top;
                updateviewxy();
                break;
              case drFLOOR:
                dragfloor(dy);
                setdirty(true);
                updatesectordlg();
                updateviewall();
                break;
              case drCEILING:
                dragceiling(dy);
                setdirty(true);
                updatesectordlg();
                updateviewall();
                break;
            }
            break;
        }
        // if right button is pressed, drags the view
        if (dragview2) {
          coord3d dx=(x-dragposx)/scale;
          coord3d dy=(dragposy-y)/scale;
          switch (dragview2) {
            case drXY:
              editx0=oldx0-dx;
              edity0=oldy0-dy;
              break;
            case drXZ:
              editx0=oldx0-dx;
              editz0=oldz0-dy;
              break;
            case drYZ:
              edity0=oldy0-dx;
              editz0=oldz0-dy;
              break;
          }
          updateview0();
        }
        updatecoord(lParam);
        break;
      }

    // closes the application
    case WM_CLOSE:
      if (!dirtyfl || confirmsave()) DestroyWindow(hwndMain);
      break;

    // resizes the main window
    case WM_SIZE:
      calculatesize();
      break;
    default: return DefWindowProc(hwnd,uMsg,wParam,lParam);
  }
  return 0;
} 
 
// initialization
static void reset( void )
{
  editx0=0;edity0=0;editz0=0;
  scale=0.1;
  gridsize=128;
  gridfl=true;
  defaultfloorz=-128;
  defaultceilingz=128;
  contour=NULL;
  lastc=NULL;
  viewxyfl=true;
  viewxzfl=true;
  viewyzfl=true;
  view3dfl=true;
  mode=mSELECT;
  filenew();
}

// keys for 3D view movement
static int keys[]={VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_HOME,VK_PRIOR,
'X',VK_SPACE,'A','Z','O','P',VK_CLEAR,VK_SHIFT,VK_ESCAPE};
#define KeyPressed(x) ((GetKeyState(x) & 0x8000)!=0)

// idle function (called when all messages are processed)
static void app_idle( void )
{
  int time=timeGetTime();
  if (messagetimeout && messagetimeout<time) setmessage(NULL);
  if (view3dfl) {
    coord3d x0=view.x;
    coord3d y0=view.y;
    if (GetFocus()==hwndMain) {
      // reads the keyboard and sets player_keys variable
      player_keys=0;
      for (int i=0,m=1;i<sizeof(keys)/sizeof(keys[0]);i++,m<<=1)
        if (KeyPressed(keys[i])) player_keys|=m;
    }
    player_idle(time);
    memset(hbmpbuffer,0,(view3d.right-view3d.left+1)*(view3d.bottom-view3d.top+1));
    if (!map_draw()) {
      memset(hbmpbuffer,0,(view3d.right-view3d.left+1)*(view3d.bottom-view3d.top+1));
      Tsector *s=findsectorf(view.x,view.y);
      if (s) view.z=s->getzf(view.x,view.y)+100;
    }
    updateview3d();
    if (currentpfl && (x0!=view.x || y0!=view.y)) updateviewxy();
  }
  UpdateWindow(hwndMain);
}

static Ttoolbar filebuttons[]={
  {TBSTYLE_BUTTON,ID_FILENEW,"New (Ctrl+N)"},
  {TBSTYLE_BUTTON,ID_FILEOPEN,"Open (Ctrl+O)"},
  {TBSTYLE_BUTTON,ID_FILESAVE,"Save (Ctrl+S)"},
};

static Ttoolbar viewbuttons[]={
  {TBSTYLE_BUTTON,ID_ZOOMIN,"Zoom in (+)"},
  {TBSTYLE_BUTTON,ID_ZOOMOUT,"Zoom out (-)"},
  {TBSTYLE_BUTTON,ID_ZOOMGRIDIN,"Zoom grid in (Alt++)"},
  {TBSTYLE_BUTTON,ID_ZOOMGRIDOUT,"Zoom grid out (Alt+-)"},
  {TBSTYLE_CHECKED,ID_TOGGLEGRID,"Toggle grid (G)"},
  {TBSTYLE_CHECKED,ID_TOGGLEXY,"Toggle XY view (Alt+1)"},
  {TBSTYLE_CHECKED,ID_TOGGLEXZ,"Toggle XZ view (Alt+2)"},
  {TBSTYLE_CHECKED,ID_TOGGLEYZ,"Toggle YZ view (Alt+3)"},
  {TBSTYLE_CHECKED,ID_TOGGLE3D,"Toggle 3D view (Alt+4)"},
  {TBSTYLE_CHECK,ID_VIEWSECTOR,"Toggle Sector window"},
  {TBSTYLE_CHECK,ID_VIEWCLUSTERS,"Toggle Clusters window"},
  {TBSTYLE_CHECK,ID_VIEWTEXTURES,"Toggle Textures window"},
};

static Ttoolbar editbuttons[]={
  {TBSTYLE_BUTTON,ID_NEWCONTOUR,"New Sector (Ins)"},
  {TBSTYLE_BUTTON,ID_DELETESECTOR,"Delete Sector (Ctrl+D)"},
  {TBSTYLE_BUTTON,ID_INSERTSECTOR,"Insert Sector (Ctrl+I)"},
  {TBSTYLE_BUTTON,ID_JOINSECTORS,"Join Sectors (Ctrl+J)"},
  {TBSTYLE_BUTTON,ID_SECTOREDIT,"Edit Sector (Ctrl+E)"},
  {TBSTYLE_BUTTON,ID_NEWVERTEX,"New Vertex (V)"},
  {TBSTYLE_BUTTON,ID_NEWCLUSTER,"New Cluster"},
  {TBSTYLE_DROPDOWN,ID_TEXTURES,"Edit Textures (Ctrl+T)"},
};

// application entry point
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int nCmdShow)
{
  if (!createmainwindow("BOOM Editor",hInstance,IDR_BOOMEDITMENU,IDI_BOOMICON,0,-1)) return FALSE;
  int widths[]={60,120,-1};
  createstatusbar(3,widths);
  createrebar();
  createtoolbar(sizeof(filebuttons)/sizeof(Ttoolbar),filebuttons,IDR_FILETOOLBAR,true,true);
  createtoolbar(sizeof(viewbuttons)/sizeof(Ttoolbar),viewbuttons,IDR_VIEWTOOLBAR,true,true);
  HMENU toolmenu=LoadMenu(hinst,MAKEINTRESOURCE(IDR_TOOLMENU));
  editbuttons[7].menu=GetSubMenu(toolmenu,0);
  createtoolbar(sizeof(editbuttons)/sizeof(Ttoolbar),editbuttons,IDR_EDITTOOLBAR,true,true);
  CheckMenuItem(GetMenu(hwndMain),ID_VIEWSTATUSBAR,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_FILETOOLBAR,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_VIEWTOOLBAR,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_EDITTOOLBAR,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEXY,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEXZ,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEYZ,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_TOGGLE3D,MF_CHECKED);
  CheckMenuItem(GetMenu(hwndMain),ID_TOGGLEGRID,MF_CHECKED);
  calculatesize();
  draw2d_init();
  map_init(NULL);
  init_clusters();
  initverts();
  reset();
  InvalidateRect(hwndMain,NULL,0);
  mainwindowloop(IDR_BOOMEDITACCEL,app_idle);
  if (hwndSector) DestroyWindow(hwndSector);
  if (hwndClusters) DestroyWindow(hwndClusters);
  drawdone();
  map_done();
  freetextures();
  return 0;
}
