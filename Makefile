top_objdir := bin
app_objdir := $(top_objdir)/app
lib_objdir := $(top_objdir)/lib
test_objdir := $(top_objdir)/tests

app_cfiles := 	core/m_error_codes.c	\
				core/m_alloc.c			\
				core/m_block.c			\
				core/m_bump_arena.c		\
				core/m_context.c		\
				core/m_eff_desc.c		\
				core/m_expression.c		\
				core/m_expr_scope.c		\
				core/m_files.c			\
				core/m_hfunc.c			\
				core/m_lv_log.c			\
				core/m_parameter.c		\
				core/m_param_update.c	\
				core/m_transformer.c	\
				core/m_pipeline.c		\
				core/m_state.c			\
				core/m_sequence.c		\
				core/m_profile.c		\
				core/m_resource.c		\
				core/m_representation.c	\
				core/m_printf.c			\
				ui/m_button.c			\
				ui/m_home_view.c		\
				ui/m_menu.c				\
				ui/m_parameter_widget.c	\
				ui/m_profile_settings.c	\
				ui/m_profile_view.c		\
				ui/m_sequence_list.c	\
				ui/m_sequence_view.c	\
				ui/m_transformer_select.c	\
				ui/m_transformer_settings.c	\
				ui/m_transformer_view.c	\
				ui/m_page_id.c			\
				ui/m_ui.c				\
				fpga/m_fpga_encoding.c	\
				fpga/m_fpga_comms.c		\
				fpga/m_fpga_io.c		\
				fpga/m_reg_format.c		\
				parser/m_asm_parser.c	\
				parser/m_dict_extract.c	\
				parser/m_dictionary.c	\
				parser/m_eff_parser.c	\
				parser/m_eff_section.c	\
				parser/m_expr_parser.c	\
				parser/m_tokenizer.c

lib_cfiles := 	core/m_error_codes.c	\
				core/m_representation.c	\
				core/m_parameter.c		\
				core/m_resource.c		\
				core/m_expression.c		\
				core/m_expr_scope.c		\
				core/m_block.c			\
				core/m_eff_desc.c		\
				core/m_transformer.c	\
				core/m_pipeline.c		\
				core/m_profile.c		\
				core/m_printf.c			\
				core/m_hfunc.c			\
				core/m_bump_arena.c		\
				parser/m_tokenizer.c	\
				parser/m_expr_parser.c	\
				parser/m_dictionary.c	\
				parser/m_eff_parser.c	\
				parser/m_eff_section.c	\
				parser/m_asm_parser.c	\
				parser/m_dict_extract.c	\
				fpga/m_fpga_encoding.c	\
				fpga/m_reg_format.c		\
				fpga/m_fpga_io.c


lib_hfiles := core/m_linked_list.h $(lib_cfiles:.c=.h)
app_hfiles := core/m_linked_list.h $(app_cfiles:.c=.h)

lib_srcs := $(addprefix $(components)/,$(lib_cfiles))
lib_objs := $(addprefix $(lib_objdir)/,$(lib_cfiles:.c=.o))
lib_hdrs := main/m_lib.h main/m_lib_cmph.h $(addprefix components/,$(lib_hfiles))

app_objs := $(app_objdir)/m_desktop.o $(addprefix $(app_objdir)/,$(app_cfiles:.c=.o))
app_hdrs := desktop/m_desktop.h $(addprefix components/,$(app_hfiles))

COMP_DIRS := $(wildcard components/*)
INC_FLAGS_LIB := $(addprefix -I,$(COMP_DIRS))
INC_FLAGS_APP := \
	-Idesktop \
	-Idesktop/lvgl \
	-Idesktop/freertos/include \
	-Idesktop/freertos/portable/ThirdParty/GCC/Posix \
	-Idesktop/freertos/portable/MemMang \
	$(addprefix -I,$(COMP_DIRS))

CFLAGS_LIB := -fPIC -lm -DM_LIBRARY -Imain $(INC_FLAGS_LIB) -g
CFLAGS_APP := -DM_DESKTOP -Idesktop -Imain $(INC_FLAGS_APP) -g
LDFLAGS_APP := -lm -lSDL2 -lpthread
CFLAGS_TEST := $(CFLAGS_APP) -Itests

HDR_INSTALL_DIR := /usr/include/libM/

LVGL_DIR := desktop/lvgl

LVGL_SRC := $(shell find $(LVGL_DIR)/src -name "*.c"  ! -name "lv_linux.c"  ! -name "lv_pthread.c")
LVGL_OBJ := $(patsubst $(LVGL_DIR)/src/%.c,$(app_objdir)/lvgl/%.o,$(LVGL_SRC))

APP_SRC := desktop/m_desktop.c $(addprefix components/,$(app_cfiles))
APP_OBJ := $(patsubst %.c,$(app_objdir)/%.o,$(APP_SRC))

FREERTOS_DIR := desktop/freertos

FREERTOS_CORE_SRC := \
	$(FREERTOS_DIR)/tasks.c \
	$(FREERTOS_DIR)/queue.c \
	$(FREERTOS_DIR)/list.c \
	$(FREERTOS_DIR)/timers.c \
	$(FREERTOS_DIR)/event_groups.c \
	$(FREERTOS_DIR)/stream_buffer.c

FREERTOS_HEAP_SRC := \
	$(FREERTOS_DIR)/portable/MemMang/heap_4.c

FREERTOS_PORT_SRC := \
	$(FREERTOS_DIR)/portable/ThirdParty/GCC/Posix/port.c \
	$(FREERTOS_DIR)/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c

FREERTOS_SRC := \
	$(FREERTOS_CORE_SRC) \
	$(FREERTOS_HEAP_SRC) \
	$(FREERTOS_PORT_SRC)

FREERTOS_OBJ := $(patsubst %.c,$(app_objdir)/%.o,$(FREERTOS_SRC))

ALL_APP_OBJ := $(APP_OBJ) $(LVGL_OBJ) $(FREERTOS_OBJ)

TEST_SRC := $(shell find tests -name "*.c")
TEST_OBJ := $(patsubst tests/%.c,$(test_objdir)/%.o,$(TEST_SRC))
TEST_APP_OBJ := $(filter-out $(app_objdir)/desktop/m_desktop.o,$(APP_OBJ))

app: M

tests: M_tests

clean:
	rm -r $(top_objdir)

appclean:
	rm -r $(app_objdir)/desktop $(app_objdir)/components

appclean_full: 
	rm -r $(app_objdir)

fullclean:
	rm -r $(top_objdir)
	
M: $(ALL_APP_OBJ) | $(app_objdir) $(app_objdir)/desktop
	gcc -o $@ $^ `sdl2-config --libs` -lm -lpthread

M_tests: $(TEST_OBJ) $(TEST_APP_OBJ) $(LVGL_OBJ) $(FREERTOS_OBJ)
	gcc -Wl,--undefined=__start___m_tests -Wl,--undefined=__stop___m_tests \
	    -o $@ $^ `sdl2-config --libs` -lm -lpthread

all : app lib
	idf.py build

$(app_objdir)/%.o: %.c $(app_hdrs) | $(app_objdir)
	mkdir -p $(dir $@)
	gcc $(CFLAGS_APP) `sdl2-config --cflags` -c $< -o $@

$(app_objdir)/%.o : components/%.c | $(app_objdir)
	gcc $(CFLAGS_APP) -c $< -o $@

$(app_objdir)/desktop/%.o : desktop/%.c | $(app_objdir)/desktop
	gcc $(CFLAGS_APP) -c $< -o $@

$(app_objdir)/lvgl/%.o: $(LVGL_DIR)/src/%.c | $(app_objdir)
	mkdir -p $(dir $@)
	gcc $(CFLAGS_APP) `sdl2-config --cflags` -c $< -o $@

$(app_objdir)/desktop/freertos/%.o: desktop/freertos/%.c | $(app_objdir)
	mkdir -p $(dir $@)
	gcc $(CFLAGS_APP) `sdl2-config --cflags` -c $< -o $@

$(test_objdir)/%.o: tests/%.c | $(test_objdir)
	mkdir -p $(dir $@)
	gcc $(CFLAGS_TEST) -c $< -o $@

lib: $(lib_objdir)/libM.so

$(lib_objdir)/libM.so: $(lib_objs)
	gcc -shared -o $@ $^

lib_install: | $(HDR_INSTALL_DIR)
	cp $(lib_objdir)/libM.so /usr/lib/
	cp $(lib_hdrs) $(HDR_INSTALL_DIR)
	ldconfig

$(lib_objdir)/%.o : components/%.c | $(lib_objdir)
	gcc $(CFLAGS_LIB) -c $< -o $@

$(top_objdir):
	mkdir $(top_objdir)

$(app_objdir): | $(top_objdir)
	mkdir $(app_objdir)
	mkdir $(app_objdir)/components
	mkdir $(app_objdir)/components/core
	mkdir $(app_objdir)/components/parser
	mkdir $(app_objdir)/components/fpga
	mkdir $(app_objdir)/components/ui

$(app_objdir)/desktop : | $(app_objdir)
	mkdir $(app_objdir)/desktop

$(lib_objdir):  | $(top_objdir)
	mkdir $(lib_objdir)
	mkdir $(lib_objdir)/core
	mkdir $(lib_objdir)/parser
	mkdir $(lib_objdir)/fpga

$(test_objdir): | $(top_objdir)
	mkdir $(test_objdir)

$(HDR_INSTALL_DIR) :
	mkdir $(HDR_INSTALL_DIR)

libclean:
	rm -r $(lib_objdir)
