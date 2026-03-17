#ifndef M_INT_TRANSFORMER_SELECT_H_
#define M_INT_TRANSFORMER_SELECT_H_

typedef struct
{
	lv_obj_t *button;
	lv_obj_t *label;
	
	uint16_t type;
	char *name;
	m_effect_desc *eff;
} m_effect_selector_button;

DECLARE_LINKED_PTR_LIST(m_effect_selector_button);

typedef struct
{
	lv_obj_t *button_list;
	
	m_effect_selector_button_pll *buttons;
	
	int page_offset;
} m_effect_selector_str;

int init_effect_selector_button(m_effect_selector_button *button, int index);
int init_effect_selector_button_from_effect(m_effect_selector_button *button, m_effect_desc *eff);

//char *effect_type_name(uint16_t type);

int init_effect_selector(m_ui_page *page);
int init_effect_selector_eff(m_ui_page *page);
int configure_effect_selector(m_ui_page *page, void *data);
int create_effect_selector_ui(m_ui_page *page);
int create_effect_selector_ui_eff(m_ui_page *page);
int free_effect_selector_ui(m_ui_page *page);
int enter_effect_selector(m_ui_page *page);
int refresh_effect_selector(m_ui_page *page);

void enter_effect_selector_cb(lv_event_t *e);

#endif

