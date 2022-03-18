//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   geom.cpp - basic geometry functions implementation                 //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _GEOM_CPP
#include "geom.h"

#include <math.h>

// sets width and height of the screen
void setscreensize(int w,int h)
{
  // FOV: de momento, el FOV es 90° horizontal en una pantalla 4:3
  //   -> ~67° vertical en cualquier pantalla horizontal
  //   -> ~67° horizontal en cualquier pantalla vertical
  // El espacio cámara es 0..1, por tanto el lado más estrecho mide <1
  // OJO! El FOV vertical habrá que limitarlo ya que limita la rotación de la cámara en X
  // (puesto que no se permite proyectar nada que esté detrás de la cámara)
  // (ver pág 32-33 del doc)

  if(w > h) {
    scr_dx = 1.f;
    scr_dy = (coord2d)h/w;
    scr_ox = scr_dx/2;
    scr_oy = scr_dy/2;
    scr_foc = scr_oy*4/3;
  } else {
    scr_dx = (coord2d)w/h;
    scr_dy = 1.f;
    scr_ox = scr_dx / 2;
    scr_oy = scr_dy / 2;
    scr_foc = scr_ox*3/4;
  }
  screenw = w;
  screenh = h;
}

// sets current view point and view angles
void setviewpoint( coord3d x,coord3d y,coord3d z,angle ax,angle ay,angle az )
{
  view.x=x;
  view.y=y;
  view.z=z;
  viewa[0]=ax;
  viewa[1]=ay;
  viewa[2]=az;
  sinx=sin(ax);cosx=cos(ax);
  siny=sin(ay);cosy=cos(ay);
  sinz=sin(az);cosz=cos(az);
}

// applies rotation along z axis
void rotatez(coord3d x,coord3d y,coord3d *xr,coord3d *yr)
{
  *xr=(x-view.x)*cosz+(y-view.y)*sinz;
  *yr=-(x-view.x)*sinz+(y-view.y)*cosz;
}

// applies rotation along x and y axes
void rotatexyp(coord3d x,coord3d y,coord3d z,coord2d *xr,coord2d *yr)
{
  coord3d x1=x*cosy+z*siny;
  coord3d z1=-x*siny+z*cosy;
  coord3d y2=y*cosx+z1*sinx;
  coord3d z2=-y*sinx+z1*cosx;
  *xr=x1*scr_foc/y2;
  *yr=z2*scr_foc/y2;
}

// applies rotation along y and x axes
void rotateyxp(coord3d x,coord3d y,coord3d z,coord2d *xr,coord2d *yr)
{
  coord3d y1=y*cosx-z*sinx;
  coord3d z1=y*sinx+z*cosx;
  coord3d x2=x*cosy-z1*siny;
  coord3d z2=x*siny+z1*cosy;
  *xr=x2*scr_foc/y1;
  *yr=z2*scr_foc/y1;
}

// applies rotation along y and x axes
void rotateyxv(Tvector *v)
{
  coord3d y1=v->y*cosx+v->z*sinx;
  coord3d z1=-v->y*sinx+v->z*cosx;
  coord3d x2=v->x*cosy+z1*siny;
  coord3d z2=-v->x*siny+z1*cosy;
  v->x=x2;v->y=y1;v->z=z2;
}

// projects (x,y,z) to screen coordinates
void project(coord3d x,coord3d y,coord3d z,coord2d *x2,coord2d *y2)
{
  *x2=x*scr_foc/z;
  *y2=(y-view.z)*scr_foc/z;
}

// projects only x coordinate
coord2d projectx(coord3d x,coord3d y)
{
  return x*scr_foc/y;
}

// projects only z coordinate
coord2d projectz(coord3d z,coord3d y)
{
  return (z-view.z)*scr_foc/y;
}
