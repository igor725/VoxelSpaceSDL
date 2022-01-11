#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_log.h>
#include "engine.h"
#include "map.h"

char droppedFileType = 0;
char *droppedFile = NULL;

void DND_Window(void *ptr) {
	if(ptr) SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
}

void DND_Event(void *ptr) {
	SDL_Event *ev = (SDL_Event *)ptr;
	char *tmpsymptr = NULL, typesym = 0;
	Camera *cam = NULL;
	Map *map = NULL;
	switch(ev->type) {
		case SDL_DROPFILE:
			tmpsymptr = SDL_strchr(ev->drop.file, '\\');
			if(!tmpsymptr) tmpsymptr = SDL_strchr(ev->drop.file, '/');
			typesym = tmpsymptr ? *++tmpsymptr : *ev->drop.file;
			if(typesym == 'C' || typesym == 'c' || typesym == 'D' || typesym == 'd') {
				if(!droppedFile) {
					droppedFile = ev->drop.file;
					droppedFileType = typesym;
				} else if(droppedFileType == typesym) {
					SDL_free(droppedFile);
					droppedFile = ev->drop.file;
					droppedFileType = typesym;
				} else {
					Engine_GetObjects(&cam, &map);
					Map_Close(map);
					if(typesym == 'C' || typesym == 'c')
						Map_Open(map, ev->drop.file, droppedFile);
					else
						Map_Open(map, droppedFile, ev->drop.file);
					SDL_free(ev->drop.file);
					SDL_free(droppedFile);
					droppedFile = NULL;
				}
			}
			break;
	}
}
