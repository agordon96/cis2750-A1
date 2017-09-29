#ifndef CALENDARPARSER_H
#define CALENDARPARSER_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LinkedListAPI.h"

//Error codes that indicate what went wrong during parsing
typedef enum ers {OK, INV_FILE, INV_CAL, INV_VER, DUP_VER, INV_PRODID, DUP_PRODID, INV_EVENT, INV_CREATEDT, OTHER_ERROR} ErrorCode;

//Represents iCalendar Date-time
typedef struct dt {
	//YYYYMMDD
	char date[9]; 
	//hhmmss
	char time[7]; 
	//indicates whether this is UTC time
	bool	UTC;  
} DateTime;

//Represents a generic iCalendar property
typedef struct prop {
	//Property name.  We will assume that the property name, even if malformed, does not exceed 200 bytes
	char 	propName[200]; 
	//Property description.  We use a C99 flexible array member, which we will discuss in class.
	char	propDescr[]; 
} Property;

//Represents an iCalendar alarm component
typedef struct alarm {
	//Alarm action.  We will assume that the action, even if malformed, does not exceed 1000 bytes
    char    action[200];
	//Alarm trigger.
    char*   trigger;
	//Additional alarm properties.  All objects in the list will be of type Property.  It may be empty.
    List    properties;
} Alarm;

//Represents an iCalendar event component
typedef struct evt {
	//Event user ID.  We will assume that the UserID, even if malformed, does not exceed 1000 bytes
	char 		UID[1000];
	//Alarm creation date-time.
    DateTime 	creationDateTime;
	//Additional event properties.  All objects in the list will be of type Property.  It may be empty.
	List 	    properties;
	//List of alarms associated with the event.  All objects in the list will be of type Alarm.  It may be empty.
    List        alarms;
	
} Event;

//Represents an iCalendar object
typedef struct ical {
	//iCalendar version
	float 	version;
	//Product ID.  We will assume that the UserID, even if malformed, does not exceed 1000 bytes
	char 	prodID[1000];
	//Reference to an event.  We will assume that every calendar object will have an event.
	Event* event;
} Calendar;




/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or 
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the 
		appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ErrorCode createCalendar(char* fileName, Calendar** obj);


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj);


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj); 


/** Function to "convert" the ErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into 
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
const char* printError(ErrorCode err);   

#endif	
