#ifndef TOK_H_
#define TOK_H_

int char_is_letter(char c);

#define TOKENIZER_STATE_IDLE			0
#define TOKENIZER_STATE_NAME			1
#define TOKENIZER_STATE_NUMBER			2
#define TOKENIZER_STATE_NUMBER_HEX		3
#define TOKENIZER_STATE_NUMBER_BIN		4
#define TOKENIZER_STATE_STRING			5
#define TOKENIZER_STATE_STRESC			6
#define TOKENIZER_STATE_LEADING_ZERO	7
#define TOKENIZER_STATE_DONE			8

#define TOKENIZER_POLICY_DISCARD   		0
#define TOKENIZER_POLICY_ACCEPT	   		1
#define TOKENIZER_POLICY_SINGULAR  		2
#define TOKENIZER_POLICY_BEGIN	   		3
#define TOKENIZER_POLICY_END_ACCEPT   	4
#define TOKENIZER_POLICY_END_DISCARD 	5
#define TOKENIZER_POLICY_COMPLAIN		6

typedef struct kest_token_ll {
	char *data;
	int line;
	int index;
	struct kest_token_ll *next;
} kest_token_ll;

struct kest_eff_parsing_state;

int kest_tokenize_content(struct kest_eff_parsing_state *ps);

int kest_token_ll_safe_aappend(kest_token_ll **list_ptr, char *x, int line, int index);

int kest_token_ll_advance(kest_token_ll **list);
int kest_token_ll_skip_ws(kest_token_ll **list);

int token_is_newline(char *str);
int token_is_char(char *str, char c);

int token_is_int(char *token);
int token_is_number(char *token);
int token_is_name(char *token);
int token_is_dict_entry_seperator(char *token);

float token_to_float(char *token);

kest_token_ll *kest_token_span_to_ll(kest_token_ll *start, kest_token_ll *end);

#endif
