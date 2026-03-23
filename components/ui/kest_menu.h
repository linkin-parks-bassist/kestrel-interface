#ifndef KEST_INT_MAIN_MENU_H_
#define KEST_INT_MAIN_MENU_H_

struct kest_menu_page_str;

#define MENU_ITEM_TEXT_MAX_LEN 256

#define MENU_ITEM_PAD					0
#define MENU_ITEM_PAGE_LINK 			1
#define MENU_ITEM_PAGE_LINK_INDIRECT 	2
#define MENU_ITEM_CALLBACK_BUTTON 		3
#define MENU_ITEM_PRESET_LISTING 		4
#define MENU_ITEM_SEQUENCE_LISTING 		5
#define MENU_ITEM_PARAMETER_WIDGET		6
#define MENU_ITEM_DANGER_BUTTON			7

typedef struct kest_menu_item
{
	int type;
	
	char *text;
	char *desc;
	
	void (*action_cb)(void *arg);
	void (*click_cb)(lv_event_t *e);
	void *cb_arg;
	
	void *data;
	
	lv_obj_t *obj;
	lv_obj_t *label;
	lv_obj_t **extra;
	
	lv_timer_t *timer;
	
	kest_ui_page *linked_page;
	kest_ui_page **linked_page_indirect;
	
	kest_ui_page *parent;
	
	int long_pressed;
	void *lp_configure_arg;
	
	kest_representation rep;
} kest_menu_item;

DECLARE_LINKED_PTR_LIST(kest_menu_item);

int init_menu_item(kest_menu_item *item);

typedef struct kest_menu_page_str
{
	int type;
	
	char *name;
	
	kest_menu_item_pll *items;
	kest_ui_page *next_page;
	
	void *data;
	
} kest_menu_page_str;

typedef struct kest_main_menu_str
{
	lv_obj_t *top_pad;
	lv_obj_t *pw_pad;
	lv_obj_t *gains_container;
	kest_parameter_widget input_gain;
	kest_parameter_widget output_gain;
	
	kest_button presets_button;
	kest_button sequences_button;
	
	kest_button msc_button;
	
	kest_danger_button erase_sd_card_button;
} kest_main_menu_str;

int init_menu_item(kest_menu_item *item);
int create_menu_item_ui(kest_menu_item *item, lv_obj_t *parent);
int delete_menu_item_ui(kest_menu_item *item);
int free_menu_item(kest_menu_item *item);
int refresh_menu_item(kest_menu_item *item);

int init_menu_page_str(kest_menu_page_str *str);

int init_menu_page(kest_ui_page *page);
int configure_menu_page(kest_ui_page *page, void *data);
int create_menu_page_ui(kest_ui_page *page);
int free_menu_page_ui(kest_ui_page *page);
int enter_menu_page(kest_ui_page *page);
int refresh_menu_page(kest_ui_page *page);

int menu_page_add_item(kest_menu_page_str *str, kest_menu_item *item);

int init_main_menu(kest_ui_page *page);
int configure_main_menu(kest_ui_page *page, void *data);
int create_main_menu_ui(kest_ui_page *page);
int enter_main_menu(kest_ui_page *page);

int menu_page_remove_item(kest_ui_page *page, kest_menu_item *item);

void enter_main_menu_cb(lv_event_t *e);

kest_menu_item *create_preset_listing_menu_item(char *text, kest_preset *preset, kest_ui_page *parent);

void preset_listing_delete_button_cb(lv_event_t *e);
void disappear_preset_listing_delete_button(lv_timer_t *timer);
void menu_item_preset_listing_released_cb(lv_event_t *e);
void menu_item_preset_listing_long_pressed_cb(lv_event_t *e);

int preset_listing_menu_item_refresh_active(struct kest_menu_item *item);
int preset_listing_menu_item_change_name(struct kest_menu_item *item, char *name);

#endif
