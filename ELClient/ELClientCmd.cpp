/*! \file ELClientCmd.cpp
    \brief Constructor and functions for ELClientCmd
    \author B. Runnels
    \author T. von Eicken
    \date 2016
*/
// Copyright (c) 2016 by B. Runnels and T. von Eicken

#include "ELClientCmd.h"

/*! ELClientCmd(ELClient* elc)
    @brief Constructor for ELClientCmd
*/
ELClientCmd::ELClientCmd(ELClient* elc) :_elc(elc) {}

/*! GetTime()
@brief Get time from ESP
@details Time from the ESP is unformated value of seconds
@warning If the ESP cannot connect to the NTP server or the connection NTP server is not setup, 
	then this time is the number of seconds since the last reboot of the ESP
@return <code>uint32_t</code>
	current time as number of seconds
	- since Thu Jan  1 00:00:58 UTC 1970 if ESP has time from NTP
	- since last reboot of ESP if no NTP time is available
@par Example
@code
	uint32_t t = cmd.GetTime();
	Serial.print("Time: "); Serial.println(t);
@endcode
*/
uint32_t ELClientCmd::GetTime() {
  _elc->Request(CMD_GET_TIME, 0, 0);
  _elc->Request();

  ELClientPacket *pkt = _elc->WaitReturn();
  return pkt ? pkt->value : 0;
}

/*! SysRestart()
@brief Restart both ESP and uC
@details RST input of uC must be connected to ESP and reset pin must be set correct 
@warning If the reset pin of the uC is not connected or the setup is wrong, only the ESP will reboot.
	The ESP will not respond to this command because it will try to reset the uC and itself after receiving the command
@par Example
@code
	cmd.SysRestart();
@endcode
*/
uint32_t ELClientCmd::SysRestart() {
  _elc->Request(CMD_RESTARTSYS, 0, 0);
  _elc->Request();
}

/*! EspRestart()
@brief Restart ESP
@warning The ESP will not respond to this command because it will try to reset and itself after receiving the command
@par Example
@code
	cmd.SysRestart();
@endcode
*/
uint32_t ELClientCmd::EspRestart() {
  _elc->Request(CMD_RESTART, 0, 0);
  _elc->Request();
}

