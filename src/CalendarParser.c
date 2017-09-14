#include "CalendarParser.h"
#include "LinkedListAPI.h"
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

    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 2));
    strcat(toReturn, "\n");
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
  printf("called prop\n");
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
  char *firstChar;
  size_t lineLen;

  firstChar = toClear;
  while(*firstChar != '\0' && isspace(*firstChar)) {
    ++firstChar;
  }

  lineLen = strlen(firstChar) + 1;
  memmove(toClear, firstChar, lineLen);

  if(toClear[strlen(toClear) - 2] == '\r' && toClear[strlen(toClear) - 1] == '\n') {
    toClear[strlen(toClear) - 2] = '\0';
  } else if(toClear[strlen(toClear) - 1] == '\r' || toClear[strlen(toClear) - 1] == '\n') {
    toClear[strlen(toClear) - 1] = '\0';
  }
}

ErrorCode createCalendar(char* fileName, Calendar** obj) {
  char *token;
  char *tempFile;
  char line[256];
  char toIgnore[256];
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
  strcpy(toIgnore, "");

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
    fclose(file);
    obj = NULL;
    return INV_FILE;
  }

  if(!fgets(line, sizeof(line), file)) {
    free(cal);
    fclose(file);
    obj = NULL;
    return INV_FILE;
  }

  clearSpaces(line);
  if(strcmp(line, "BEGIN:VCALENDAR") != 0) {
    free(cal);
    fclose(file);
    obj = NULL;
    return INV_CAL;
  }

  while(fgets(line, sizeof(line), file)) {
    clearSpaces(line);
    token = strtok(line, ":;");

    if(strcmp(toIgnore, "") != 0 && strcmp(token, "END") == 0) {
      token = strtok(NULL, "");
      if(strcmp(toIgnore, token) == 0) {
        strcpy(toIgnore, "");
      }

      continue;
    } else if(strcmp(toIgnore, "") != 0) {
      continue;
    }

    if(!token) {
      free(cal);
      fclose(file);
      obj = NULL;
      return INV_CAL;
    }

    if(strcmp(token, "VERSION") == 0) {
      if(cal->version != 0.0) {
        free(cal);
        fclose(file);
        obj = NULL;
        return DUP_VER;
      }

      token = strtok(NULL, "");
      cal->version = atof(token);

      if(cal->version == 0.0) {
        free(cal);
        fclose(file);
        obj = NULL;
        return INV_VER;
      }
    } else if(strcmp(token, "PRODID") == 0) {
      if(strcmp(cal->prodID, "") != 0) {
        free(cal);
        fclose(file);
        obj = NULL;
        return DUP_PRODID;
      }

      token = strtok(NULL, "");
      if(!strcpy(cal->prodID, token)) {
        free(cal);
        fclose(file);
        obj = NULL;
        return INV_PRODID;
      }
    } else if(strcmp(token, "BEGIN") == 0) {
      token = strtok(NULL, "");
      if(strcmp(token, "VEVENT") != 0) {
        strcpy(toIgnore, token);
        continue;
      }

      int eventEnded = 0;
      cal->event = (Event*)malloc(sizeof(Event));
      strcpy(cal->event->UID, "");
      strcpy(cal->event->creationDateTime.date, "");
      cal->event->properties.head = NULL;
      cal->event->alarms.head = NULL;

      int inAlarm = 0;
      Alarm *alarm;
      Property *prop;

      while(fgets(line, sizeof(line), file) && !eventEnded) {
        clearSpaces(line);
        token = strtok(line, ":;");

        if(strcmp(toIgnore, "") != 0 && strcmp(token, "END") == 0) {
          token = strtok(NULL, "");
          if(strcmp(toIgnore, token) == 0) {
            strcpy(toIgnore, "");
          }

          continue;
        } else if(strcmp(toIgnore, "") != 0) {
          continue;
        }

        if(strcmp(token, "END") == 0) {
          token = strtok(NULL, "");
          if(strcmp(token, "VEVENT") == 0 && !inAlarm) {
            if(strcmp(cal->event->UID, "") == 0 || strcmp(cal->event->creationDateTime.date, "") == 0) {
              free(cal);
              fclose(file);
              obj = NULL;
              return INV_EVENT;
            }

            eventEnded = 1;
            break;
          } else if(strcmp(token, "VALARM") == 0 && inAlarm) {
            if(!alarm || strcmp(alarm->action, "") == 0 || !alarm->trigger) {
              free(cal);
              fclose(file);
              obj = NULL;
              return INV_EVENT;
            }

            if(!cal->event->alarms.head) {
              cal->event->alarms = initializeList(&printFuncAlarm, &deleteFuncAlarm, &compareFuncAlarm);
            }

            insertBack(&cal->event->alarms, (void*)alarm);
            inAlarm = 0;
          } else {
            free(cal);
            fclose(file);
            obj = NULL;
            return INV_EVENT;
          }
        } else if(strcmp(token, "BEGIN") == 0) {
          token = strtok(NULL, "");
          if(strcmp(token, "VALARM") != 0 || inAlarm) {
            strcpy(toIgnore, token);
            continue;
          } else {
            inAlarm = 1;
            alarm = (Alarm*)malloc(sizeof(Alarm));
            strcpy(alarm->action, "");
            alarm->trigger = NULL;
            alarm->properties.head = NULL;
          }
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

            token = strtok(NULL, "T");
            if(!token || strlen(token) != 8) {
              free(cal);
              fclose(file);
              obj = NULL;
              return INV_CREATEDT;
            }

            strcpy(dt.date, token);
            token = strtok(NULL, "T");
            if(!token || strlen(token) != 7) {
              free(cal);
              fclose(file);
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
          } else if(strcmp(token, "UID") == 0) {
            token = strtok(NULL, "");
            strcpy(cal->event->UID, token);
          } else {
            if(strcmp(token, "DTSTART") == 0 && !isDtStart) {
              isDtStart = 1;
            } else if((strcmp(token, "DTSTART") == 0 && isDtStart)
              || ((strcmp(token, "DTEND") == 0 || strcmp(token, "DURATION") == 0) && (!isDtStart || isDtEnded))) {
              free(cal);
              fclose(file);
              obj = NULL;
              return INV_EVENT;
            } else if(strcmp(token, "DTEND") == 0 || strcmp(token, "DURATION") == 0) {
              isDtEnded = 1;
            }

            if(!cal->event->properties.head) {
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
        fclose(file);
        obj = NULL;
        return INV_EVENT;
      }
    } else if(strcmp(token, "END") == 0) {
      token = strtok(NULL, "");
      if(strcmp(token, "VCALENDAR") != 0) {
        free(cal);
        fclose(file);
        obj = NULL;
        return INV_CAL;
      }

      calEnded = 1;
    }
  }

  if(!cal->version || strcmp(cal->prodID, "") == 0 || !cal->event || !calEnded) {
    free(cal);
    fclose(file);
    obj = NULL;
    return INV_CAL;
  }

  *obj = cal;
  fclose(file);
  return OK;
}

void deleteCalendar(Calendar* obj) {
  Node *curr;
  Node *next;
  Node *propCurr;
  Node *propNext;
  Alarm *alarm;

  if(!obj) {
    return;
  }

  if(obj->event) {
    if(obj->event->properties.head) {
      curr = obj->event->properties.head;
      while(curr) {
        next = curr->next;
        if(curr->data) {
          free(curr->data);
        }

        free(curr);
        curr = next;
      }
    }

    if(obj->event->alarms.head) {
      curr = obj->event->alarms.head;
      while(curr) {
        next = curr->next;
        alarm = (Alarm*)curr->data;
        if(alarm->trigger) {
          free(alarm->trigger);
        }

        if(alarm->properties.head) {
          propCurr = alarm->properties.head;
          while(propCurr) {
            propNext = propCurr->next;
            if(propCurr->data) {
              free(propCurr->data);
            }

            free(propCurr);
            propCurr = propNext;
          }
        }

        if(curr->data) {
          free(curr->data);
        }

        free(curr);
        curr = next;
      }
    }

    free(obj->event);
  }

  free(obj);
}

char* printCalendar(const Calendar* obj) {
  char *toReturn;
  char *temp;
  char *alarmHeader = "\tAlarms:\n";
  char *propHeader = "\n\tOther properties:\n";
  int len;
  Node *node;

  if(!obj || !obj->event) {
    return NULL;
  }

  toReturn = (char*)malloc(sizeof(float) + sizeof(int)
    + (sizeof(char) * (strlen(obj->prodID) + strlen(obj->event->UID) + strlen(obj->event->creationDateTime.date) + strlen(obj->event->creationDateTime.time) + 80)));
  sprintf(toReturn, "Calendar: version = %.6f, prodID = %s\nEvent\n\tUID = %s\n\tcreationDateTime = %s:%s, UTC=%d\n"
    , obj->version, obj->prodID, obj->event->UID, obj->event->creationDateTime.date, obj->event->creationDateTime.time, obj->event->creationDateTime.UTC);
  len = strlen(toReturn);

  toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(alarmHeader) + 1));
  strcat(toReturn, alarmHeader);
  len = strlen(toReturn);

  if(obj->event->alarms.head) {
    node = obj->event->alarms.head;
    while(node) {
      temp = printFuncAlarm(node->data);
      toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
      strcat(toReturn, temp);
      len = strlen(toReturn);

      node = node->next;
      free(temp);
    }
  } else {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 8));
    strcat(toReturn, "\t\tNone\n");
    len = strlen(toReturn);
  }

  toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(propHeader) + 1));
  strcat(toReturn, propHeader);
  len = strlen(toReturn);

  if(obj->event->properties.head) {
    node = obj->event->properties.head;
    while(node) {
      temp = printFuncProp(node->data);
      toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
      strcat(toReturn, temp);
      len = strlen(toReturn);

      node = node->next;
      free(temp);
    }
  } else {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 7));
    strcat(toReturn, "\t\tNone");
    len = strlen(toReturn);
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
