#include <string.h>
#include <stdio.h>
// Говорим SDL, что не нужно переписывать main
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "main.h"

#ifdef USE_SDL_IMAGE
#include <SDL_image.h>
#endif

int windowWidth = 0, windowHeight = 0;

static SDL_Surface *ReencodeSurface(SDL_Surface *old) {
	SDL_Surface *new = old;

	if(old->format->format != SDL_PIXELFORMAT_ARGB8888) {
		new = SDL_ConvertSurfaceFormat(old, SDL_PIXELFORMAT_ARGB8888, 0);
		SDL_FreeSurface(old);
	}
	
	return new;
}

static int LoadMap(struct sMap *map, const char *diffuse, const char *height) {
	SDL_Surface *sDiffuse = NULL, *sHeight = NULL;

#ifndef USE_SDL_IMAGE
	sDiffuse = SDL_LoadBMP(diffuse);
	sHeight = SDL_LoadBMP(height);
#else
	sDiffuse = IMG_Load(diffuse);
	sHeight = IMG_Load(height);
#endif

	if(sDiffuse == NULL || sHeight == NULL) {
		SDL_LogError(0, "Failed to open image: %s.", SDL_GetError());
		return 0;
	}

	if(sDiffuse->w != sHeight->w || sDiffuse->h != sHeight->h) {
		SDL_LogError(0, "Diffuse map and height map have different dimensions.");
		return 0;
	}
	
	if((sHeight->w & (sHeight->w - 1)) != 0) {
		SDL_LogError(0, "Failed to process image: Image width must be power of two.");
		return 0;
	}

	sDiffuse = ReencodeSurface(sDiffuse);
	sHeight = ReencodeSurface(sHeight);

	map->shift = (int)log2f(sHeight->w);
	map->width = sHeight->w;
	map->height = sHeight->h;
	map->widthp = map->width - 1,
	map->heightp = map->height - 1;
	map->hiddeny = SDL_calloc(4, windowWidth);
	map->color = SDL_calloc(4, sHeight->w * sHeight->h);
	map->altitude = SDL_calloc(1, sHeight->w * sHeight->h);

	const unsigned char *datah = sHeight->pixels;
	unsigned int *datac = sDiffuse->pixels;
	for(int i = 0; i < sHeight->w * sHeight->h; i++) {
		map->color[i] = datac[i];
		map->altitude[i] = datah[i << 2];
	}

	/*
		Высвобождаем память, выделенную
		под два surface объекта, так как
		они больше не нужны.
	*/
	SDL_FreeSurface(sDiffuse);
	SDL_FreeSurface(sHeight);
	map->ready = 1;
	return 1;
}

static void FreeMap(struct sMap *map) {
	if(map->altitude) {
		SDL_free(map->altitude);
		map->altitude = NULL;
	}
	if(map->hiddeny) {
		SDL_free(map->hiddeny);
		map->hiddeny = NULL;
	}
	if(map->color) {
		SDL_free(map->color);
		map->color = NULL;
	}
	map->ready = 0;
}

static unsigned char GetHeight(struct sMap *map, Point *p) {
	if(!map->ready) return 0;
	unsigned int offset = (((int)p->y & map->widthp) << map->shift) + ((int)p->x & map->heightp);
	return map->altitude[offset];
}

static void NormalizeCameraPosition(struct sContext *ctx) {
	float minHeight = (float)GetHeight(&ctx->map, &ctx->camera.position) + 10.0f;
	if(ctx->camera.height < minHeight) ctx->camera.height = minHeight;
}

static void DrawVerticalLine(int *pixels, int x, int top, int bottom, int color) {
	// Линия за пределами окна? Низя такое.
	if(top < 0) top = 0;
	if(top > bottom) return;

	int offset = ((top * windowWidth) + x);
	for(int i = top; i < bottom; i++) {
		pixels[offset] = color;
		offset += windowWidth;
	}
}

static void DrawMap(struct sContext *ctx, int *pixels) {
	for(int i = 0; i < windowWidth * windowHeight; i++)
		pixels[i] = 0x9090E0FF;

	if(!ctx->map.ready) return;

	float sinang = SDL_sinf(ctx->camera.angle),
	cosang = SDL_cosf(ctx->camera.angle);

	for(int i = 0; i < windowWidth; i++)
		ctx->map.hiddeny[i] = windowHeight;

	float deltaz = 1.0f;
	for(float z = 1.0f; z < ctx->camera.distance; z += deltaz) {
		Point pLeft = {-cosang * z - sinang * z, sinang * z - cosang * z};
		Point pRight = {cosang * z - sinang * z, -sinang * z - cosang * z};
		Point delta = {
			(pRight.x - pLeft.x) / (float)windowWidth,
			(pRight.y - pLeft.y) / (float)windowWidth
		};

		pLeft.x += ctx->camera.position.x;
		pLeft.y += ctx->camera.position.y;

		for(int i = 0; i < windowWidth; i++) {
			unsigned int offset = (((int)pLeft.y & ctx->map.widthp) << ctx->map.shift) + ((int)pLeft.x & ctx->map.heightp);
			float top = (ctx->camera.height - (float)ctx->map.altitude[offset]) / z * 240.0f + ctx->camera.horizon;
			DrawVerticalLine(pixels, i, (int)top, ctx->map.hiddeny[i], ctx->map.color[offset]);
			/*
				Слегка ускоряем рендер путём скрытия
				накладываемых друг на друга частей линий
			*/
			if(top < ctx->map.hiddeny[i]) ctx->map.hiddeny[i] = (int)top;
			pLeft.x += delta.x; pLeft.y += delta.y;
		}

		deltaz += ctx->zstep;
	}
}

/*
	Выполняем определённые действия
	при нажатии пользователем кнопок.
*/
#define MAX_KEYBINDS 4
static SDL_Scancode input[MAX_KEYBINDS] = {0, 0, 0, 0};

static int ProcessKeyDown(struct sContext *ctx, SDL_KeyboardEvent *ev) {
	SDL_Scancode code = ev->keysym.scancode;
	switch(code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_S:
			input[0] = code;
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_D:
			input[1] = code;
			break;
		case SDL_SCANCODE_E:
		case SDL_SCANCODE_Q:
			input[2] = code;
			break;
		case SDL_SCANCODE_R:
		case SDL_SCANCODE_F:
			input[3] = code;
			break;
		case SDL_SCANCODE_SPACE:
			if(ev->keysym.mod & KMOD_LCTRL)
				ctx->camera.distance = 800.0f;
			else {
				ctx->camera.distance += (ev->keysym.mod & KMOD_SHIFT ? -150.0f : 150.0f);
				if(ctx->camera.distance < 300.0f || ctx->camera.distance > 3000.0f)
					ctx->camera.distance = 300.0f;
			}
			ctx->redrawMap = 1;
			SDL_Log("Render distance changed to %.2f.", ctx->camera.distance);
			break;
		case SDL_SCANCODE_ESCAPE: // Закрываем приложение при нажатии ESC
			return 0;
		default: break;
	}

	return 1;
}

static void ProcessKeyUp(SDL_KeyboardEvent *ev) {
	SDL_Scancode code = ev->keysym.scancode;
	switch(code) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_S:
			if(input[0] == code) input[0] = 0;
			break;
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_D:
			if(input[1] == code) input[1] = 0;
			break;
		case SDL_SCANCODE_E:
		case SDL_SCANCODE_Q:
			if(input[2] == code) input[2] = 0;
			break;
		case SDL_SCANCODE_R:
		case SDL_SCANCODE_F:
			if(input[3] == code) input[3] = 0;
			break;
		default: break;
	}
}

static void UpdateInput(struct sContext *ctx) {
	float deltaMult = (float)ctx->deltaTime * 0.03f;
	if(input[3])
		ctx->camera.horizon += (input[3] == SDL_SCANCODE_R ? 4.0f : -4.0f) * deltaMult;
	if(input[2])
		ctx->camera.height += (input[2] == SDL_SCANCODE_E ? 4.0f : -4.0f) * deltaMult;
	if(input[1]) {
		float direction = (input[1] == SDL_SCANCODE_A ? 1.0f : -1.0f);
		if(ctx->mouseGrabbed){
			ctx->camera.position.x -= 10.0f * direction * SDL_cosf(ctx->camera.angle) * deltaMult;
			ctx->camera.position.y -= 10.0f * direction * SDL_sinf(-ctx->camera.angle) * deltaMult;
		} else
			ctx->camera.angle += 0.1f * direction * deltaMult;
	}
	if(input[0]) {
		float direction = input[0] == SDL_SCANCODE_W ? 1.0f : -1.0f;
		ctx->camera.position.x -= direction * 4.0f * SDL_sinf(ctx->camera.angle) * deltaMult;
		ctx->camera.position.y -= direction * 4.0f * SDL_cosf(ctx->camera.angle) * deltaMult;
		ctx->camera.height += (ctx->camera.horizon - ctx->camera.ihorizon) * direction * 0.02f * deltaMult;
	}

	/*
		Рисуем мир заново, если была нажата
		хоть какая-то кнопка из зарезервированных.
	*/
	for(int i = 0; i < MAX_KEYBINDS; i++) if(input[i]) {
		ctx->redrawMap = 1;
		break;
	}
}

static void ToggleMouseGrab(struct sContext *ctx) {
	ctx->mouseGrabbed = !ctx->mouseGrabbed;
	SDL_SetRelativeMouseMode(ctx->mouseGrabbed);
	SDL_SetWindowGrab(ctx->wnd, ctx->mouseGrabbed);
}

static void ProcessSDLEvents(struct sContext *ctx) {
	SDL_Event ev;
	char *tmpsymptr, typesym;
	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
			case SDL_QUIT:
				ctx->running = 0;
				break;
			case SDL_KEYDOWN:
				ctx->running = ProcessKeyDown(ctx, &ev.key);
				break;
			case SDL_KEYUP:
				ProcessKeyUp(&ev.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(ev.button.button == SDL_BUTTON_LEFT)
					ToggleMouseGrab(ctx);
				break;
			case SDL_MOUSEBUTTONUP:
				if(ev.button.button == SDL_BUTTON_LEFT)
					ToggleMouseGrab(ctx);
				break;
			case SDL_MOUSEMOTION:
				if(ctx->mouseGrabbed) {
					ctx->camera.angle -= (float)ev.motion.xrel * (MOUSE_SENS / (float)ctx->deltaTime);
					ctx->camera.horizon -= (float)ev.motion.yrel;
					ctx->redrawMap = 1;
				}
				break;
			case SDL_DROPFILE:
				tmpsymptr = strrchr(ev.drop.file, '\\');
				if(!tmpsymptr) tmpsymptr = strrchr(ev.drop.file, '/');
				typesym = tmpsymptr ? *++tmpsymptr : *ev.drop.file;
				if(typesym == 'C' || typesym == 'D') {
					if(!ctx->droppedFile) {
						ctx->droppedFile = ev.drop.file;
						ctx->droppedFileType = typesym;
					} else if(ctx->droppedFileType == typesym) {
						SDL_free(ctx->droppedFile);
						ctx->droppedFile = ev.drop.file;
						ctx->droppedFileType = typesym;
					} else {
						SDL_Log("Reloading map using \"%s\" and \"%s\".", ev.drop.file, ctx->droppedFile);
						FreeMap(&ctx->map);
						if(typesym == 'C')
							ctx->redrawMap = LoadMap(&ctx->map, ev.drop.file, ctx->droppedFile);
						else
							ctx->redrawMap = LoadMap(&ctx->map, ctx->droppedFile, ev.drop.file);
						SDL_free(ev.drop.file);
						SDL_free(ctx->droppedFile);
						ctx->droppedFile = NULL;
					}
				}
				break;
		}
	}
}

static SDL_Window *CreateSDLWindow(unsigned int flags) {
	SDL_Window *wnd;
	if((wnd = SDL_CreateWindow("VoxelSpace SDL",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT, flags
	)) == NULL) return NULL;
	SDL_GetWindowSize(wnd, &windowWidth, &windowHeight);
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	return wnd;
}

static void EndSDL(struct sContext *ctx) {
	if(ctx->wnd) {
		SDL_DestroyWindow(ctx->wnd);
		ctx->wnd = NULL;
	}
	if(ctx->render) {
		SDL_DestroyRenderer(ctx->render);
		ctx->render = NULL;
	}
	if(ctx->screen) {
		SDL_DestroyTexture(ctx->screen);
		ctx->screen = NULL;
	}
	SDL_Quit();
}

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;
	struct sContext ctx = {
		.running = 1,
		.zstep = 0.002f,
		.camera = {
			.position = {
				.x = 512.0f,
				.y = 800.0f
			},
			.height = 178.0f,
			.horizon = CAMERA_HORIZON,
			.ihorizon = CAMERA_HORIZON,
			.distance = 600.0f
		}
	};

	// Инициализируем SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_LogError(0, "SDL_Init failed: %s.", SDL_GetError());
		return 1;
	}
	
	if((ctx.wnd = CreateSDLWindow(WINDOW_FLAGS)) == NULL) {
		SDL_LogError(0, "Failed to create SDL window: %s.", SDL_GetError());
		EndSDL(&ctx);
		return 1;
	}

	/*
		Ищем у пользователя рендерер, подходящий
		под наши рендерерские запросы.
	*/
	if((ctx.render = SDL_CreateRenderer(ctx.wnd, -1, RENDERER_FLAGS)) == NULL) {
		SDL_LogError(0, "Failed to create SDL renderer: %s.", SDL_GetError());
		EndSDL(&ctx);
		return 1;
	} else {
		SDL_RendererInfo ri;
		if(SDL_GetRendererInfo(ctx.render, &ri) == 0)
			SDL_Log("Using %s %s renderer.", ri.flags & SDL_RENDERER_ACCELERATED ? "hardware" : "software", ri.name);
		else
			SDL_LogError(0, "Failed to retrieve SDL renderer info: %s.", SDL_GetError());
	}

	/*
		Создаём текстуру, которая будет использоваться
		для хранения последнего отрисованного кадра.
	*/
	if((ctx.screen = SDL_CreateTexture(ctx.render,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		windowWidth, windowHeight
	)) == NULL) {
		SDL_LogError(0, "Failed to create SDL texture: %s.", SDL_GetError());
		EndSDL(&ctx);
		return 1;
	}

	ctx.redrawMap = LoadMap(&ctx.map, "maps/C1W.bmp", "maps/D1.bmp");

	unsigned int lastTime = 0, currentTime = 0;
	while(ctx.running) {
		// Обрабатываем накопившиеся эвенты SDL
		ProcessSDLEvents(&ctx);
		
		// Изменяем положение камеры в соответствии с нажатыми кнопками
		UpdateInput(&ctx);

		// Запрещаем летать за пределами мира
		NormalizeCameraPosition(&ctx);

		// Перерисовываем мир, если нужно
		if(ctx.redrawMap) {
			int *pixels, pitch;
			SDL_LockTexture(ctx.screen, NULL, (void **)&pixels, &pitch);
			DrawMap(&ctx, pixels);
			SDL_UnlockTexture(ctx.screen);
			ctx.redrawMap = 0;
		}

		// Рисуем в SDL окне нашу текстуру
		SDL_RenderClear(ctx.render);
		SDL_RenderCopy(ctx.render, ctx.screen, NULL, NULL);
		SDL_RenderPresent(ctx.render);

		lastTime = currentTime;
		currentTime = SDL_GetTicks();
		ctx.deltaTime = currentTime - lastTime;
	}
	
	// Очищаемся
	FreeMap(&ctx.map);
	EndSDL(&ctx);
	return 0;
}