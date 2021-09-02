/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef __OCPN_DATASTREAMEVENT_H__
#define __OCPN_DATASTREAMEVENT_H__

#include <wx/event.h>
#include <string>

class DataStream;

class OCPN_DataStreamEvent : public wxEvent {
public:
  OCPN_DataStreamEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
  ~OCPN_DataStreamEvent();

  // accessors
  void SetNMEAString(std::string string) { m_NMEAstring = string; }
  void SetStream(DataStream *pDS) { m_pDataStream = pDS; }
  std::string GetNMEAString() const { return m_NMEAstring; }
  DataStream *GetStream() const { return m_pDataStream; }

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

  wxString ProcessNMEA4Tags();

private:
  std::string m_NMEAstring;
  DataStream *m_pDataStream;
};

#endif  // __OCPN_DATASTREAMEVENT_H__
