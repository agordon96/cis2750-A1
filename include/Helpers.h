#include <ctype.h>

char* printFuncProp(void *toBePrinted);
char* printFuncAlarm(void *toBePrinted);
int compareFuncProp(const void *first, const void *second);
int compareFuncAlarm(const void *first, const void *second);
void deleteFuncProp(void *toBeDeleted);
void deleteFuncAlarm(void *toBeDeleted);
void clearSpaces(char *toClear);
