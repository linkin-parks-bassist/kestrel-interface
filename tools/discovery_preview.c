/* Desktop navigation proposal. Descriptor loading and matching here are not
 * the production catalogue, pagination or final predicate editor. */
#define init_effect_selector_eff preview_original_init
#define create_effect_selector_ui_eff preview_original_create
#include "../components/ui/kest_effect_select.c"
#undef init_effect_selector_eff
#undef create_effect_selector_ui_eff
#include <strings.h>
#include <stdint.h>

static kest_ui_page *preview_page;
static lv_obj_t *preview_list, *search, *filters;
static bool search_keyboard_open;
static int axis = -1, subset;
static const char *facet;
static const char *axes[] = {"By type", "By instrument", "By genre", "By keyword"};

typedef struct rule {
    int kind, axis, negated; /* 0: ALL, 1: ANY, 2: membership */
    const char *value;
    struct rule *parent, *children, *next;
} rule;
static rule *active_rules, *draft_rules, *editing;
static lv_obj_t *editor;
static int picker_axis;

static string_list *facets(kest_effect_desc *eff, int which)
{
    switch (which) {
    case 0: return &eff->types;
    case 1: return &eff->instruments;
    case 2: return &eff->genres;
    default: return &eff->keywords;
    }
}

static int has_facet(kest_effect_desc *eff, const char *value)
{
    string_list *list = facets(eff, axis);
    for (size_t i = 0; i < list->count; ++i)
        if (!strcmp(list->entries[i], value)) return 1;
    return 0;
}

static int rule_matches(const rule *node, kest_effect_desc *eff)
{
    int result = node->kind == 0;
    if (node->kind == 2) {
        string_list *list = facets(eff, node->axis);
        for (size_t i = 0; i < list->count; ++i)
            if (!strcmp(list->entries[i], node->value)) result = 1;
    } else {
        for (rule *child = node->children; child; child = child->next) {
            int match = rule_matches(child, eff);
            if ((node->kind == 0 && !match) || (node->kind == 1 && match)) {
                result = match;
                break;
            }
        }
    }
    return node->negated ? !result : result;
}

static int contains(const char *text, const char *query)
{
    if (!text) return 0;
    size_t len = strlen(query);
    for (; *text; ++text)
        if (!strncasecmp(text, query, len)) return 1;
    return len == 0;
}

static int matches_rules(kest_effect_desc *eff, const rule *rules)
{
    if (rules && !rule_matches(rules, eff)) return 0;
    if (facet && !has_facet(eff, facet)) return 0;
    const char *query = lv_textarea_get_text(search);
    if (!*query || contains(eff->name, query)) return 1;
    for (int a = 0; a < 4; ++a) {
        string_list *list = facets(eff, a);
        for (size_t i = 0; i < list->count; ++i)
            if (contains(list->entries[i], query)) return 1;
    }
    return 0;
}

static int matches(kest_effect_desc *eff) { return matches_rules(eff, active_rules); }

static void render(void);
static void dismiss_keyboard(lv_event_t *e);
static void select_axis(lv_event_t *e)
{
    axis = (int)(intptr_t)lv_event_get_user_data(e);
    subset = axis < 0;
    facet = NULL;
    render();
}

static void show_facet(const char *value)
{
    facet = value;
    subset = 1;
    render();
}
static void select_facet(lv_event_t *e) { show_facet(lv_event_get_user_data(e)); }

/* Opt-in desktop observation; logging is unsuitable for timing measurements. */
static void trace_row_draw(lv_event_t *e)
{
    lv_obj_t *button = lv_event_get_target(e);
    lv_area_t area;
    lv_obj_get_coords(button, &area);
    printf("PREVIEW_DRAW y=%d..%d name=%s\n", (int)area.y1, (int)area.y2,
           lv_label_get_text(lv_obj_get_child(button, 0)));
}

static lv_obj_t *row_on(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *data)
{
    lv_obj_t *button, *label;
    create_standard_button_click_short(&button, &label, parent,
                                       (char *)text, cb, data);
    if (getenv("KEST_DISCOVERY_TRACE_DRAW"))
        lv_obj_add_event_cb(button, trace_row_draw, LV_EVENT_DRAW_MAIN_BEGIN, NULL);
    return button;
}

static void row(const char *text, lv_event_cb_t cb, void *data)
{
    row_on(preview_list, text, cb, data);
}

static int compare_facets(const void *a, const void *b)
{
    const char *left = *(const char *const *)a, *right = *(const char *const *)b;
    int order = strcasecmp(left, right);
    return order ? order : strcmp(left, right);
}

static void create_virtual_rows(kest_effect_selector_str *, const char **, unsigned);
static void create_picker_rows(lv_obj_t *, const char **, unsigned);

static void facet_rows(lv_obj_t *parent, int which, lv_event_cb_t cb)
{
    size_t count = 0;
    for (kest_effect_desc_pll *p = global_cxt.effects; p; p = p->next)
        count += facets(p->data, which)->count;
    if (!count) {
        lv_obj_t *label = lv_label_create(parent);
        lv_label_set_text(label, "No categories assigned yet");
        return;
    }
    const char **values = malloc(count * sizeof(*values));
    if (!values) abort(); /* An isolated desktop review build. */
    size_t index = 0;
    for (kest_effect_desc_pll *p = global_cxt.effects; p; p = p->next) {
        string_list *list = facets(p->data, which);
        for (size_t i = 0; i < list->count; ++i) values[index++] = list->entries[i];
    }
    qsort(values, count, sizeof(*values), compare_facets);
    size_t unique = 0;
    for (size_t i = 0; i < count; ++i)
        if (!i || strcmp(values[i], values[i - 1]))
            values[unique++] = values[i];
    if (parent == preview_list && getenv("KEST_DISCOVERY_VIRTUAL_ROWS")) {
        create_virtual_rows(NULL, values, unique);
        return; /* The current view owns this pointer array until render(). */
    }
    if (parent != preview_list && getenv("KEST_DISCOVERY_VIRTUAL_ROWS")) {
        create_picker_rows(parent, values, unique);
        return;
    }
    for (size_t i = 0; i < unique; ++i)
        row_on(parent, values[i], cb, (void *)values[i]);
    free(values); /* Row callbacks borrow descriptor strings, not this array. */
}

/* Desktop proposal: bound widgets, while the full-descriptor scan still awaits
 * the production catalogue. The environment switch permits pixel comparisons. */
#define ROW_SLOTS (DISPLAY_VRES / STANDARD_BUTTON_SHORT_HEIGHT + 2)
static kest_effect_selector_button row_slots[ROW_SLOTS];
static kest_effect_desc **row_results;
static const char **facet_results;
static unsigned row_count, slot_count;
static int row_stride, row_first = -1, virtual_rows;

static void refresh_rows(lv_event_t *e)
{
    (void)e;
    if (!virtual_rows) return;
    int first = LV_MAX(0, lv_obj_get_scroll_y(preview_list)) / row_stride;
    if (first) --first;
    if (first == row_first) return;
    row_first = first;
    for (unsigned i = 0; i < slot_count; ++i) {
        unsigned index = first + i;
        kest_effect_selector_button *slot = &row_slots[i];
        if (index >= row_count) {
            lv_obj_add_flag(slot->button, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        slot->eff = facet_results ? NULL : row_results[index];
        slot->name = facet_results ? (char *)facet_results[index] : slot->eff->name;
        lv_label_set_text(slot->label, slot->name);
        lv_obj_set_pos(slot->button, 0, index * row_stride);
        lv_obj_remove_flag(slot->button, LV_OBJ_FLAG_HIDDEN);
    }
}

static void virtual_facet_clicked(lv_event_t *e)
{
    kest_effect_selector_button *slot = lv_event_get_user_data(e);
    show_facet(slot->name);
}

static void create_virtual_rows(kest_effect_selector_str *str, const char **values, unsigned count)
{
    facet_results = values;
    row_count = count;
    if (str) {
        row_results = malloc(count * sizeof(*row_results));
        if (!row_results) abort();
        unsigned index = 0;
        for (kest_effect_selector_button_pll *p = str->buttons; p; p = p->next)
            if (matches(p->data->eff)) row_results[index++] = p->data->eff;
    }
    row_stride = STANDARD_BUTTON_SHORT_HEIGHT + lv_obj_get_style_pad_row(preview_list, LV_PART_MAIN);
    lv_obj_set_layout(preview_list, LV_LAYOUT_NONE);
    lv_obj_t *extent = lv_obj_create(preview_list);
    lv_obj_set_size(extent, 1, count * row_stride - (row_stride - STANDARD_BUTTON_SHORT_HEIGHT));
    lv_obj_set_style_bg_opa(extent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(extent, 0, 0);
    lv_obj_remove_flag(extent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    slot_count = LV_MIN(count, ROW_SLOTS);
    for (unsigned i = 0; i < slot_count; ++i) {
        if (str) {
            init_effect_selector_button_from_effect(&row_slots[i], row_results[i]);
            create_effect_selector_button_ui_eff(&row_slots[i], preview_list);
        } else {
            row_slots[i].name = (char *)values[i];
            create_standard_button_click_short(&row_slots[i].button, &row_slots[i].label,
                preview_list, row_slots[i].name, virtual_facet_clicked, &row_slots[i]);
        }
    }
    row_first = -1;
    virtual_rows = 1;
    refresh_rows(NULL);
}

static void render(void)
{
    virtual_rows = 0;
    free(row_results);
    row_results = NULL;
    free(facet_results);
    facet_results = NULL;
    lv_obj_clean(preview_list);
    lv_obj_set_flex_flow(preview_list, LV_FLEX_FLOW_COLUMN);
    const char *query = lv_textarea_get_text(search);
    if (subset || *query || active_rules) {
        set_panel_text(preview_page, facet ? (char *)facet : "Effects");
        unsigned count = 0;
        kest_effect_selector_str *str = preview_page->data_struct;
        for (kest_effect_selector_button_pll *p = str->buttons; p; p = p->next) {
            if (!matches(p->data->eff)) continue;
            if (getenv("KEST_DISCOVERY_VIRTUAL_ROWS")) { ++count; continue; }
            create_effect_selector_button_ui_eff(p->data, preview_list);
            if (getenv("KEST_DISCOVERY_TRACE_DRAW"))
                lv_obj_add_event_cb(p->data->button, trace_row_draw,
                                    LV_EVENT_DRAW_MAIN_BEGIN, NULL);
            ++count;
        }
        if (count && getenv("KEST_DISCOVERY_VIRTUAL_ROWS")) create_virtual_rows(str, NULL, count);
        if (!count) {
            lv_obj_t *label = lv_label_create(preview_list);
            lv_label_set_text(label, "No matching effects");
        }
    } else if (axis < 0) {
        set_panel_text(preview_page, "Add Effect");
        for (int a = 0; a < 4; ++a) row(axes[a], select_axis, (void *)(intptr_t)a);
        row("All effects", select_axis, (void *)(intptr_t)-1);
    } else {
        set_panel_text(preview_page, (char *)axes[axis]);
        facet_rows(preview_list, axis, select_facet);
    }
    lv_obj_scroll_to_y(preview_list, 0, LV_ANIM_OFF);
    row_first = -1;
    refresh_rows(NULL);
}

static void back(lv_event_t *e)
{
    if (*lv_textarea_get_text(search)) lv_textarea_set_text(search, "");
    else if (facet) { facet = NULL; subset = 0; }
    else if (axis >= 0 || subset) { axis = -1; subset = 0; }
    else { dismiss_keyboard(e); enter_parent_page_cb(e); return; }
    dismiss_keyboard(e);
    render();
}

static void search_changed(lv_event_t *e) { (void)e; render(); }
static void dismiss_keyboard(lv_event_t *e)
{
    (void)e;
    hide_keyboard();
    search_keyboard_open = false;
    lv_obj_set_height(preview_list, DISPLAY_VRES - TOP_PANEL_HEIGHT - 120);
}
static void search_focused(lv_event_t *e)
{
    (void)e;
    if (search_keyboard_open) return;
    search_keyboard_open = true;
    lv_obj_remove_flag(filters, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(search, STANDARD_CONTAINER_WIDTH - 115);
    lv_obj_set_height(preview_list,
        DISPLAY_VRES - TOP_PANEL_HEIGHT - 120 - DISPLAY_VRES / 3);
    lv_obj_remove_event_cb(search, dismiss_keyboard);
    spawn_keyboard(preview_page->screen, search, dismiss_keyboard, NULL,
                   dismiss_keyboard, NULL);
}

static rule *new_rule(int kind, rule *parent)
{
    rule *node = calloc(1, sizeof(*node));
    if (!node) abort(); /* An isolated desktop review build. */
    node->kind = kind;
    node->parent = parent;
    return node;
}

static void free_rule(rule *node)
{
    if (!node) return;
    for (rule *child = node->children, *next; child; child = next) {
        next = child->next;
        free_rule(child);
    }
    free(node);
}

static void append_rule(rule *parent, rule *child)
{
    rule **tail = &parent->children;
    while (*tail) tail = &(*tail)->next;
    *tail = child;
}

static rule *clone_rule(const rule *source, rule *parent)
{
    rule *node = new_rule(source->kind, parent);
    node->axis = source->axis;
    node->value = source->value;
    node->negated = source->negated;
    for (rule *child = source->children; child; child = child->next)
        append_rule(node, clone_rule(child, node));
    return node;
}

static void render_editor(void);
static void update_match_count(void)
{
    unsigned count = 0;
    for (kest_effect_desc_pll *p = global_cxt.effects; p; p = p->next)
        count += matches_rules(p->data, draft_rules);
    lv_label_set_text_fmt(lv_msgbox_get_title(editor), "Filters: %u matches", count);
}

static void edit_group(lv_event_t *e)
{
    editing = lv_event_get_user_data(e);
    render_editor();
}

static void group_mode(lv_event_t *e)
{
    editing->kind = lv_dropdown_get_selected(lv_event_get_target(e));
    update_match_count();
}

static void group_negation(lv_event_t *e)
{
    editing->negated = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    update_match_count();
}

static void toggle_tag(lv_event_t *e)
{
    rule *node = lv_event_get_user_data(e);
    node->negated = !node->negated;
    render_editor();
}

static void remove_rule(lv_event_t *e)
{
    rule *node = lv_event_get_user_data(e);
    rule **link = &node->parent->children;
    while (*link != node) link = &(*link)->next;
    *link = node->next;
    free_rule(node);
    render_editor();
}

static void add_tag_value(const char *value)
{
    rule *node = new_rule(2, editing);
    node->axis = picker_axis;
    node->value = value;
    append_rule(editing, node);
    render_editor();
}

static void add_tag(lv_event_t *e) { add_tag_value(lv_event_get_user_data(e)); }

/* Independent slots keep a popup from rebinding the underlying results. */
static struct {
    lv_obj_t *parent;
    const char **values;
    kest_effect_selector_button slots[ROW_SLOTS];
    unsigned count, used;
    int stride, first;
} picker;

static void refresh_picker(lv_event_t *e)
{
    (void)e;
    int first = LV_MAX(0, lv_obj_get_scroll_y(picker.parent)) / picker.stride;
    first = LV_MAX(0, first - 2); /* Back row plus one row above the viewport. */
    if (first == picker.first) return;
    picker.first = first;
    for (unsigned i = 0; i < picker.used; ++i) {
        unsigned index = first + i;
        kest_effect_selector_button *slot = &picker.slots[i];
        if (index >= picker.count) {
            lv_obj_add_flag(slot->button, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        slot->name = (char *)picker.values[index];
        lv_label_set_text(slot->label, slot->name);
        lv_obj_set_pos(slot->button, 0, (index + 1) * picker.stride);
        lv_obj_remove_flag(slot->button, LV_OBJ_FLAG_HIDDEN);
    }
}

static void picker_deleted(lv_event_t *e)
{
    (void)e;
    lv_obj_remove_event_cb(picker.parent, refresh_picker);
    free(picker.values);
    picker.values = NULL;
    picker.parent = NULL;
}

static void picker_clicked(lv_event_t *e)
{
    kest_effect_selector_button *slot = lv_event_get_user_data(e);
    add_tag_value(slot->name);
}

static void create_picker_rows(lv_obj_t *parent, const char **values, unsigned count)
{
    picker.parent = parent;
    picker.values = values;
    picker.count = count;
    picker.used = LV_MIN(count, ROW_SLOTS);
    picker.stride = STANDARD_BUTTON_SHORT_HEIGHT + lv_obj_get_style_pad_row(parent, LV_PART_MAIN);
    lv_obj_set_layout(parent, LV_LAYOUT_NONE);
    lv_obj_t *extent = lv_obj_create(parent);
    lv_obj_set_size(extent, 1, (count + 1) * picker.stride -
                    (picker.stride - STANDARD_BUTTON_SHORT_HEIGHT));
    lv_obj_set_style_bg_opa(extent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(extent, 0, 0);
    lv_obj_remove_flag(extent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(extent, picker_deleted, LV_EVENT_DELETE, NULL);
    for (unsigned i = 0; i < picker.used; ++i) {
        kest_effect_selector_button *slot = &picker.slots[i];
        slot->name = (char *)values[i];
        create_standard_button_click_short(&slot->button, &slot->label, parent,
                                          slot->name, picker_clicked, slot);
    }
    lv_obj_scroll_to_y(parent, 0, LV_ANIM_OFF);
    picker.first = -1;
    lv_obj_add_event_cb(parent, refresh_picker, LV_EVENT_SCROLL, NULL);
    refresh_picker(NULL);
}

static void clean_editor_content(lv_obj_t *content)
{
    lv_obj_clean(content); /* Extent deletion releases the picker pointer array. */
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_scroll_to_y(content, 0, LV_ANIM_OFF);
}

static void choose_axis(lv_event_t *e)
{
    picker_axis = (int)(intptr_t)lv_event_get_user_data(e);
    lv_obj_t *content = lv_msgbox_get_content(editor);
    clean_editor_content(content);
    row_on(content, "Back to group", edit_group, editing);
    facet_rows(content, picker_axis, add_tag);
}

static void choose_tag(lv_event_t *e)
{
    (void)e;
    lv_obj_t *content = lv_msgbox_get_content(editor);
    clean_editor_content(content);
    row_on(content, "Back to group", edit_group, editing);
    for (int a = 0; a < 4; ++a)
        row_on(content, axes[a], choose_axis, (void *)(intptr_t)a);
}

static void add_group(lv_event_t *e)
{
    (void)e;
    rule *group = new_rule(0, editing);
    append_rule(editing, group);
    editing = group;
    render_editor();
}

static void render_editor(void)
{
    lv_obj_t *content = lv_msgbox_get_content(editor);
    clean_editor_content(content);
    update_match_count();
    if (editing->parent) row_on(content, "Back to parent group", edit_group, editing->parent);
    lv_obj_t *mode = lv_dropdown_create(content);
    lv_obj_set_width(mode, LV_PCT(100));
    lv_dropdown_set_options(mode, "Match all\nMatch any");
    lv_dropdown_set_selected(mode, editing->kind);
    lv_obj_add_event_cb(mode, group_mode, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_t *negate = lv_checkbox_create(content);
    lv_checkbox_set_text(negate, "NOT this group");
    if (editing->negated) lv_obj_add_state(negate, LV_STATE_CHECKED);
    lv_obj_add_event_cb(negate, group_negation, LV_EVENT_VALUE_CHANGED, NULL);
    row_on(content, "Add filter", choose_tag, NULL);
    row_on(content, "Add group", add_group, NULL);
    if (!editing->children) {
        lv_obj_t *hint = lv_label_create(content);
        lv_label_set_text(hint, "Empty ALL matches everything; empty ANY matches nothing.");
        lv_obj_set_width(hint, LV_PCT(100));
    }
    for (rule *child = editing->children; child; child = child->next) {
        lv_obj_t *line = lv_obj_create(content);
        lv_obj_set_size(line, LV_PCT(100), STANDARD_BUTTON_SHORT_HEIGHT + 48);
        lv_obj_remove_flag(line, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_flex_flow(line, LV_FLEX_FLOW_ROW);
        char text[256];
        if (child->kind == 2)
            snprintf(text, sizeof(text), "%s%s: %s", child->negated ? "NOT " : "",
                     axes[child->axis] + 3, child->value);
        else
            snprintf(text, sizeof(text), "%sGroup: match %s", child->negated ? "NOT " : "",
                     child->kind == 0 ? "all" : "any");
        lv_obj_t *button = row_on(line, text, child->kind == 2 ? toggle_tag : edit_group, child);
        lv_obj_set_width(button, LV_PCT(80));
        button = row_on(line, LV_SYMBOL_CLOSE, remove_rule, child);
        lv_obj_set_width(button, 42);
    }
}

static void editor_deleted(lv_event_t *e)
{
    (void)e;
    free_rule(draft_rules);
    draft_rules = editing = NULL;
    editor = NULL;
}

static void cancel_filters(lv_event_t *e) { (void)e; lv_msgbox_close(editor); }
static void clear_filters(lv_event_t *e)
{
    (void)e;
    free_rule(draft_rules);
    draft_rules = editing = new_rule(0, NULL);
    render_editor();
}

static void apply_filters(lv_event_t *e)
{
    (void)e;
    free_rule(active_rules);
    active_rules = draft_rules;
    draft_rules = NULL;
    if (active_rules->kind == 0 && !active_rules->children && !active_rules->negated) {
        free_rule(active_rules);
        active_rules = NULL;
    }
    lv_msgbox_close(editor);
    render();
}

static void filter_dialog(lv_event_t *e)
{
    dismiss_keyboard(e);
    draft_rules = active_rules ? clone_rule(active_rules, NULL) : new_rule(0, NULL);
    editing = draft_rules;
    editor = lv_msgbox_create(NULL);
    lv_obj_set_size(editor, LV_PCT(90), LV_PCT(80));
    lv_msgbox_add_title(editor, "Filters (preview)");
    lv_msgbox_add_close_button(editor);
    lv_obj_add_event_cb(editor, editor_deleted, LV_EVENT_DELETE, NULL);
    const char *labels[] = {"Apply", "Cancel", "Clear"};
    lv_event_cb_t callbacks[] = {apply_filters, cancel_filters, clear_filters};
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *button = lv_msgbox_add_footer_button(editor, labels[i]);
        lv_obj_set_height(lv_obj_get_parent(button), STANDARD_BUTTON_SHORT_HEIGHT + 24);
        lv_obj_set_size(button, LV_PCT(30), STANDARD_BUTTON_SHORT_HEIGHT);
        lv_obj_add_event_cb(button, callbacks[i], LV_EVENT_CLICKED, NULL);
    }
    render_editor();
}

int create_effect_selector_ui_eff(kest_ui_page *page)
{
    if (page->ui_created) return NO_ERROR;
    preview_page = page;
    page->screen = lv_obj_create(NULL);
    create_panel_with_back_button(page);
    lv_obj_remove_event_cb(page->panel->left_button, enter_parent_page_cb);
    lv_obj_add_event_cb(page->panel->left_button, back, LV_EVENT_CLICKED, page);
    create_standard_button_list_tall(&preview_list, page->screen);
    lv_obj_add_event_cb(preview_list, refresh_rows, LV_EVENT_SCROLL, NULL);
    lv_obj_set_align(preview_list, LV_ALIGN_TOP_LEFT);
    lv_obj_set_pos(preview_list, (DISPLAY_HRES - STANDARD_CONTAINER_WIDTH) / 2,
                   TOP_PANEL_HEIGHT + 80);
    lv_obj_set_height(preview_list, DISPLAY_VRES - TOP_PANEL_HEIGHT - 120);
    search = lv_textarea_create(page->screen);
    lv_obj_set_pos(search, (DISPLAY_HRES - STANDARD_CONTAINER_WIDTH) / 2,
                   TOP_PANEL_HEIGHT + 20);
    lv_textarea_set_one_line(search, true);
    lv_obj_set_width(search, STANDARD_CONTAINER_WIDTH);
    lv_textarea_set_placeholder_text(search, "Search effects");
    lv_obj_add_event_cb(search, search_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(search, search_focused, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(search, search_focused, LV_EVENT_CLICKED, NULL);
    filters = lv_button_create(page->screen);
    lv_obj_set_pos(filters, (DISPLAY_HRES + STANDARD_CONTAINER_WIDTH) / 2 - 105,
                   TOP_PANEL_HEIGHT + 20);
    lv_obj_update_layout(search);
    lv_obj_set_size(filters, 105, lv_obj_get_height(search));
    lv_obj_t *label = lv_label_create(filters);
    lv_label_set_text(label, "Filters");
    lv_obj_center(label);
    lv_obj_add_event_cb(filters, filter_dialog, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(filters, LV_OBJ_FLAG_HIDDEN);
    page->ui_created = 1;
    render();
    return NO_ERROR;
}

int init_effect_selector_eff(kest_ui_page *page)
{
    int result = preview_original_init(page);
    if (result == NO_ERROR) page->create_ui = create_effect_selector_ui_eff;
    return result;
}
