#ifndef KEST_PAGE_ID_H_
#define KEST_PAGE_ID_H_

typedef struct {
	int  type;
	char fname[32];
	int  id;
} kest_page_identifier;

struct kest_context;
kest_ui_page *kest_page_id_find_page(struct kest_context *cxt, kest_page_identifier pid);

int kest_ui_page_create_identifier(kest_ui_page *page, kest_page_identifier *id);

#endif
