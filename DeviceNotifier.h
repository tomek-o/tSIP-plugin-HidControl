/** \file
    \brief Device attach/detach notifier

    This class implements a worker thread that uses a hidden window to receive WM_DEVICECHANGE
    messages for specified device interface class.
    Note: callback used by this class is static!
*/

/** \note Common device interface class GUIDs
USB Raw Device	{a5dcbf10-6530-11d2-901f-00c04fb951ed}
Disk Device	    {53f56307-b6bf-11d0-94f2-00a0c91efb8b}
Network Card	{ad498944-762f-11d0-8dcb-00c04fc3358c}
HID	            {4d1e55b2-f16f-11cf-88cb-001111000030}
Palm	        {784126bf-4190-11d4-b5c2-00c04f687a67}
*/

#ifndef DEVICENOTIFIER_H_INCLUDED
#define DEVICENOTIFIER_H_INCLUDED

#include <windef.h>
#include <string>

namespace nsDeviceNotifier
{
    class DeviceNotifier
    {
    private:
        HANDLE Thread;
    public:
        DeviceNotifier();
        ~DeviceNotifier();
        int RegisterForNotifications(GUID hidGuid/*, HANDLE deviceHandle*/);
        typedef int (* CallbackFunction)(std::string path, bool attached);
        static CallbackFunction pCallbackFunction;
    };
}

#endif // DEVICENOTIFIER_H_INCLUDED
