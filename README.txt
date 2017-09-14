Aaron Gordon 0884023

This was done under the assumption that the strings fetched from the printCalendar
and printError functions will be freed in the main code as they are mallocated in
the .c file. It is also assumed that the calendar will be destroyed if it returns
OK since it is the main parameter for the deleteCalendar function. It is also
assumed that CalendarParser.h is included in the main file.

Everything *should* work and there should be no memory leaks.
