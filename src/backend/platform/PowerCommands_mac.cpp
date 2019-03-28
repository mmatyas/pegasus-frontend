// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "PowerCommands.h"

#include "LocaleUtils.h"

#include <QDebug>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>


namespace {

bool send_apple_event(AEEventID apple_event_id)
{
    const ProcessSerialNumber system_psn { 0, kSystemProcess };
    OSStatus error = noErr;

    AEAddressDesc target_desc;
    error = AECreateDesc(typeProcessSerialNumber,
                         &system_psn, sizeof(system_psn),
                         &target_desc);
    if (error != noErr)
        return false;

    AppleEvent apple_event { typeNull, NULL };
    error = AECreateAppleEvent(kCoreEventClass, apple_event_id,
                               &target_desc,
                               kAutoGenerateReturnID, kAnyTransactionID,
                               &apple_event);
    AEDisposeDesc(&target_desc);
    if (error != noErr)
        return false;

    AppleEvent event_reply { typeNull, NULL };
    error = AESendMessage(&apple_event, &event_reply,
                          kAENoReply, kAEDefaultTimeout);
    AEDisposeDesc(&apple_event);
    if (error != noErr)
        return false;

    AEDisposeDesc(&event_reply);
    return true;
}

} // namespace


namespace platform {
namespace power {

void reboot()
{
    if (!send_apple_event(kAERestart))
        qWarning().noquote() << tr_log("[error] Requesting reboot has failed.");
}

void shutdown()
{
    if (!send_apple_event(kAEShutDown))
        qWarning().noquote() << tr_log("[error] Requesting shutdown has failed.");
}

} // namespace power
} // namespace platform
