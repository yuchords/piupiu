#include "sdl_driver.h"
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 240
#define WINDOW_HEIGHT 240

static SDL_Window * window;
static SDL_Renderer * renderer;
static SDL_Texture * texture;

static void monitor_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void monitor_cb(lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px);
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

static uint32_t keypad_key = 0;
static lv_indev_state_t keypad_state = LV_INDEV_STATE_RELEASED;

static uint32_t keycode_to_ctrl_key(SDL_Keycode sdl_key)
{
    switch(sdl_key) {
        case SDLK_UP: return LV_KEY_PREV;
        case SDLK_DOWN: return LV_KEY_NEXT;
        case SDLK_RIGHT: return LV_KEY_NEXT;
        case SDLK_LEFT: return LV_KEY_PREV;
        case SDLK_ESCAPE: return LV_KEY_ESC;
        case SDLK_DELETE: return LV_KEY_DEL;
        case SDLK_BACKSPACE: return LV_KEY_BACKSPACE;
        case SDLK_RETURN:
        case SDLK_KP_ENTER: return LV_KEY_ENTER;
        case SDLK_TAB: return LV_KEY_NEXT;
        case SDLK_HOME: return LV_KEY_HOME;
        case SDLK_END: return LV_KEY_END;
        default: return sdl_key;
    }
}

extern "C" void sdl_driver_event_handler(SDL_Event * event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
            keypad_state = LV_INDEV_STATE_PRESSED;
            keypad_key = keycode_to_ctrl_key(event->key.keysym.sym);
            break;
        case SDL_KEYUP:
            keypad_state = LV_INDEV_STATE_RELEASED;
            break;
        default:
            break;
    }
}

extern "C" void sdl_driver_init(void)
{
    SDL_Init(SDL_INIT_VIDEO);


    window = SDL_CreateWindow("Piupiu Simulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);

    /*Initialize the display*/
    static lv_disp_draw_buf_t disp_buf;
    static lv_color_t buf[WINDOW_WIDTH * 40]; /*Declare a buffer for 40 lines*/
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, WINDOW_WIDTH * 40);    /*Initialize the display buffer*/

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.flush_cb = monitor_flush;    /*Set your driver function*/
    disp_drv.monitor_cb = monitor_cb;     /*Set the monitor function to update the screen*/
    disp_drv.draw_buf = &disp_buf;          /*Assign the buffer to the display*/
    disp_drv.hor_res = WINDOW_WIDTH;       /*Set the horizontal resolution*/
    disp_drv.ver_res = WINDOW_HEIGHT;      /*Set the vertical resolution*/
    lv_disp_drv_register(&disp_drv);        /*Finally register the driver*/

    /*Initialize the input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);             /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = mouse_read;            /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    /*Initialize the keypad driver*/
    static lv_indev_drv_t keypad_drv;
    lv_indev_drv_init(&keypad_drv);
    keypad_drv.type = LV_INDEV_TYPE_KEYPAD;
    keypad_drv.read_cb = keypad_read;
    lv_indev_t * keypad_indev = lv_indev_drv_register(&keypad_drv);
    
    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(keypad_indev, g);
}

/**
 * Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished.
 * @param disp_drv pointer to display driver
 * @param area the area to update
 * @param color_p pointer to the color buffer
 */
static void monitor_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    lv_coord_t hres = disp_drv->hor_res;
    lv_coord_t vres = disp_drv->ver_res;

    /*Return if the area is out the screen*/
    if(area->x2 < 0 || area->y2 < 0 || area->x1 > hres - 1 || area->y1 > vres - 1) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    SDL_Rect rect;
    rect.x = area->x1;
    rect.y = area->y1;
    rect.w = w;
    rect.h = h;

    SDL_UpdateTexture(texture, &rect, color_p, w * sizeof(lv_color_t));

    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);
}

/**
 * Called when the frame is flushed (all chunks are rendered)
 */
static void monitor_cb(lv_disp_drv_t * disp_drv, uint32_t time, uint32_t px)
{
    (void)disp_drv;
    (void)time;
    (void)px;

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

/**
 * Read the mouse position
 * @param indev_drv pointer to the input device driver
 * @param data pointer to an input device data to fill
 */
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

    int x, y;
    const uint32_t btn = SDL_GetMouseState(&x, &y);

    int win_w = 0;
    int win_h = 0;
    SDL_GetWindowSize(window, &win_w, &win_h);
    if(win_w > 0 && win_h > 0) {
        x = x * WINDOW_WIDTH / win_w;
        y = y * WINDOW_HEIGHT / win_h;
    }

    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x >= WINDOW_WIDTH) x = WINDOW_WIDTH - 1;
    if(y >= WINDOW_HEIGHT) y = WINDOW_HEIGHT - 1;

    data->point.x = x;
    data->point.y = y;

    if(btn & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/**
 * Read the keypad input
 */
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/
    data->state = keypad_state;
    data->key = keypad_key;
}

extern "C" uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        start_ms = SDL_GetTicks();
    }

    return SDL_GetTicks() - start_ms;
}
