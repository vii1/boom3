//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   boomdos.cpp - main file for MS-DOS version                         //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "engine/boom.h"
#include "engine/player.h"
#include "engine/draw.h"
#include "engine/geom.h"
#include "kbd.h"
#include "timer.h"
#include "mouse.h"
#include "grtext.h"
#include "vbe20.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

static int scrwidth=0,scrheight=0;
static bool waitfl=true;
static bool backbuffl=false;

// parses command line parameters
void parsecmdline(int argc,char *argv[])
{
  for (int i=0;i<argc;i++) {
    char *arg=argv[i];
    if (arg[0]=='/' ||arg[0]=='-') {
      switch(arg[1]) {
        case 'x':
        case 'X':
          scrwidth=atol(arg+2);
          break;
        case 'y':
        case 'Y':
          scrheight=atol(arg+2);
          break;
        case 'w':
        case 'W':
          waitfl=false;
          break;
        case 'b':
        case 'B':
          backbuffl=true;
          break;
      }
    }
  }
}

void printusage( void )
{
  printf("\nBOOM 2 Engine by Ivaylo Beltchev\n");
  printf("MS-DOS version\n");
  printf("Usage: boomdos <boo file> [/x<width> /y<height> /w /b]\n");
  printf("/x<width>, /y<height> - screen size (requires VBE 2.0)\n");
  printf("/w - don't wait for the vertical retrace (VBE 2.0 only)\n");
  printf("/b - back buffer is in video memory (VBE 2.0 only)\n");
  printf("Example: boomdos level.boo /x640 /y480 /w\n");
  printf("Controls:\n");
  printf("Arrows    - forward, backward, turn left, turn right\n");
  printf("Home/PgUp - strafe left/right\n");
  printf("X         - crouch\n");
  printf("Space     - jump\n");
  printf("A/Z       - look up/down\n");
  printf("O/P       - rotate left/right\n");
  printf("NumPad 5  - center view\n");
  printf("Shift     - run\n");
  printf("Esc       - exit\n\n");
  if (!vbe_init()) {
    printf("VBE 2.0 not found.\n\n");
    vbe_done();
    return;
  }
  printf("Press Enter to view the available video modes. Esc to skip.\n");
  char c;
  do
    c=getch();
  while (c!=27 && c!=13);
  if (c==27) {
    vbe_done();
    return;
  }
  for (int i=0;i<vbemodenum;i++)
    if (vbemodes[i].bpp==8)
      printf("%dx%d\n",vbemodes[i].width,vbemodes[i].height);
  vbe_done();
}

// application entry point
void main(int argc,char *argv[])
{
  int system_time=0;
  int frame_counter=0;
  int time0;
  char s[20]="";

  if (argc<2) {
    printusage();
    return;
  }

  // parses command line parameters
  parsecmdline(argc-2,argv+2);

  // initializes the video mode and geometry module
  int mode=-1;
  if (vbe_init()) {
    mode=vbe_findmode(scrwidth,scrheight,8);
    if (!mode) mode=-1;
  }
  if (mode==-1) vbe_done();
  if (!vbe_setmode(mode,backbuffl)) goto err;
  setscreensize(vbe_width,vbe_height);
  scr_cols=vbe_width;
  scr_bpp=vbe_bpp;

  // initializes the keyboard
  kbd_init();

  // initializes the timer
  timer_init();

  // initializes the mouse
  mouse_init();

  // reads the map
  if (!map_init(argv[1])) goto err;
  vbe_setpal((Tcolor *)palette,0,256);

  // main loop
  time0=timer_clocks;
  while(player_idle(timer_clocks)) {
    // draws the current frame
    draw_init(vbe_backbuffer,vbe_width,vbe_height);
    map_draw();

    // calculates the frame rate
    frame_counter++;
    int dt=timer_clocks-time0;
    if (dt>500) {
      float fps = (frame_counter*1000.)/dt;
      sprintf(s,"%.1f FPS",fps);
      time0 = timer_clocks;
      frame_counter = 0;
    }
    textout(vbe_backbuffer,0,0,s,255);

    // flips the video pages
    vbe_flip(waitfl);

    // applies the mouse movement
    if (player_keys & kRUN) player_rotate(-mousedy*0.01,0,-mousedx*0.01);
    else player_rotate(-mousedy*0.005,0,-mousedx*0.005);
    mousedx=0;mousedy=0;

    // clears the keyboard queue
    while (kbhit()) getch();
  }
 err:
  vbe_done();
  vbe_settext();
}
