#ifndef VSCONSTANTS_H
#define VSCONSTANTS_H
#define WINDOW_WIDTH 800 // Ширина SDL окна
#define WINDOW_HEIGHT ((int)(WINDOW_WIDTH * 0.75)) // Высота SDL окна
#define SKY_COLOR 0x9090E0FF // Цвет неба в формате RGBA
#define MOUSE_SENS 0.057f // Чувствительность мыши при управлении камерой
#define WINDOW_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE // SDL флаги для создания окна
#define RENDERER_FLAGS SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE // SDL флаги для рендерера

#define INPUT_MAX_CONTROLLERS 8 // Максимально возможное количество обрабатываемых геймпадов
#define INPUT_MAX_KEYBINDS 4 //

#define CAMERA_MOVE_STEP 4.0f // Скорость передвижения камеры
#define CAMERA_ANGLE_STEP 0.08f // Шаг вращения камеры
#define CAMERA_HORIZON_STEP 23.0f // Шаг изменения угла наклона
#define CAMERA_HEIGHT_STEP 4.0f // Шаг изменения высоты
#define CAMERA_HEIGHT_MOD 0.02f // Влияние линии горизонта на вектор движения камеры
#define CAMERA_DISTANCE_STEP 150.0f // Шаг изменения дальности прорисовки
#define CAMERA_HEIGHT_DEFAULT 178.0f // Высота камеры по умолчанию
#define CAMERA_DISTANCE_DEFAULT 750.0f // Дальность прорисовки по умолчанию
#define CAMERA_POSITION_DEFAULT POINT_MAKE(512.0f, 800.0f) // Стартовая позиция камеры
#define CAMERA_HORIZON_DEFAULT ((float)WINDOW_HEIGHT * 0.50f) // Угол наклона камеры по умолчанию
#define CAMERA_ZSTEP_DEFAULT 0.002f // Шаг по оси Z по умолчанию
#define CAMERA_ZSTEP_MIN 0.001f // Минимальный шаг по оси Z
#define CAMERA_ZSTEP_MAX 0.24f // После этого значения изображение становится месивом из пикселей
#define CAMERA_ZSTEP_STEP 0.001f // Название этого дефайна звучит устрашающе
#endif
