idf_component_register(SRCS "kest_sgtl5000.c"
							"kest_sd.c"
							"kest_footswitch.c"
							"waveshare_dsi_touch_5_a.c"
							"kest_touch_irq.c"
							"kest_i2c.c"
					REQUIRES
							driver
							esp_lcd_touch_gt911
							esp32_p4_nano
							fatfs
							core
							parser
							fpga
							tinyusb
							esp_tinyusb
					INCLUDE_DIRS "." "../../main" "../" "../ui" "../core" "../parser")
