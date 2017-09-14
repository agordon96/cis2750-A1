Aaron Gordon 0884023

This was done under the following assumptions:
- The strings fetched from the printCalendar and printError functions will be freed in the main code
- The calendar will be destroyed if it returns OK since it is the main parameter for the deleteCalendar function
- That CalendarParser.h is included in the main file
- That, if there are multiple events, they are ignored as this was not specified as an error and wouldn't be in a normal iCal parser

Everything *should* work and there should be no memory leaks.
