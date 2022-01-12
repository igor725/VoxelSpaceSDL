#ifndef VSTYPES_H
#define VSTYPES_H
typedef enum eListeners {
	LISTEN_ENGINE_START, // Вызвать при запуске движка
	LISTEN_ENGINE_UPDATE, // Вызывать каждый тик
	LISTEN_ENGINE_DRAW, // Вызывать перед отправкой данных рендереру
	LISTEN_CONTROLLER_FAIL, // Вызвать при ошибке чтения геймпада
	LISTEN_CONTROLLER_ADD, // Вызвать при подключении геймпада к компьютеру
	LISTEN_CONTROLLER_DEL, // Вызвать при отключении геймпада от компьютера
	LISTEN_SDL_WINDOW, // Вызвать при создании SDL окна
	LISTEN_SDL_EVENT, // Вызывать при получении события от SDL
	LISTEN_MAX
} Listeners;

typedef enum eErrors {
	ERROR_OK,
	ERROR_MAPLOAD_LOAD,
	ERROR_MAPLOAD_IMGSIZE,
	ERROR_MAPLOAD_WIDTHINVALID,
	ERROR_MAPLOAD_SCALE,
	ERROR_MAPLOAD_MAPSMISMATCH,
	ERROR_MALLOC_FAIL,
	ERROR_MAX
} Errors;

static const char *Errors_Strings[ERROR_MAX] = {
	"ERROR_OK",
	"ERROR_MAPLOAD_LOAD",
	"ERROR_MAPLOAD_IMGSIZE",
	"ERROR_MAPLOAD_WIDTHINVALID",
	"ERROR_MAPLOAD_SCALE",
	"ERROR_MAPLOAD_MAPSMISMATCH",
	"ERROR_MALLOC_FAIL"
};

typedef struct sPoint {
	float x, y;
} Point;

#define POINT_ADD(a, b) (a).x += (b).x; (a).y += (b).y;
#define POINT_MAKE(_x, _y) {.x=_x, .y=_y}

typedef struct sMap {
	int ready; // Была ли карта успешно загружена
	int redraw; // Нужно ли перерисовывать карту
	int width, height; // Размерность карты
	int shift; // Побитовое смещение ширины карты
	int *hiddeny; // Спрятанные линии
	int *color; // Текстура карты
	unsigned char *altitude; // Высоты карты
} Map;

typedef struct sCamera {
	Point position; // Текущая позиция камеры
	float height; // Высота камеры над картой
	float distance; // Дальность прорисовки
	float horizon; // Уровень горизонта
	float angle; // Угол поворота камеры
	float zstep; // Шаг по оси Z
} Camera;
#endif
