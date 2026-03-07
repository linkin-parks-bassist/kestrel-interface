#ifndef MY_PRINTF_H_
#define MY_PRINTF_H_

#define M_ENABLE_PRINTF

void m_printf_init();
void m_printf(const char *fmt, ...);
void m_printf_cont(const char *fmt, ...);

#define M_PRINTF(...) if (PRINTLINES_ALLOWED) {m_printf("%s:%03d: ", FNAME, __LINE__);m_printf(__VA_ARGS__);};
#define M_PRINTF_(...) if (PRINTLINES_ALLOWED) {m_printf(__VA_ARGS__);};

#endif
