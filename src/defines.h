#ifndef VSCONSTANTS_H
#define VSCONSTANTS_H
#ifndef min
#define min(a, b) (((a)<(b))?(a):(b))
#define max(a, b) (((a)>(b))?(a):(b))
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#define POINT_ADD(a, b) (a).x += (b).x; (a).y += (b).y;
#define POINT_MAKE(_x, _y) {.x=_x, .y=_y}

#ifdef USE_THREADED_RENDER
#define GRAPHICS_TITLE "VoxelSpace SDL (MT)"
#else
#define GRAPHICS_TITLE "VoxelSpace SDL"
#endif

#define GRAPHICS_WIDTH 800 // Ширина SDL окна
#define GRAPHICS_HEIGHT ((int)(GRAPHICS_WIDTH * 0.75)) // Высота SDL окна
// Для отключения вертикальной синхронизации раскомментируйте строчку ниже
// #define GRAPHICS_FLAGS SDL_RENDERER_TARGETTEXTURE
#ifndef GRAPHICS_FLAGS
#define GRAPHICS_FLAGS SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
#endif

#define INPUT_MOUSE_SENS 0.057f // Чувствительность мыши при управлении камерой
#define INPUT_TOUCH_SENS 80.0f // Чувствительность тач панели геймпада
#define INPUT_MAX_PADS 8 // Максимально возможное количество обрабатываемых геймпадов
#define INPUT_MAX_KEYBINDS 4 // Наборы клавиш, обрабатываемые движком
#define INPUT_GRAVITATION_MULT 9.81f // Множитель гравитации
#define INPUT_JUMP_VELOCITY 3.0f // Сила прыжка в режиме гравитации

#define CAMERA_MOVE_STEP 4.0f // Скорость передвижения камеры
#define CAMERA_ANGLE_STEP 0.08f // Шаг вращения камеры
#define CAMERA_HORIZON_STEP 0.0383f // Шаг изменения угла наклона
#define CAMERA_HEIGHT_DEFAULT 178.0f // Высота камеры по умолчанию
#define CAMERA_HEIGHT_MAX 1000.0f // Максимальная высота камеры
#define CAMERA_HEIGHT_MOD 0.02f // Влияние линии горизонта на вектор движения камеры
#define CAMERA_DISTANCE_DEFAULT 750.0f // Дальность прорисовки по умолчанию
#define CAMERA_DISTANCE_MIN 300.0f // Минимальная дальность прорисовки
#define CAMERA_DISTANCE_MAX 10000.0f // Максимальная дальность прорисовки
#define CAMERA_DISTANCE_STEP 150.0f // Шаг изменения дальности прорисовки
#define CAMERA_POSITION_DEFAULT POINT_MAKE(512.0f, 800.0f) // Стартовая позиция камеры
#define CAMERA_ZSTEP_DEFAULT 0.002f // Шаг по оси Z по умолчанию
#define CAMERA_ZSTEP_MIN 0.001f // Минимальный шаг по оси Z
#define CAMERA_ZSTEP_MAX 0.16f // После этого значения изображение становится месивом из пикселей
#define CAMERA_ZSTEP_MOD 0.001f // Название этого дефайна звучит устрашающе
#endif
