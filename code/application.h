#ifndef APPLICAITON_H
#define APPLICATION_H

struct application
{
	SDL_Window* Window;
	SDL_GLContext Context;
	v2 Mouse;
	v2 RelMouse;
	bool IsRunning;
	bool Keys[SDL_NUM_SCANCODES];
};

#endif