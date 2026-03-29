#ifndef MY_PRINTF_H_
#define MY_PRINTF_H_

#define KEST_ENABLE_PRINTF

void kest_printf_init();
void kest_printf(const char *fmt, ...);
void kest_printf_cont(const char *fmt, ...);

void kest_puts(kest_string str);

#define KEST_PRINTF(...) do {if (PRINTLINES_ALLOWED) {kest_printf("%s:%03d: ", FNAME, __LINE__);kest_printf(__VA_ARGS__);};} while (0);
#define KEST_PRINTF_(...) do {if (PRINTLINES_ALLOWED) {kest_printf(__VA_ARGS__);};} while (0);
#define KEST_PRINTF_FORCE(...) do {kest_printf(__VA_ARGS__);} while (0);

#define KEST_PUTS(str) do {if (PRINTLINES_ALLOWED) {kest_printf("%s:%03d: ", FNAME, __LINE__);kest_puts(str);};} while (0);
#define KEST_PUTS_(str) do {if (PRINTLINES_ALLOWED) {kest_puts(str);};} while (0);
#define KEST_PUTS_FORCE(str) do {kest_puts(str);} while (0);

#endif
