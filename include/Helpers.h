#include "CalendarParser.h"
#include <ctype.h>

char* printFuncProp(void *toBePrinted);
char* printFuncAlarm(void *toBePrinted);
int compareFuncProp(const void *first, const void *second);
int compareFuncAlarm(const void *first, const void *second);
void deleteFuncProp(void *toBeDeleted);
void deleteFuncAlarm(void *toBeDeleted);
ErrorCode badError(Calendar *cal, FILE *file, Calendar **obj, ErrorCode err);
void clearSpaces(char *toClear);
