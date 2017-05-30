#include "SDL.h"
#include <iostream>

int main( int argc, char* argv[] )
{
	SDL_Init( SDL_INIT_EVERYTHING );
	atexit( SDL_Quit );
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_CreateWindowAndRenderer( 1024, 600, SDL_WINDOW_RESIZABLE, &window, &renderer );
	SDL_SetWindowTitle( window, "Boom 3 Demo" );
	bool quit = false;
	do {
		SDL_Event event;
		while( SDL_PollEvent( &event ) ) {
			switch( event.type ) {
				case SDL_EventType::SDL_KEYDOWN:
					if( event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & KMOD_CTRL) ) {
						quit = true;
					}
					break;
			}
			if( quit ) break;
		}
		if( quit ) break;
		SDL_RenderPresent( renderer );
	} while( !quit );
	return 0;
}