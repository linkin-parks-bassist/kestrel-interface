#ifndef KEST_INT_EFFECT_SELECT_H_
#define KEST_INT_EFFECT_SELECT_H_

typedef struct
{
	lv_obj_t *button;
	lv_obj_t *label;
	
	uint16_t type;
	char *name;
	kest_effect_desc *eff;
} kest_effect_selector_button;

DECLARE_LINKED_PTR_LIST(kest_effect_selector_button);

typedef struct
{
	lv_obj_t *button_list;
	
	kest_effect_selector_button_pll *buttons;
	
	int page_offset;
} kest_effect_selector_str;

int init_effect_selector_button(kest_effect_selector_button *button, int index);
int init_effect_selector_button_from_effect(kest_effect_selector_button *button, kest_effect_desc *eff);

//char *effect_type_name(uint16_t type);

int init_effect_selector(kest_ui_page *page);
int init_effect_selector_eff(kest_ui_page *page);
int configure_effect_selector(kest_ui_page *page, void *data);
int create_effect_selector_ui(kest_ui_page *page);
int create_effect_selector_ui_eff(kest_ui_page *page);
int free_effect_selector_ui(kest_ui_page *page);
int enter_effect_selector(kest_ui_page *page);
int refresh_effect_selector(kest_ui_page *page);

void enter_effect_selector_cb(lv_event_t *e);

#endif

