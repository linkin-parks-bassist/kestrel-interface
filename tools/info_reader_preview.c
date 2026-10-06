/* Isolated host experiment: reuse INFO grammar/validation without extracting
 * parameters/resources or compiling code. This is not a production reader API. */
#include "../components/parser/kest_eff_parser.c"
#define main full_compiler_main
#include "compile_eff.c"
#undef main

static int read_info(const char *path)
{
    kest_eff_parsing_state ps = {0};
    init_parsing_state(&ps);
    ps.fname = path;
    kest_eff_desc_file_section section = {0};
    kest_ast_node info = {.type = KEST_AST_NODE_SECTION, .data = &section};
    kest_expression_ptr_list expressions;
    if (kest_expression_capture_begin(&expressions) != NO_ERROR) return 1;
    FILE *file = fopen(path, "rb");
    int result = ERR_BAD_ARGS;
    if (!file) goto done;
    if (fseek(file, 0, SEEK_END)) goto done;
    long size = ftell(file);
    if (size < 0 || size > INT_MAX || fseek(file, 0, SEEK_SET)) goto done;
    ps.file_size = size;
    ps.content = kest_parser_alloc(size + 1);
    if (!ps.content || fread(ps.content, 1, size, file) != (size_t)size) goto done;
    ps.content[size] = 0;
    if (kest_tokenize_content(&ps) != NO_ERROR || ps.errors) goto done;
    if (kest_parser_lineize_content(&ps) != NO_ERROR) goto done;
    kest_token_ll *start = NULL, *end = NULL, *previous = NULL;
    int score = 0;
    for (kest_token_ll *token = ps.tokens; token; token = token->next) {
        int next = get_section_start_score(token->data, score);
        if (score == 2) {
            if (next != 3) goto done;
            if (start && !end) end = previous;
            if (!strcmp(token->data, "INFO")) {
                start = token->next;
                end = NULL;
                info.line = token->line;
            }
            score = 0;
        } else score = next;
        previous = token;
    }
    if (!start) goto done;
    section.name = "INFO";
    section.tokens = kest_token_span_to_ll(start, end);
    if (!section.tokens || kest_parse_entry_section(&ps, &info) != NO_ERROR ||
        kest_validate_discovery_info(&ps, &info) != NO_ERROR || ps.errors) goto done;
    kest_eff_entry *name = kest_eff_section_lookup(&info, "name");
    kest_eff_entry *cname = kest_eff_section_lookup(&info, "cname");
    if (!name || name->type != KEST_EFF_ENTRY_TYPE_STR ||
        (cname && cname->type != KEST_EFF_ENTRY_TYPE_STR)) goto done;
    char *name_text = kest_parser_strndup(name->value.val_string, 128);
    char *cname_text = cname ? kest_parser_strndup(cname->value.val_string, 128) :
                              kest_cname_from_name(name_text);
    if (!name_text || !cname_text) goto done;
    print_info_text("name", 0, name_text);
    print_info_text("cname", 0, cname_text);
    kest_eff_entry *description = kest_eff_section_lookup(&info, "description");
    if (description) print_info_text("description", 0, description->value.val_string);
    for (unsigned i = 0; i < 4; ++i) {
        kest_eff_entry *entry = kest_eff_section_lookup(&info, discovery_list_names[i]);
        size_t count = entry ? entry->value.val_list->count : 0;
        printf("KEST eff-info field=%s count=%u\n", discovery_list_names[i], (unsigned)count);
        for (size_t j = 0; j < count; ++j)
            print_info_text(discovery_list_names[i], j, entry->value.val_list->entries[j].value.val_string);
    }
    puts("KEST eff-info result=0");
    result = NO_ERROR;
done:
    if (file) fclose(file);
    fprintf(stderr, "INFO preview result=%d arena=%zu descriptors=%zu expressions=%zu parameters=%zu resources=%zu\n",
            result, kest_eff_parser_mempool.pos,
            kest_effect_desc_mem_pool.size - kest_effect_desc_mem_pool.free_count,
            kest_expression_mem_pool.size - kest_expression_mem_pool.free_count,
            kest_parameter_mem_pool.size - kest_parameter_mem_pool.free_count,
            kest_dsp_resource_mem_pool.size - kest_dsp_resource_mem_pool.free_count);
    for (size_t i = 0; i < section.dict_.count; ++i)
        kest_free_parsed_eff_entry(&section.dict_.entries[i]);
    kest_expression_capture_end();
    kest_expression_capture_destroy(&expressions);
    kest_eff_parser_reset_mempool();
    return result != NO_ERROR || ferror(stdout);
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "Usage: %s INPUT.eff ...\n", argv[0]); return 2; }
    if (kest_mem_init() != NO_ERROR || kest_eff_parser_init_mempool() != NO_ERROR) return 1;
    int result = 0;
    for (int i = 1; i < argc; ++i) result |= read_info(argv[i]);
    kest_eff_parser_deinit_mempool();
    return result;
}
