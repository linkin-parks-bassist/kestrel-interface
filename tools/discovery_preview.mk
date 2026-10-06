# Desktop review build; replaces the selector object without changing firmware.
preview_objs := $(filter-out $(app_objdir)/components/ui/kest_effect_select.o,$(ALL_APP_OBJ))

.PHONY: discovery-preview
discovery-preview: bin/discovery-preview

$(app_objdir)/tools/discovery_preview.o: components/ui/kest_effect_select.c

bin/discovery-preview: $(preview_objs) $(app_objdir)/tools/discovery_preview.o
	gcc -o $@ $^ $(LDFLAGS_APP)

bin/discovery-rounded-preview: $(preview_objs) $(app_objdir)/tools/discovery_preview.o $(app_objdir)/tools/rounded_fill_preview.o
	gcc -o $@ $^ $(LDFLAGS_APP) -Wl,--wrap=lv_draw_sw_fill

bin/rounded-fill-check: $(LVGL_OBJ) $(FREERTOS_OBJ) $(app_objdir)/tools/rounded_fill_preview.o $(app_objdir)/tools/rounded_fill_check.o
	gcc -o $@ $^ $(LDFLAGS_APP) -Wl,--wrap=lv_draw_sw_fill

$(app_objdir)/tools/rounded_corners_preview.o: tools/rounded_fill_preview.c

bin/rounded-corners-check: $(LVGL_OBJ) $(FREERTOS_OBJ) $(app_objdir)/tools/rounded_corners_preview.o $(app_objdir)/tools/rounded_fill_check.o
	gcc -o $@ $^ $(LDFLAGS_APP) -Wl,--wrap=lv_draw_sw_fill
