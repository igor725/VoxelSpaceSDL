#include "engine.h"
#include "argparse.h"
#include <SDL_hints.h>
#include <SDL_error.h>

#ifdef _WIN32
#include <direct.h>
#define chdir _chdir
#else
#include <unistd.h>
#endif

static int TestArg(char *arg, char *test, char *alias) {
	return SDL_strcasecmp(arg, test) == 0 ||
	SDL_strcasecmp(arg, alias) == 0;
}

// TODO: Доделать когда-нибудь
static char *HelpLines[] = {
	"",
	NULL
};

static int ShowHelp(void) {
	for(int i = 0; HelpLines[i]; i++)
		SDL_Log("%s", HelpLines[i]);
	return 1;
}

int CommandArgs_Parse(int argc, char *argv[], EngineSettings *es) {
	char *pathend = SDL_strrchr(argv[0], '\\');
	if(!pathend) pathend = SDL_strrchr(argv[0], '/');
	if(pathend) {
		*pathend = '\0';
		if(chdir(argv[0]) == -1)
			SDL_LogWarn(0, "Failed to change directory");
	}

	int cursor = 1;
	while(cursor < argc) {
		char pre, *argstr = argv[cursor++];
		if((pre = *argstr++) != '/' && pre != '-') {
			return ShowHelp();
		}
		if(TestArg(argstr, "diffuse", "dm")) {
			es->diffusemap = argv[cursor++];
		} else if(TestArg(argstr, "height", "hm")) {
			es->heightmap = argv[cursor++];
		} else if(TestArg(argstr, "novsync", "nv")) {
			es->vsync = 0;
		} else if(TestArg(argstr, "winwidth", "ww")) {
			es->width = SDL_atoi(argv[cursor++]);
			if(!es->width) return ShowHelp();
		} else if(TestArg(argstr, "winheight", "wh")) {
			es->height = SDL_atoi(argv[cursor++]);
			if(!es->height) return ShowHelp();
		} else return ShowHelp();
	}

	return 0;
}
