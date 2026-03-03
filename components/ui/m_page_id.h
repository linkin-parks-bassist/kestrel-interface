#ifndef M_PAGE_ID_H_
#define M_PAGE_ID_H_

typedef struct {
	int  type;
	char fname[32];
	int  id;
} m_page_identifier;

struct m_context;
m_ui_page *m_page_id_find_page(struct m_context *cxt, m_page_identifier pid);

int m_ui_page_create_identifier(m_ui_page *page, m_page_identifier *id);

#endif
