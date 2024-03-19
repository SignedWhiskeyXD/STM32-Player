#include "button.h"
#include "daemon_tasks.h"
#include "display.h"
#include "flash/w25_flash.h"
#include "states/states.h"
#include "stm32f1xx_hal.h"
#include "vs1053/vs1053.h"
#include "ili9341/lcd.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/display/ili9341/lv_ili9341.h"

static void SystemClock_Config()
{
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitTypeDef RCC_OscInitStruct = {.OscillatorType      = RCC_OSCILLATORTYPE_HSI,
                                            .HSIState            = RCC_HSI_ON,
                                            .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
                                            .PLL.PLLState        = RCC_PLL_ON,
                                            .PLL.PLLSource       = RCC_PLLSOURCE_HSI_DIV2,
                                            .PLL.PLLMUL          = RCC_PLL_MUL16};
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
                                            .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
                                            .AHBCLKDivider  = RCC_SYSCLK_DIV1,
                                            .APB1CLKDivider = RCC_HCLK_DIV1,
                                            .APB2CLKDivider = RCC_HCLK_DIV1};
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void initPlayer()
{
    HAL_Init();
    SystemClock_Config();

    initScreen();
    initKeys();
    SPI_FLASH_Init();

    MyError error = initSD();
    if (error != OPERATION_SUCCESS) {
        setLastError(error);
        setGlobalState(PLAYER_ERROR);
        return;
    }
    loadFiles();

    VS_Init();

    if (VS_Ram_Test() != VS_RAM_TEST_GOOD) {
        setLastError(VS_RAM_TEST_FAILED);
        setGlobalState(PLAYER_ERROR);
        return;
    }

    VS_HD_Reset();
    VS_Soft_Reset();

    setGlobalState(BROWSING_MENU);
}

void ui_init()
{
    lv_obj_t *obj;

    /* set screen background to white */
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);

    /* create label */
    obj = lv_label_create(scr);
    lv_obj_set_align(obj, LV_ALIGN_CENTER);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_width(obj, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(obj, lv_color_black(), 0);
    lv_label_set_text(obj, "Hello World!");
}

void initTFT()
{
    /* Initialize LVGL */
    lv_init();

    /* Initialize LCD I/O */
    if (lcdInit() != 0)
        return;

    /* Create the LVGL display object and the LCD display driver */
    lcd_disp = lv_ili9341_create(LCD_H_RES, LCD_V_RES, LV_LCD_FLAG_NONE, lcd_send_cmd, lcd_send_color);
    lv_display_set_rotation(lcd_disp, LV_DISPLAY_ROTATION_270);

    /* Allocate draw buffers on the heap. In this example we use one partial buffers of 1/10th size of the screen */
    lv_color_t * buf1 = NULL;

    uint32_t buf_size = LCD_H_RES * LCD_V_RES / 10 * lv_color_format_get_size(lv_display_get_color_format(lcd_disp));

    buf1 = lv_malloc(buf_size);

    lv_display_set_buffers(lcd_disp, buf1, NULL, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    ui_init();
}

int main()
{
    initPlayer();

    initTFT();

    launchDaemonTasks();

    while (1) {}
}
