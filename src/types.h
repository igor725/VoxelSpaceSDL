#ifndef VSTYPES_H
#define VSTYPES_H
typedef enum eListeners {
	LISTEN_ENGINE_START, // Вызвать при запуске движка
	LISTEN_ENGINE_UPDATE, // Вызывать каждый тик
	LISTEN_ENGINE_DRAW, // Вызывать перед отправкой данных рендереру
	LISTEN_SDL_WINDOW, // Вызвать при создании SDL окна
	LISTEN_SDL_EVENT, // Вызывать при получении события от SDL
	LISTEN_MAX
} Listeners;

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
