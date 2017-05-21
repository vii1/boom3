//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   boomd3d.cpp - main file for Direct 3D version                      //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "boom.h"
#include "win32.h"
#include "dialog.h"
#include "system.h"
#include "display.h"
#include "mouse.h"
#include "player.h"
#include "geom.h"
#include "resource.h"
#include <stdio.h>

static bool activefl;
static idle_handler *prev_idle_handler;

////////////////////////////////////////////////////////////////////////////

bool oncommand(WPARAM wParam,LPARAM lParam)
{
  return false;
}

//static int mousex,mousey;

long windowproc( HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_ACTIVATEAPP:
      activefl=wParam!=0;
      break;
  }
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

#define SRS(a,b) halt( D3D_OK!=d3d_device->SetRenderState(a,b), "d3d_device->SetRenderState() failed" )
#define STSS(a,b) halt( D3D_OK!=d3d_device->SetTextureStageState(0,a,b), "d3d_device->SetTextureStageState() failed" )

// sets render state for Direct 3D
static void set_render_state( void )
{
  SRS( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
  SRS( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE );
  SRS( D3DRENDERSTATE_ZENABLE, FALSE );
  SRS( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
  STSS( D3DTSS_MAGFILTER, D3DTFG_LINEAR );
  STSS( D3DTSS_MINFILTER, D3DTFG_LINEAR );
  STSS( D3DTSS_MIPFILTER, D3DTFP_LINEAR );

  STSS(D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
  STSS(D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
}
#undef STSS
#undef SRS

// keys for player movement
static int keys[]={VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_HOME,VK_PRIOR,
'X',VK_SPACE,'A','Z','O','P',VK_CLEAR,VK_SHIFT,VK_ESCAPE};
#define KeyPressed(x) ((GetKeyState(x) & 0x8000)!=0)

// idle function. called every frame
static void app_idle(long system_time)
{
  static float fps;
  static long frame_counter;
  static long frame_time;
  static long old_system_time;

  if (!old_system_time) old_system_time=system_time;
  long delta_time = system_time-old_system_time;
  old_system_time = system_time;

  // reads the keyboard and sets player_keys variable
  player_keys=0;
  for (int i=0,m=1;i<sizeof(keys)/sizeof(keys[0]);i++,m<<=1)
    if (KeyPressed(keys[i])) player_keys|=m;
  // reads the mouse
  mouse_read();
  // applies the mouse movement
  if (player_keys & kRUN) player_rotate(-mousedy*0.01,0,-mousedx*0.01);
  else player_rotate(-mousedy*0.005,0,-mousedx*0.005);
  // advances the game with the elapsed time
  if (!player_idle(system_time)) exit(0);
  prev_idle_handler( system_time );

  // draws the current frame
  DDBLTFX ddbltfx;
  ddbltfx.dwSize = sizeof(ddbltfx);
  ddbltfx.dwFillColor = 0;
  dd_buffer_back->Blt( NULL, NULL, NULL, DDBLT_COLORFILL|DDBLT_WAIT, &ddbltfx );
  d3d_device->BeginScene();
  map_draw();
  d3d_device->EndScene();

  // calculates the frame rate
  frame_counter++;
  frame_time += delta_time;
  if (frame_time>200) {
    fps = frame_counter/(frame_time/1000.f);
    frame_time = 0;
    frame_counter = 0;
  }
  HDC hdc;
  if (DD_OK==dd_buffer_back->GetDC(&hdc)) {
    char buf[80];
    sprintf( buf, " %.1f fps", fps );
    SetBkMode( hdc, TRANSPARENT );
    SetTextColor( hdc, RGB( 0, 255, 0 ) );
    TextOut( hdc, 0, 0, buf, strlen(buf) );    
    dd_buffer_back->ReleaseDC( hdc );
  }
  // flips the video buffer
  dd_buffer_front->Flip( NULL, DDFLIP_WAIT );
}

// selection of Direct 3D driver and video mode
static DisplayDriverDesc *drivers;
static int ndrivers,driver;
static DisplayDeviceDesc *devices;
static int ndevices,device;
static DisplayModeDesc *modes;
static int nmodes,mode;

static bool initdriver(int d)
{
  if (!display_driver_init(drivers+d)) return false;
  display_devices_available(&devices,&ndevices);
  int i;
  for (i=0;i<ndevices;i++)
    if (devices[i].hardware) break;
  if (i==ndevices) goto err;

  if (!display_device_init(devices+i)) goto err;
  return true;
 err:
  display_driver_done();
  return false;
}

static void changedriver(int d)
{
  cb_reset(IDC_MODE);
  driver=d;
  if (!initdriver(d)) {
    nmodes=0;
    cb_addstring(IDC_MODE,"<no HAL devices found>");
    cb_setcurrent(IDC_MODE,0);
    return;
  }
  display_modes_available(&modes,&nmodes);
  display_driver_done();
  for (int i=0;i<nmodes;i++) {
    char s[50];
    sprintf(s,"%dx%dx%d",modes[i].width,modes[i].height,modes[i].bit_depth);
    cb_addstring(IDC_MODE,s);
  }
  cb_setcurrent(IDC_MODE,0);
}

BOOL CALLBACK SelectProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
  switch (uMsg) {
    case WM_INITDIALOG:
      {
        for (int i=0;i<ndrivers;i++)
          cb_addstring(IDC_DRIVER,drivers[i].name);
        cb_setcurrent(IDC_DRIVER,0);
        cb_setcurrent(IDC_MODE,0);
        changedriver(0);
        return 1;
      }
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDC_DRIVER:
          if (HIWORD(wParam)==CBN_SELCHANGE) {
            changedriver(cb_getcurrent(IDC_DRIVER));
            SetFocus(hwndDlg);
            return 1;
          }
          else break;
        case IDOK:
          mode=cb_getcurrent(IDC_MODE);
          if (mode>=0 && mode<nmodes) enddialog(1);
          else MessageBox(hwndDlg,"No hardware accelerated devices for this driver.","BOOM D3D",MB_OK);
          return 1;
        case IDCANCEL:
          enddialog(0);
          return 1;
      }
  }
  return 0;
}

// application initialization
bool app_init(char *command_line)
{
  char name[_MAX_PATH]="";
  // selects the map file
  if (!*command_line) {
    if (!openfilename(name,"Select map","Map Files (*.boo)\0*.boo\0","boo",true)) return false;
    command_line=name;
  }
  prev_idle_handler = register_idle_handler(app_idle);

  // initializes the display
  halt(!display_init());
  halt(!display_drivers_available(&drivers,&ndrivers));
  if (!rundialog(IDD_SELECT,SelectProc)) return false;
  initdriver(driver);
  setscreensize(modes[mode].width,modes[mode].height);
  halt(!display_mode_init(1,true,modes+mode));
  set_render_state();

  // initializes the engine
  return map_init(command_line);
}

// idle function. called every frame
static void idle( void )
{
  if (activefl) {
    system_call_idle_handlers(timeGetTime());
  }
}

// application entry point
int WINAPI WinMain( HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
  if (!createmainwindow("boomd3d",hInstance,0,0,CreateSolidBrush(RGB_MAKE(0,0,0)))) return FALSE;
  system_init();
  if (!app_init(lpCmdLine)) return FALSE;
  mouse_init();
  return mainwindowloop(0,idle);
}
