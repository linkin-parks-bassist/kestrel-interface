#include <esp_lcd_touch.h>

#include "bsp/esp32_p4_nano.h"
#include "soc/soc_caps.h"
#include <inttypes.h>

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#include "bsp/esp32_p4_nano.h"

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "esp_dma_utils.h"

#include <lvgl.h>

#include "m_error_codes.h"

int waveshare_dsi_touch_5_a_init(lv_disp_t **disp)
{
	bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false,
        }
    };
    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

	return NO_ERROR;
}
