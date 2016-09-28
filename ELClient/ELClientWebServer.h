// Copyright (c) 2016 by B. Runnels and T. von Eicken

#ifndef _EL_CLIENT_WEB_SERVER_H_
#define _EL_CLIENT_WEB_SERVER_H_

#include <Arduino.h>
#include "ELClient.h"

typedef enum
{
  BUTTON_PRESS,  /**< called when HTML button is pressed */
  SET_FIELD,     /**< called when a form field is submitted */
  REFRESH,       /**< called at page refresh */
  LOAD,          /**< called at the first page load */
} WebServerCommand;

// callback funtion
typedef void (*WebServerCallback)(WebServerCommand command, char * data, int dataLen); /**< Typedef for web server callback */

struct _Handlers;

// This class implements function for web-server
class ELClientWebServer {
public:
  // Initialize with an ELClient object
  ELClientWebServer(ELClient* elc);
  
  // initializes the web-server
  void    setup();
  
  // registers an URL handler
  void    registerHandler(const char * URL, WebServerCallback callback);
  // registers an URL handler
  void    registerHandler(const __FlashStringHelper * URL, WebServerCallback callback);
  // registers an URL handler
  void    registerHandler(const String &URL, WebServerCallback callback);
  // unregisters an URL handler
  void    unregisterHandler(const char * URL);
  // unregisters an URL handler
  void    unregisterHandler(const __FlashStringHelper * URL);
  // unregisters an URL handler
  void    unregisterHandler(const String &URL);
  // notifies ESP8266, that MCU is interested in web-server callbacks
  void    registerCallback();
  
  // sets int value of an HTML field
  void    setArgInt(const char * name, int32_t value);
  // sets JSON value of an HTML field
  void    setArgJson(const char * name, const char * value);
  // sets string value of an HTML field
  void    setArgString(const char * name, const char * value);
  // sets boolean value of an HTML field
  void    setArgBoolean(const char * name, uint8_t value);
  // sets null value of an HTML field
  void    setArgNull(const char * name);
  // sets float value of an HTML field
  void    setArgFloat(const char * name, float f);

  // sets int value of an HTML field
  void    setArgInt(const __FlashStringHelper * name, int32_t value);
  // sets JSON value of an HTML field
  void    setArgJson(const __FlashStringHelper * name, const char * value);
  // sets JSON value of an HTML field
  void    setArgJson(const __FlashStringHelper * name, const __FlashStringHelper * value);
  // sets string value of an HTML field
  void    setArgString(const __FlashStringHelper * name, const char * value);
  // sets string value of an HTML field
  void    setArgString(const __FlashStringHelper * name, const __FlashStringHelper * value);
  // sets boolean value of an HTML field
  void    setArgBoolean(const __FlashStringHelper * name, uint8_t value);
  // sets null value of an HTML field
  void    setArgNull(const __FlashStringHelper * name);
  // sets float value of an HTML field
  void    setArgFloat(const __FlashStringHelper * name, float f);

  // SET_FIELD: gets the value of the field as integer
  int32_t getArgInt();
  // SET_FIELD: gets the value of the field as string
  char *  getArgString();
  // SET_FIELD: gets the value of the field as boolean
  uint8_t getArgBoolean();
  // SET_FIELD: gets the value of the field as float
  float   getArgFloat();

  // returns the web-server instance
  static ELClientWebServer * getInstance() { return instance; }
  
  void processPacket(ELClientPacket *packet); // internal
  
private:
  ELClient* _elc;
  CallbackPacketHandler nextPacketHandler;
  
  static uint8_t webServerPacketHandler(ELClientPacket * packet);
  static ELClientWebServer * instance;
  
  uint8_t                    remote_ip[4]; /**< Remote IP of web server request */
  uint16_t                   remote_port; /**< Remote port of web server request */
  
  char *                     arg_ptr; /**< Pointer to arguments */
  
  struct _Handler          * handlers; /**< Structure of callback function pointers */
};

#endif // _EL_CLIENT_WEB_SERVER_H_
