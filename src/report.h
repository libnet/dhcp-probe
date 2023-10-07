#ifndef REPORT_H
#define REPORT_H

extern void report_init(int dont_fork, char *logfile_name);
extern char *get_errmsg(void);

#ifdef STDC_HEADERS
extern void report(int, char *, ...);
#else
extern void report(); 
#endif

void close_and_reopen_log_file(char * logfile_name);

#endif /* not REPORT_H */
