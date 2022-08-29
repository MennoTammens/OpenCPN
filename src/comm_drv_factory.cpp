/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_factory: Communication driver factory.
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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
 **************************************************************************/

// FIXME  Why is this needed?
#ifdef __MSVC__
#include "winsock2.h"
#include "wx/msw/winundef.h"
#endif

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "comm_util.h"
#include "comm_drv_n2k_serial.h"
#include "comm_drv_n0183_serial.h"
#include "comm_drv_n0183_net.h"
#include "comm_drv_signalk_net.h"
#include "comm_navmsg_bus.h"
#include "comm_drv_registry.h"

#ifndef __WXMSW__
#include "comm_drv_n2k_socketcan.h"
#endif

std::shared_ptr<AbstractCommDriver> MakeCommDriver(
    const ConnectionParams* params) {
  wxLogMessage(
      wxString::Format(_T("MakeCommDriver: %s"), params->GetDSPort().c_str()));

  auto& msgbus = NavMsgBus::GetInstance();
  auto& registry = CommDriverRegistry::getInstance();
  switch (params->Type) {
    case SERIAL:
      switch (params->Protocol) {
        case PROTO_NMEA2000: {
          auto driver = std::make_shared<CommDriverN2KSerial>(params, msgbus);
          registry.Activate(driver);
          return driver;
          break;
        }
        default: {
          auto driver = std::make_shared<CommDriverN0183Serial>(params, msgbus);
          registry.Activate(driver);
          return driver;

          break;
        }
      }
    case NETWORK:
      switch (params->NetProtocol) {
        case SIGNALK: {
          auto driver = std::make_shared<CommDriverSignalKNet>(params, msgbus);
          registry.Activate(driver);
          return driver;
          break;
        }
        default: {
          auto driver = std::make_shared<CommDriverN0183Net>(params, msgbus);
          registry.Activate(driver);
          return driver;
          break;
        }
      }

#if defined(__linux__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
    case SOCKETCAN:
    {
      auto driver = std::make_shared<CommDriverN2KSocketCAN>(params, msgbus);
      registry.Activate(driver);
      return driver;
      break;
    }
#endif

#if 0  // FIXME (dave)
    case INTERNAL_GPS:
      return new InternalGPSDataStream(input_consumer, params);
    case INTERNAL_BT:
      return new InternalBTDataStream(input_consumer, params);

#endif
    default:
      break;
  }
  return NULL;
};
