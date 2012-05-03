#ifndef MOLDYN_UTILITIES_H
#define MOLDYN_UTILITIES_H

extern char *program_name;

void moldyn_log(const char *log_message);
void moldyn_error(const char *error_message);
void moldyn_exit(int error_code);
void moldyn_usage(void);
#endif
