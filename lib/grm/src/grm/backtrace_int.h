#ifndef BACKTRACE_INT_H_INCLUDED
#define BACKTRACE_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### internal interface ##################################################################### */

/* ========================= functions ============================================================================== */

void install_backtrace_handler(void);
void install_backtrace_handler_if_enabled(void);
void uninstall_backtrace_handler(void);
void uninstall_backtrace_handler_if_enabled(void);
int backtrace_enabled(void);

#ifdef __cplusplus
}
#endif
#endif /* ifndef BACKTRACE_INT_H_INCLUDED */
