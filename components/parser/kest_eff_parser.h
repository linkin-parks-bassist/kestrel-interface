#ifndef KEST_PARSER_H_
#define KEST_PARSER_H_

#define KEST_EFF_PARSER_MEM_POOL_SIZE_KB 512

#define KEST_AST_NODE_ROOT		0
#define KEST_AST_NODE_SECTION	1
#define KEST_AST_NODE_DICT		2

typedef struct kest_ast_node {
	int type;
	int line;
	void *data;
	
	struct kest_ast_node *child;
	struct kest_ast_node *next;
} kest_ast_node;

struct kest_asm_line_pll;

typedef struct kest_eff_parsing_state {
	const char *fname;
	const char *name;
	const char *cname;
	char *content;
	int file_size;
	char **lines;
	
	int len;
	int n_lines;
	int current_line;
	
	char *version_string;
	
	kest_effect_desc *result;
	kest_token_ll *tokens;
	
	kest_token_ll *current_token;
	
	struct kest_asm_line_pll 	*asm_lines;
	kest_block_pll 				*blocks;
	kest_parameter_pll 			*parameters;
	kest_setting_pll 			*settings;
	kest_dsp_resource_pll 		*resources;
	kest_named_expression_pll 	*def_exprs;
	
	kest_driver_list drivers;
	
	kest_scope *scope;
	
	kest_ast_node *ast;
	
	int errors;
} kest_eff_parsing_state;

extern const char *ver_str;

struct kest_dictionary;

int kest_parse_dictionary(kest_eff_parsing_state *ps, kest_dictionary **result, const char *name);

kest_effect_desc *kest_read_eff_desc_from_file(char *fname);

void kest_parser_print_info   (kest_eff_parsing_state *ps, const char *error_msg, ...);
void kest_parser_warn(kest_eff_parsing_state *ps, const char *error_msg, ...);
void kest_parser_error  (kest_eff_parsing_state *ps, const char *error_msg, ...);

void kest_parser_print_info_at  (kest_eff_parsing_state *ps, kest_token_ll *token, const char *error_msg, ...);
void kest_parser_warn_at		(kest_eff_parsing_state *ps, kest_token_ll *token, const char *error_msg, ...);
void kest_parser_error_at  		(kest_eff_parsing_state *ps, kest_token_ll *token, const char *error_msg, ...);

void kest_parser_print_info_at_line(kest_eff_parsing_state *ps, int line, const char *error_msg, ...);
void kest_parser_warn_at_line		(kest_eff_parsing_state *ps, int line, const char *error_msg, ...);
void kest_parser_error_at_line  	(kest_eff_parsing_state *ps, int line, const char *error_msg, ...);

void kest_parser_print_info_at_node(kest_eff_parsing_state *ps, kest_ast_node *node, const char *error_msg, ...);
void kest_parser_warn_at_node		(kest_eff_parsing_state *ps, kest_ast_node *node, const char *error_msg, ...);
void kest_parser_error_at_node  	(kest_eff_parsing_state *ps, kest_ast_node *node, const char *error_msg, ...);

int kest_eff_parser_init_mempool();
int kest_eff_parser_reset_mempool();
int kest_eff_parser_deinit_mempool();

void *kest_parser_alloc(size_t size);
char *kest_parser_strndup(const char *str, int n);

extern kest_allocator *kest_parser_allocator;

#endif
