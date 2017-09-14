#include "CalendarParser.h"
#include "Helpers.h"

char* printFuncProp(void *toBePrinted) {
  char *toReturn;
  Property *prop;

  if(!toBePrinted) {
    return NULL;
  }

  prop = (Property*)toBePrinted;
  toReturn = (char*)malloc(sizeof(char) * (strlen(prop->propName) + strlen(prop->propDescr) + 7));
  sprintf(toReturn, "\t\t- %s:%s\n", prop->propName, prop->propDescr);

	return toReturn;
}

char* printFuncAlarm(void *toBePrinted) {
  char *toReturn;
  char *temp;
  int len;
  Alarm *alarm;
  Node *node;

  if(!toBePrinted) {
    return NULL;
  }

  alarm = (Alarm*)toBePrinted;
  toReturn = (char*)malloc(sizeof(char) * (strlen(alarm->action) + strlen(alarm->trigger) + 38));
  sprintf(toReturn, "\t\tAction: %s\n\t\tTrigger: %s\n\t\tProperties:\n", alarm->action, alarm->trigger);
  len = strlen(toReturn);

  if(&alarm->properties) {
    node = alarm->properties.head;
    while(node) {
      temp = alarm->properties.printData(node->data);
      toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
      strcat(toReturn, temp);
      len = strlen(toReturn);

      node = node->next;
      free(temp);
    }
  }

	return toReturn;
}

int compareFuncProp(const void *first, const void *second) {
  Property *prop1;
	Property *prop2;

	if(!first || !second) {
		return 0;
	}

	prop1 = (Property*)first;
	prop2 = (Property*)second;

	return strcmp((char*)prop1->propName, (char*)prop2->propName);
}

int compareFuncAlarm(const void *first, const void *second) {
  Alarm *alarm1;
	Alarm *alarm2;

	if(!first || !second) {
		return 0;
	}

	alarm1 = (Alarm*)first;
	alarm2 = (Alarm*)second;

	return strcmp((char*)alarm1->action, (char*)alarm2->action);
}

void deleteFuncProp(void *toBeDeleted) {
  Property *prop;

	if(!toBeDeleted) {
    return;
  }

  prop = (Property*)toBeDeleted;
  free(prop);
}

void deleteFuncAlarm(void *toBeDeleted) {
	Alarm *alarm;

  if(!toBeDeleted) {
    return;
  }

  alarm = (Alarm*)toBeDeleted;
  clearList(&alarm->properties);

  if(alarm->trigger) {
    free(alarm->trigger);
  }

  free(alarm);
}

void clearSpaces(char *toClear) {
  const char *firstChar;
  char *endChar;
  size_t lineLen;

  firstChar = toClear;
  while(*firstChar != '\0' && isspace(*firstChar)) {
    ++firstChar;
  }

  lineLen = strlen(firstChar) + 1;
  memmove(toClear, firstChar, lineLen);

  endChar = toClear + lineLen;
  while(toClear < endChar && isspace(*endChar)) {
    --endChar;
  }

  *endChar = '\0';
}

ErrorCode createCalendar(char* fileName, Calendar** obj) {
  char *token;
  char *tempFile;
  char line[256];
  int isDtStart = 0;
  int isDtEnded = 0;
  int calEnded = 0;
  Calendar *cal = (Calendar*)malloc(sizeof(Calendar));
  FILE *file;

  cal->version = 0.0;
  strcpy(cal->prodID, "");
  cal->event = NULL;

  if(!obj) {
    free(cal);
    obj = NULL;
    return INV_CAL;
  } else if(!fileName || strcmp(fileName, "") == 0) {
    free(cal);
    obj = NULL;
    return INV_FILE;
  }

  tempFile = (char*)malloc(sizeof(char) * (strlen(fileName) + 1));
  strcpy(tempFile, fileName);

  token = strtok(tempFile, ".");
  if(token != NULL) {
    token = strtok(NULL, ".");
  }

  if(!token || strcmp(token, "ics") != 0) {
    free(cal);
    free(tempFile);
    obj = NULL;
    return INV_FILE;
  }

  free(tempFile);
  file = fopen(fileName, "r");
  if(!file) {
    free(cal);
    obj = NULL;
    return INV_FILE;
  }

  clearSpaces(line);
  if(!fgets(line, sizeof(line), file)) {
    free(cal);
    obj = NULL;
    return INV_FILE;
  } else if(strcmp(line, "BEGIN:VCALENDAR") != 0) {
    free(cal);
    obj = NULL;
    return INV_CAL;
  }

  while(fgets(line, sizeof(line), file)) {
    clearSpaces(line);

    token = strtok(line, ":;");
    if(!token) {
      free(cal);
      obj = NULL;
      return INV_CAL;
    }

    if(strcmp(token, "VERSION") == 0) {
      if(cal->version != 0.0) {
        free(cal);
        obj = NULL;
        return DUP_VER;
      }

      token = strtok(NULL, "");
      cal->version = atof(token);

      if(cal->version == 0.0) {
        free(cal);
        obj = NULL;
        return INV_VER;
      }
    } else if(strcmp(token, "PRODID") == 0) {
      if(strcmp(cal->prodID, "") == 0) {
        free(cal);
        obj = NULL;
        return DUP_PRODID;
      }

      token = strtok(NULL, "");
      if(!strcpy(cal->prodID, token)) {
        free(cal);
        obj = NULL;
        return INV_PRODID;
      }
    } else if(strcmp(token, "BEGIN") == 0) {
      token = strtok(NULL, "");
      if(strcmp(token, "VEVENT") != 0) {
        free(cal);
        obj = NULL;
        return INV_EVENT;
      }

      int eventEnded = 0;
      cal->event = (Event*)malloc(sizeof(Event));
      strcpy(cal->event->UID, "");
      cal->event->properties.head = NULL;
      cal->event->alarms.head = NULL;

      while(!eventEnded || fgets(line, sizeof(line), file)) {
        int inAlarm = 0;
        Alarm *alarm;
        Property *prop;

        clearSpaces(line);
        token = strtok(line, ":;");

        if(strcmp(token, "END") == 0) {
          token = strtok(NULL, "");
          if(strcmp(token, "VEVENT") == 0 && !inAlarm) {
            if(strcmp(cal->event->UID, "") == 0 || !&cal->event->creationDateTime) {
              free(cal);
              obj = NULL;
              return INV_EVENT;
            }
          } else if(strcmp(token, "VALARM") == 0 && inAlarm) {
            if(!alarm || strcmp(alarm->action, "") == 0 || !alarm->trigger) {
              free(cal);
              obj = NULL;
              return INV_EVENT;
            }

            if(!cal->event->alarms.head) {
              cal->event->alarms = initializeList(&printFuncAlarm, &deleteFuncAlarm, &compareFuncAlarm);
            }

            insertBack(&cal->event->alarms, (void*)alarm);
          } else {
            free(cal);
            obj = NULL;
            return INV_EVENT;
          }
        } else if(strcmp(token, "BEGIN") == 0) {
          token = strtok(NULL, "");
          if(strcmp(token, "VALARM") != 0 || inAlarm) {
            free(cal);
            obj = NULL;
            return INV_EVENT;
          }

          inAlarm = 1;
          alarm = (Alarm*)malloc(sizeof(Alarm));
          strcpy(alarm->action, "");
          alarm->trigger = NULL;
          alarm->properties.head = NULL;
        } else {
          if(inAlarm) {
            if(strcmp(token, "TRIGGER") == 0) {
              token = strtok(NULL, "");
              alarm->trigger = (char*)malloc(sizeof(char) * strlen(token) + 1);
              strcpy(alarm->trigger, token);
            } else if(strcmp(token, "ACTION") == 0) {
              token = strtok(NULL, "");
              strcpy(alarm->action, token);
            } else {
              if(!alarm->properties.head) {
                alarm->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
              }

              prop = (Property*)malloc(sizeof(Property));
              strcpy(prop->propName, token);
              token = strtok(NULL, "");
              prop = realloc(prop, sizeof(Property) + (sizeof(char) * strlen(token) + 1));
              strcpy(prop->propDescr, token);
              insertBack(&alarm->properties, (void*)prop);
            }
          } else if(strcmp(token, "DTSTAMP") == 0) {
            DateTime dt;

            if(&cal->event->creationDateTime) {
              free(cal);
              obj = NULL;
              return INV_CREATEDT;
            }

            token = strtok(NULL, "T");
            if(!token || strlen(token) != 9) {
              free(cal);
              obj = NULL;
              return INV_CREATEDT;
            }

            strcpy(dt.date, token);
            token = strtok(NULL, "T");
            if(!token || strlen(token) != 7) {
              free(cal);
              obj = NULL;
              return INV_CREATEDT;
            }

            dt.UTC = 0;
            if(token[6] == 'Z') {
              dt.UTC = 1;
            }

            token[6] = '\0';
            strcpy(dt.time, token);
            cal->event->creationDateTime = dt;
          } else {
            if(strcmp(token, "DTSTART") == 0 && !isDtStart) {
              isDtStart = 1;
            } else if((strcmp(token, "DTSTART") == 0 && isDtStart)
              || ((strcmp(token, "DTEND") == 0 || strcmp(token, "DURATION") == 0) && (!isDtStart || isDtEnded))) {
              free(cal);
              obj = NULL;
              return INV_EVENT;
            } else if(strcmp(token, "DTEND") == 0 || strcmp(token, "DURATION") == 0) {
              isDtEnded = 1;
            }

            if(!&cal->event->properties) {
              cal->event->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
            }

            prop = (Property*)malloc(sizeof(Property));
            strcpy(prop->propName, token);
            token = strtok(NULL, "");
            prop = realloc(prop, sizeof(Property) + (sizeof(char) * strlen(token) + 1));
            strcpy(prop->propDescr, token);
            insertBack(&cal->event->properties, (void*)prop);
          }
        }
      }

      if(!eventEnded || !cal->event->UID || (isDtStart && !isDtEnded)) {
        free(cal);
        obj = NULL;
        return INV_EVENT;
      }
    } else if(strcmp(token, "END") == 0) {
      token = strtok(NULL, "");
      if(strcmp(token, "VCALENDAR") != 0) {
        free(cal);
        obj = NULL;
        return INV_CAL;
      }

      calEnded = 1;
    }
  }

  if(!cal->version || strcmp(cal->prodID, "") == 0 || !cal->event || !calEnded) {
    free(cal);
    obj = NULL;
    return INV_CAL;
  }

  *obj = cal;
  return OK;
}

void deleteCalendar(Calendar* obj) {
  if(!obj) {
    return;
  }

  if(obj->event) {
    clearList(&obj->event->properties);
    clearList(&obj->event->alarms);
  }

  free(obj);
}

char* printCalendar(const Calendar* obj) {
  char *toReturn;
  char *temp;
  char *alarmHeader = "\tAlarms:\n";
  char *propHeader = "\n\n\tOther properties:\n";
  int len;
  Node *node;

  if(!obj || !obj->event) {
    return NULL;
  }

  toReturn = (char*)malloc(sizeof(float) + sizeof(int)
    + (sizeof(char) * (strlen(obj->prodID) + strlen(obj->event->UID) + strlen(obj->event->creationDateTime.date) + strlen(obj->event->creationDateTime.time) + 75)));
  sprintf(toReturn, "Calendar: version = %.6f, prodID = %s\nEvent\n\tUID = %s\n\tcreationDateTime = %s:%s, UTC=%d\n"
    , obj->version, obj->prodID, obj->event->UID, obj->event->creationDateTime.date, obj->event->creationDateTime.time, obj->event->creationDateTime.UTC);
  len = strlen(toReturn);

  if(&obj->event->alarms) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(alarmHeader) + 1));
    strcat(toReturn, alarmHeader);
    len = strlen(toReturn);

    node = obj->event->alarms.head;
    while(node) {
      temp = obj->event->alarms.printData(node->data);
      toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
      strcat(toReturn, temp);
      len = strlen(toReturn);

      node = node->next;
      free(temp);
    }
  }

  if(&obj->event->properties) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(propHeader) + 1));
    strcat(toReturn, propHeader);
    len = strlen(toReturn);

    node = obj->event->properties.head;
    while(node) {
      temp = obj->event->properties.printData(node->data);
      toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
      strcat(toReturn, temp);
      len = strlen(toReturn);

      node = node->next;
      free(temp);
    }
  }

  return toReturn;
}

const char* printError(ErrorCode err) {
  char *str;
  char *toReturn;

  switch(err) {
    case OK:
      str = "OK";
      break;
    case INV_FILE:
      str = "Invalid File";
      break;
    case INV_CAL:
      str = "Invalid Calendar";
      break;
    case INV_VER:
      str = "Invalid Version";
      break;
    case DUP_VER:
      str = "Multiple Versions Detected";
      break;
    case INV_PRODID:
      str = "Invalid Product ID";
      break;
    case DUP_PRODID:
      str = "Multiple Product IDs Detected";
      break;
    case INV_EVENT:
      str = "Invalid Event";
      break;
    case INV_CREATEDT:
      str = "Invalid Time Created";
      break;
    case OTHER_ERROR:
      str = "Unknown Error";
      break;
    default:
      str = "Unknown Error";
      break;
  }

  toReturn = malloc((strlen(str) + 1) * sizeof(char));
  strcpy(toReturn, str);

  return toReturn;
}
