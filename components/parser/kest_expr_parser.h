#ifndef KEST_PARSE_EXPR_H_
#define KEST_PARSE_EXPR_H_

#define SUMSUB_PRECEDENCE 10
#define MULDIV_PRECEDENCE 20
#define  UNARY_PRECEDENCE 30
#define  POWER_PRECEDENCE 40

kest_expression *kest_parse_expression(kest_eff_parsing_state *ps, kest_token_ll *tokens, kest_token_ll *tokens_end);

#endif
