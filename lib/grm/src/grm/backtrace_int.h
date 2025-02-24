#ifndef BACKTRACE_INT_H_INCLUDED
#define BACKTRACE_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### internal interface ##################################################################### */

/* ========================= functions ============================================================================== */

void installBacktraceHandler(void);
void installBacktraceHandlerIfEnabled(void);
void uninstallBacktraceHandler(void);
void uninstallBacktraceHandlerIfEnabled(void);
int backtraceEnabled(void);

#ifdef __cplusplus
}
#endif
#endif /* ifndef BACKTRACE_INT_H_INCLUDED */
