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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


// MinGW/MSServer workarounds
#ifndef EWX_RESTARTAPPS
#define EWX_RESTARTAPPS 0x00000040
#endif
#ifndef EWX_HYBRID_SHUTDOWN
#define EWX_HYBRID_SHUTDOWN 0x00400000
#endif


namespace {

void exit_windows(unsigned flags)
{
    // Get a token for this process.
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    // Get the LUID for the shutdown privilege.
    TOKEN_PRIVILEGES tkp;
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS)
        return;

    // Shut down or reboot the system.
    ExitWindowsEx(flags, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
}

} // namespace


namespace platform {
namespace power {

void reboot()
{
    exit_windows(EWX_REBOOT | EWX_RESTARTAPPS);
}

void shutdown()
{
    exit_windows(EWX_SHUTDOWN | EWX_POWEROFF | EWX_HYBRID_SHUTDOWN);
}

} // namespace power
} // namespace platform
