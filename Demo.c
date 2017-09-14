#include <stdio.h>
#include <string.h>
#include "CalendarParser.h"

int main(int argc, char *argv[]) {
	char *toPrint;
	const char *errPrint;
	Calendar *cal;
	ErrorCode err;

	err = createCalendar(argv[1], &cal);
	if(err == OK) {
		toPrint = printCalendar(cal);
		printf("%s\n", toPrint);
		free(toPrint);
		deleteCalendar(cal);
	} else {
		errPrint = printError(err);
		printf("%s\n", errPrint);
		free((char*)errPrint);
	}

	return 0;
}
