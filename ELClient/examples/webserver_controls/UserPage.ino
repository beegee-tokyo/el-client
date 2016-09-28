#include <EEPROM.h>
#include <ELClientWebServer.h>

#define MAGIC 0xABEF
#define MAX_STR_LEN  32

// EEPROM content 
#define EEPROM_POS_MAGIC          0
#define EEPROM_POS_FIRST_NAME     (EEPROM_POS_MAGIC + 2)
#define EEPROM_POS_LAST_NAME      (EEPROM_POS_FIRST_NAME + MAX_STR_LEN)
#define EEPROM_POS_AGE            (EEPROM_POS_LAST_NAME + MAX_STR_LEN)
#define EEPROM_POS_GENDER         (EEPROM_POS_AGE+1)
#define EEPROM_POS_NOTIFICATIONS  (EEPROM_POS_GENDER+1)

// initialization
void userInit()
{
  uint16_t magic;
  EEPROM.get(EEPROM_POS_MAGIC, magic);

  if( magic != MAGIC ) // EEPROM is uninitialized?
  {
    magic = MAGIC;
    // set default values
    EEPROM.put(EEPROM_POS_MAGIC, magic);
    EEPROM.update(EEPROM_POS_FIRST_NAME, 0);
    EEPROM.update(EEPROM_POS_LAST_NAME, 0);
    EEPROM.update(EEPROM_POS_AGE, 0);
    EEPROM.update(EEPROM_POS_GENDER, 'f');
    EEPROM.update(EEPROM_POS_NOTIFICATIONS, 0);
  }
}

// write a string to EEPROM
void userWriteStr(char * str, int ndx)
{
  for(uint8_t i=0; i < MAX_STR_LEN-1; i++)
  {
    EEPROM.update(ndx + i, str[i]);
    if( str[i] == 0 )
      break;
  }
  EEPROM.update(ndx + MAX_STR_LEN - 1, 0);
}

// read a string from EEPROM
void userReadStr(char * str, int ndx)
{
  for(uint8_t i=0; i < MAX_STR_LEN; i++)
  {
    str[i] = EEPROM[ndx + i];
  }
}

void userHtmlCallback(WebServerCommand command, char * data, int dataLen)
{
  switch(command)
  {
    case BUTTON_PRESS:
      // no buttons
      break;
    case SET_FIELD:
      {
        String fld = data;
        if( fld == F("first_name"))
          userWriteStr(webServer.getArgString(), EEPROM_POS_FIRST_NAME);
        else if( fld == F("last_name"))
          userWriteStr(webServer.getArgString(), EEPROM_POS_LAST_NAME);
        else if( fld == F("age"))
          EEPROM.update(EEPROM_POS_AGE, (uint8_t)webServer.getArgInt());
        else if( fld == F("gender"))
        {
          String gender = webServer.getArgString();
          EEPROM.update(EEPROM_POS_GENDER, (gender == F("male")) ? 'm' : 'f');
        }
        else if( fld == F("notifications"))
          EEPROM.update(EEPROM_POS_NOTIFICATIONS, webServer.getArgBoolean());
      }
      break;
    case LOAD:
      {
        char buf[MAX_STR_LEN];
        userReadStr( buf, EEPROM_POS_FIRST_NAME );
        webServer.setArgString(F("first_name"), buf);
        userReadStr( buf, EEPROM_POS_LAST_NAME );
        webServer.setArgString(F("last_name"), buf);
        webServer.setArgInt(F("age"), (uint8_t)EEPROM[EEPROM_POS_AGE]);
        webServer.setArgString(F("gender"), (EEPROM[EEPROM_POS_GENDER] == 'm') ? F("male") : F("female"));
        webServer.setArgBoolean(F("notifications"), EEPROM[EEPROM_POS_NOTIFICATIONS] != 0);
      }
      break;
    case REFRESH:
      // do nothing
      break;
  }
}

