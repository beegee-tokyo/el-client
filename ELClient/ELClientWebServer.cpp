#include "ELClientWebServer.h"

typedef enum {
  WS_LOAD=0,   /**< page first load */
  WS_REFRESH,  /**< page refresh */
  WS_BUTTON,   /**< button press */
  WS_SUBMIT,   /**< form submission */
} RequestReason;

typedef enum
{
  WEB_STRING=0,  /**< value type string */
  WEB_NULL,      /**< value type null */
  WEB_INTEGER,   /**< value type integer */
  WEB_BOOLEAN,   /**< value type boolean */
  WEB_FLOAT,     /**< value type float */
  WEB_JSON       /**< value type json */
} WebValueType;

// handler descriptor
struct _Handler
{
  String URL;                 /**< handler URL */
  WebServerCallback callback; /**< handler callback */
  struct _Handler * next;     /**< next handler */
};

static ELClientWebServer * ELClientWebServer::instance = 0;

/*! ELClientWebServer(ELClient* elc)
@brief Constructor for ELClientWebServer
@param elc
	Pointer to ELClient instance
@par Example
@code
	ELClientWebServer webServer(&esp);
@endcode
*/
ELClientWebServer::ELClientWebServer(ELClient* elc) :_elc(elc),handlers(0), arg_ptr(0) {
  // save the current packet handler and register a new one
  nextPacketHandler = _elc->GetCallbackPacketHandler();
  _elc->SetCallbackPacketHandler(ELClientWebServer::webServerPacketHandler);
  instance = this;
}

// packet handler for web-server
/*! webServerPacketHandler(ELClientPacket * packet)
@brief Packet handler for web-server
@note
	This function is usually not needed for applications.
@param packet
	Pointer to ELClientPacket with the request info
@par Example
@code
	no example yet
@endcode
*/
static uint8_t ELClientWebServer::webServerPacketHandler(ELClientPacket * packet)
{
  if( packet->cmd == CMD_WEB_REQ_CB )
  {
    ELClientWebServer::getInstance()->processPacket(packet);
    return 1; // packet handled
  }
  else if( ELClientWebServer::getInstance()->nextPacketHandler != 0 )
    return ELClientWebServer::getInstance()->nextPacketHandler(packet); // push to the next packet handler
    
  return 0; // packet was not handled
}

// initialization
/*! setup(void)
@brief Setup web server
@details Register callback function for web server requests on the ESP
@par Example
@code
	// register the JSON URL handlers
	webServer.registerHandler(F("/LED.html.json"), ledHtmlCallback);
	webServer.registerHandler(F("/User.html.json"), userHtmlCallback);
	webServer.registerHandler(F("/Voltage.html.json"), voltageHtmlCallback);
	webServer.setup();
@endcode
*/
void ELClientWebServer::setup()
{
  registerCallback();
}

/*! registerHandler(const char * URL, WebServerCallback callback)
@brief Register callback handlers for the web sites
@note A callback function for each website should be registered
@param URL
	URL to be handled
@param callback
	Pointer to callback function to handle request on this URL
@par Example
@code
	// register the JSON URL handlers
	webServer.registerHandler("/LED.html.json", ledHtmlCallback);
	webServer.registerHandler("/User.html.json", userHtmlCallback);
	webServer.registerHandler("/Voltage.html.json", voltageHtmlCallback);
	webServer.setup();
@endcode
*/
void ELClientWebServer::registerHandler(const char * URL, WebServerCallback callback)
{
  String s = URL;
  registerHandler(s, callback);
}

/*! registerHandler(const __FlashStringHelper * URL, WebServerCallback callback)
@brief Register callback handlers for the web sites
@note A callback function for each website should be registered
@param URL
	URL to be handled stored in program memory
@param callback
	Pointer to callback function to handle request on this URL
@par Example
@code
	// register the JSON URL handlers
	webServer.registerHandler(F("/LED.html.json"), ledHtmlCallback);
	webServer.registerHandler(F("/User.html.json"), userHtmlCallback);
	webServer.registerHandler(F("/Voltage.html.json"), voltageHtmlCallback);
	webServer.setup();
@endcode
*/
void ELClientWebServer::registerHandler(const __FlashStringHelper * URL, WebServerCallback callback)
{
  String s = URL;
  registerHandler(s, callback);
}

// registers a new handler
/*! registerHandler(const char * URL, WebServerCallback callback)
@brief Register callback handlers for the web sites
@note A callback function for each website should be registered
@param URL
	Pointer to URL string to be handled
@param callback
	Pointer to callback function to handle request on this URL
@par Example
@code
	no example yet
@endcode
*/
void ELClientWebServer::registerHandler(const String &URL, WebServerCallback callback)
{
  unregisterHandler(URL);
  
  struct _Handler * hnd = new struct _Handler(); // "new" is used here instead of malloc to call String destructor at freeing. DOn't use malloc/free.
  hnd->URL = URL;             // handler URL
  hnd->callback = callback;   // callback
  hnd->next = handlers;       // next handler
  handlers = hnd;             // change the first handler
}

/*! unregisterHandler(const char * URL)
@brief Unregister callback handlers for the web sites
@param URL
	URL to be stopped handling
@par Example
@code
	no example yet
@endcode
*/
void ELClientWebServer::unregisterHandler(const char * URL)
{
  String s = URL;
  unregisterHandler(s);
}

/*! unregisterHandler(const __FlashStringHelper * URL)
@brief Unregister callback handlers for the web sites
@param URL
	URL to be stopped handling stored in program memory
@par Example
@code
	no example yet
@endcode
*/
void ELClientWebServer::unregisterHandler(const __FlashStringHelper * URL)
{
  String s = URL;
  unregisterHandler(s);
}

// unregisters a previously registered handler
/*! unregisterHandler(const String &URL)
@brief Unregister callback handlers for the web sites
@param URL
	Pointer to URL string to be stopped handling
@par Example
@code
	no example yet
@endcode
*/
void ELClientWebServer::unregisterHandler(const String &URL)
{
  struct _Handler *prev = 0;
  struct _Handler *hnd = handlers;
  while( hnd != 0 )
  {
    if( hnd->URL == URL )
    {
      if( prev == 0 )
        handlers = hnd->next;
      else
        prev->next = hnd->next;
      
      delete hnd;
    }
    prev = hnd;
    hnd = hnd->next;
  }
}

/*! registerCallback()
@brief Register callback function for incoming request 
@note WebServer doesn't send messages to MCU only if asked. Register here to the web callback. Periodic reregistration is required in case of ESP8266 reset
@par Example
@code
	if ((millis()-last) > 4000) {
		webServer.registerCallback();  // just for esp-link reset: reregister callback at every 4s
		last = millis();
	}
@endcode
*/
void ELClientWebServer::registerCallback()
{
  // WebServer doesn't send messages to MCU only if asked
  // register here to the web callback
  // periodic reregistration is required in case of ESP8266 reset
  _elc->Request(CMD_CB_ADD, 100, 1);
  _elc->Request(F("webCb"), 5);
  _elc->Request();
}

/*! processPacket(ELClientPacket *packet)
@brief Handle incoming packets for web-server and call registered callback function
@note
	This function is usually not needed for applications.
@param packet
	Pointer to ELClientPacket with the request info
@par Example
@code
	no example yet
@endcode
*/
void ELClientWebServer::processPacket(ELClientPacket *packet)
{
  ELClientResponse response(packet);

  uint16_t shrt;
  response.popArg(&shrt, 2);
  RequestReason reason = (RequestReason)shrt; // request reason

  response.popArg(remote_ip, 4);    // remote IP address
  response.popArg(&remote_port, 2); // remote port
  
  char * url;
  int urlLen = response.popArgPtr(&url);
  
  WebServerCallback callback = 0;
  
  struct _Handler *hnd = handlers;
  while( hnd != 0 )
  {
    if( hnd->URL.length() == urlLen && memcmp( url, hnd->URL.begin(), urlLen ) == 0 )
    {
      callback = hnd->callback;
      break;
    }
    hnd = hnd->next;
  }

  if( hnd == 0 ) // no handler found for the URL
  {
    _elc->_debug->print(F("Handler not found for URL:"));
    
    for(int i=0; i < urlLen; i++)
      _elc->_debug->print( url[i] );
    _elc->_debug->println();
    return;
  }
  
  switch(reason)
  {
    case WS_BUTTON: // invoked when a button pressed
      {
        char * idPtr;
        int idLen = response.popArgPtr(&idPtr);
  
        char id[idLen+1];
        memcpy(id, idPtr, idLen);
        id[idLen] = 0;

        callback(BUTTON_PRESS, id, idLen);
      }
      break;
    case WS_SUBMIT: // invoked when a form submitted
      {
        int cnt = 4;

        while( cnt < response.argc() )
        {
          char * idPtr;
          int idLen = response.popArgPtr(&idPtr);
          int nameLen = strlen(idPtr+1);
          int valueLen = idLen - nameLen -2;

          arg_ptr = (char *)malloc(valueLen+1);
          arg_ptr[valueLen] = 0;
          memcpy(arg_ptr, idPtr + 2 + nameLen, valueLen);

          callback(SET_FIELD, idPtr+1, nameLen);
  
          free(arg_ptr);
          arg_ptr = 0;
          cnt++;
        }
      }
      return;
    case WS_LOAD: // invoked at refresh / load
    case WS_REFRESH:
      break;
    default:
      return;
  }
  
  // the response is generated here with the fields to refresh

  _elc->Request(CMD_WEB_DATA, 100, 255);     // 255 arguments (can't foretell, so use maximum)
  _elc->Request(remote_ip, 4);               // send remote IP address
  _elc->Request((uint8_t *)&remote_port, 2); // send remote port

  callback( reason == WS_LOAD ? LOAD : REFRESH, NULL, 0); // send arguments

  _elc->Request((uint8_t *)NULL, 0);         // end indicator
  _elc->Request();                           // finish packet
}


/*! setArgJson(const char * name, const char * value)
@brief Respond to web server request with a JSON
@param name
	Name of argument 
@param value
	JSON string to be sent as response
@par Example
@code
	String table = F("[[\"Time\",\"Min\",\"AVG\",\"Max\"]");
	for(uint8_t i=0; i < history_cnt; i++ )
	{
		float min_f = (float)history_min[i] / 256.f;
		float max_f = (float)history_max[i] / 256.f;
		float avg_f = (float)history_avg[i] / 256.f;
		table += ",[\"";
		table += (history_ts[i] / 1000);
		table += " s\",\"";
		table += floatToString(min_f);
		table += " V\",\"";
		table += floatToString(avg_f);
		table += " V\",\"";
		table += floatToString(max_f);
		table += " V\"]";
	}
	table += ']';
	webServer.setArgJson("table", table.begin());
@endcode
*/
void ELClientWebServer::setArgJson(const char * name, const char * value)
{
  uint8_t nlen = strlen(name);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy(buf+1, name);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value)
@brief Respond to web server request with a JSON
@param name
	Name of argument stored in program memory
@param value
	JSON string to be sent as response stored in program memory
@par Example
@code
	String table = F("[[\"Time\",\"Min\",\"AVG\",\"Max\"]");
	for(uint8_t i=0; i < history_cnt; i++ )
	{
		float min_f = (float)history_min[i] / 256.f;
		float max_f = (float)history_max[i] / 256.f;
		float avg_f = (float)history_avg[i] / 256.f;
		table += ",[\"";
		table += (history_ts[i] / 1000);
		table += " s\",\"";
		table += floatToString(min_f);
		table += " V\",\"";
		table += floatToString(avg_f);
		table += " V\",\"";
		table += floatToString(max_f);
		table += " V\"]";
	}
	table += ']';
	webServer.setArgJson("table", table.begin());
@endcode
*/
void ELClientWebServer::setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  const char * value_p = reinterpret_cast<const char *>(value);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen_P(value_p);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy_P(buf+1, name_p);
  strcpy_P(buf+2+nlen, value_p);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgJson(const __FlashStringHelper * name, const char * value)
@brief Respond to web server request with a JSON
@param name
	Name of argument stored in program memory
@param value
	JSON string to be sent as response
@par Example
@code
	String table = "[[\"Time\",\"Min\",\"AVG\",\"Max\"]";
	for(uint8_t i=0; i < history_cnt; i++ )
	{
		float min_f = (float)history_min[i] / 256.f;
		float max_f = (float)history_max[i] / 256.f;
		float avg_f = (float)history_avg[i] / 256.f;
		table += ",[\"";
		table += (history_ts[i] / 1000);
		table += " s\",\"";
		table += floatToString(min_f);
		table += " V\",\"";
		table += floatToString(avg_f);
		table += " V\",\"";
		table += floatToString(max_f);
		table += " V\"]";
	}
	table += ']';
	webServer.setArgJson(F("table"), table.begin());
@endcode
*/
void ELClientWebServer::setArgJson(const __FlashStringHelper * name, const char * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_JSON;
  strcpy_P(buf+1, name_p);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgString(const char * name, const char * value)
@brief Respond to web server request with a String
@param name
	Name of argument
@param value
	String to be sent as response
@par Example
@code
	webServer.setArgString("duty", "25_75");
@endcode
*/
void ELClientWebServer::setArgString(const char * name, const char * value)
{
  uint8_t nlen = strlen(name);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy(buf+1, name);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value)
@brief Respond to web server request with a String
@param name
	Name of argument stored in program memory
@param value
	String to be sent as response stored in program memory
@par Example
@code
	webServer.setArgString(F("duty"), F("25_75"));
@endcode
*/
void ELClientWebServer::setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  const char * value_p = reinterpret_cast<const char *>(value);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen_P(value_p);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy_P(buf+1, name_p);
  strcpy_P(buf+2+nlen, value_p);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgString(const __FlashStringHelper * name, const char * value)
@brief Respond to web server request with a String
@param name
	Name of argument stored in program memory
@param value
	String to be sent as response
@par Example
@code
	char buf[MAX_STR_LEN];
	userReadStr( buf, EEPROM_POS_FIRST_NAME );
	webServer.setArgString(F("first_name"), buf);
@endcode
*/
void ELClientWebServer::setArgString(const __FlashStringHelper * name, const char * value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  uint8_t vlen = strlen(value);
  char buf[nlen+vlen+3];
  buf[0] = WEB_STRING;
  strcpy_P(buf+1, name_p);
  strcpy(buf+2+nlen, value);
  _elc->Request(buf, nlen+vlen+2);
}

/*! setArgBoolean(const char * name, uint8_t value)
@brief Respond to web server request with a boolean value
@param name
	Name of argument
@param value
	Boolean value to be send
@par Example
@code
	webServer.setArgBoolean("notifications", EEPROM[EEPROM_POS_NOTIFICATIONS] != 0);
@endcode
*/
void ELClientWebServer::setArgBoolean(const char * name, uint8_t value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 4];
  buf[0] = WEB_BOOLEAN;
  strcpy(buf+1, name);
  buf[2 + nlen] = value;
  _elc->Request(buf, nlen+3);
}

/*! setArgBoolean(const __FlashStringHelper * name, uint8_t value)
@brief Respond to web server request with a boolean value
@param name
	Name of argument stored in program memory
@param value
	Boolean value to be send
@par Example
@code
	webServer.setArgBoolean(F("notifications"), EEPROM[EEPROM_POS_NOTIFICATIONS] != 0);
@endcode
*/
void ELClientWebServer::setArgBoolean(const __FlashStringHelper * name, uint8_t value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 4];
  buf[0] = WEB_BOOLEAN;
  strcpy_P(buf+1, name_p);
  buf[2 + nlen] = value;
  _elc->Request(buf, nlen+3);
}

/*! setArgInt(const char * name, int32_t value)
@brief Respond to web server request with an integer value
@param name
	Name of argument
@param value
	Integer value to be send
@par Example
@code
	webServer.setArgInt("age", (uint8_t)EEPROM[EEPROM_POS_AGE]);
@endcode
*/
void ELClientWebServer::setArgInt(const char * name, int32_t value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 7];
  buf[0] = WEB_INTEGER;
  strcpy(buf+1, name);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

/*! setArgInt(const __FlashStringHelper * name, int32_t value)
@brief Respond to web server request with an integer value
@param name
	Name of argument stored in program memory
@param value
	Boolean value to be send
@par Example
@code
	webServer.setArgInt(F("age"), (uint8_t)EEPROM[EEPROM_POS_AGE]);
@endcode
*/
void ELClientWebServer::setArgInt(const __FlashStringHelper * name, int32_t value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 7];
  buf[0] = WEB_INTEGER;
  strcpy_P(buf+1, name_p);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

/*! setArgNull(const char * name)
@brief Respond to web server request with a NULL value
@param name
	Name of argument
@par Example
@code
	webServer.setArgNull("invalid");
@endcode
*/
void ELClientWebServer::setArgNull(const char * name)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 2];
  buf[0] = WEB_NULL;
  strcpy(buf+1, name);
  _elc->Request(buf, nlen+2);
}

/*! setArgNull(const __FlashStringHelper * name)
@brief Respond to web server request with a NULL value
@param name
	Name of argument stored in program memory
@par Example
@code
	webServer.setArgNull(F("invalid"));
@endcode
*/
void ELClientWebServer::setArgNull(const __FlashStringHelper * name)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 2];
  buf[0] = WEB_NULL;
  strcpy_P(buf+1, name_p);
  _elc->Request(buf, nlen+2);
}

/*! setArgFloat(const char * name, float value)
@brief Respond to web server request with an float value
@param name
	Name of argument
@param value
	Float value to be send
@par Example
@code
	webServer.setArgFloat("age", 123.45);
@endcode
*/
void ELClientWebServer::setArgFloat(const char * name, float value)
{
  uint8_t nlen = strlen(name);
  char buf[nlen + 7];
  buf[0] = WEB_FLOAT;
  strcpy(buf+1, name);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

/*! setArgFloat(const __FlashStringHelper * name, float value)
@brief Respond to web server request with an float value
@param name
	Name of argument stored in memory
@param value
	Float value to be send
@par Example
@code
	webServer.setArgFloat(F("age"), 123.45);
@endcode
*/
void ELClientWebServer::setArgFloat(const __FlashStringHelper * name, float value)
{
  const char * name_p = reinterpret_cast<const char *>(name);
  
  uint8_t nlen = strlen_P(name_p);
  char buf[nlen + 7];
  buf[0] = WEB_FLOAT;
  strcpy_P(buf+1, name_p);
  memcpy(buf+2+nlen, &value, 4);
  _elc->Request(buf, nlen+6);
}

/*! getArgInt()
@brief Get integer argument from web site
@return <code>int32_t</code>
	Integer value returned from web site
@par Example
@code
	blinking_frequency = webServer.getArgInt();
@endcode
*/
int32_t ELClientWebServer::getArgInt()
{
  return (int32_t)atol(arg_ptr);
}

/*! getArgString()
@brief Get string argument from web site
@return <code>char *</code>
	String value returned from web site
@par Example
@code
	String arg = webServer.getArgString();
@endcode
*/
char * ELClientWebServer::getArgString()
{
  return arg_ptr;
}

/*! getArgBoolean()
@brief Get boolean argument from web site
@return <code>uint8_t</code>
	Boolean value returned from web site
@par Example
@code
	bool status = (bool)webServer.getArgBoolean();
@endcode
*/
uint8_t ELClientWebServer::getArgBoolean()
{
  if( strcmp_P(arg_ptr, PSTR("on")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("true")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("yes")) == 0 )
    return 1;
  if( strcmp_P(arg_ptr, PSTR("1")) == 0 )
    return 1;
  return 0;
}

/*! getArgFloat()
@brief Get integer argument from web site
@return <code>float</code>
	Float value returned from web site
@par Example
@code
	float temperature = getArgFloat();
@endcode
*/
float ELClientWebServer::getArgFloat()
{
  return atof(arg_ptr);
}
