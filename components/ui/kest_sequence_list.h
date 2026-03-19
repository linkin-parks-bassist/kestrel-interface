#ifndef KEST_INT_SEQUENCE_LIST_H_
#define KEST_INT_SEQUENCE_LIST_H_

int init_sequence_list(kest_ui_page *page);
int configure_sequence_list(kest_ui_page *page, void *data);
int create_sequence_list_ui(kest_ui_page *page);
int free_sequence_list_ui(kest_ui_page *page);
int enter_sequence_list(kest_ui_page *page);
int refresh_sequence_list(kest_ui_page *page);

struct kest_menu_item *create_sequence_listing_menu_item(char *text, kest_sequence *sequence, kest_ui_page *parent);

void sequence_listing_delete_button_cb(lv_event_t *e);
void disappear_sequence_listing_delete_button(lv_timer_t *timer);
void menu_item_sequence_listing_released_cb(lv_event_t *e);
void menu_item_sequence_listing_long_pressed_cb(lv_event_t *e);

int sequence_listing_menu_item_refresh_active(struct kest_menu_item *item);
int sequence_listing_menu_item_change_name(struct kest_menu_item *item, char *name);

#endif
