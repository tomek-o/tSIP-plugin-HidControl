#include "DeviceNotifier.h"

/*
	Copyright 2009 Tomasz Ostrowski, http://tomeko.net. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are
	permitted provided that the following conditions are met:

	   1. Redistributions of source code must retain the above copyright notice, this list of
		  conditions and the following disclaimer.

	   2. Redistributions in binary form must reproduce the above copyright notice, this list
		  of conditions and the following disclaimer in the documentation and/or other materials
		  provided with the distribution.

	THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <windows.h>

// MinGW:
// some strange problems when WINVER is defined before including windows.h
// (PVECTORED_EXCEPTION_HANDLER undefined and other)
#undef WINVER
#define WINVER 0x0501
extern "C" WINUSERAPI HDEVNOTIFY WINAPI RegisterDeviceNotificationA(HANDLE,LPVOID,DWORD);
#define DEVICE_NOTIFY_WINDOW_HANDLE 0x00000000
#include <dbt.h>
#include <stdio.h>

#include <iostream>
using namespace std;
using namespace nsDeviceNotifier;

HWND hwnd = NULL;
DeviceNotifier::CallbackFunction DeviceNotifier::pCallbackFunction = NULL;

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage (0);
        return 0;
    case WM_DEVICECHANGE:
        {
            //SetEvent(hEvent);
            PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
            //cout << "WM_DEVICECHANGE, wParam = " << wParam << endl;
            switch (wParam)
            {
            case DBT_DEVICEARRIVAL:
                //cout << "arrival" << endl;
                if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    PDEV_BROADCAST_DEVICEINTERFACE lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
                    if (DeviceNotifier::pCallbackFunction)
                    {
                        DeviceNotifier::pCallbackFunction(lpdbi->dbcc_name, true);
                    }
                }
                return TRUE;

            case DBT_DEVICEREMOVECOMPLETE:
                //cout << "remove" << endl;
                if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    PDEV_BROADCAST_DEVICEINTERFACE lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
                    if (DeviceNotifier::pCallbackFunction)
                    {
                        DeviceNotifier::pCallbackFunction(lpdbi->dbcc_name, false);
                    }
                }
                return TRUE;

            default:
                return TRUE;
            }
        }
    return 0;
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}

DWORD WINAPI fnThreadFunc(LPVOID lpParam)
{
    WNDCLASS wc;
    MSG msg;
    HANDLE hInstance = GetModuleHandle(NULL);
    wc.style = 0;                                   // Class style
    wc.lpfnWndProc = (WNDPROC) WndProc;             // Window procedure address
    wc.cbClsExtra = 0;                              // Class extra bytes
    wc.cbWndExtra = 0;                              // Window extra bytes
    wc.hInstance = (HINSTANCE)hInstance;            // Instance handle
    wc.hIcon = LoadIcon (NULL, IDI_WINLOGO);        // Icon handle
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);      // Cursor handle
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1); // Background color
    wc.lpszMenuName = NULL;                         // Menu name
    wc.lpszClassName = "MyWndClass";                // WNDCLASS name

    RegisterClass (&wc);


    hwnd = CreateWindow (
               "MyWndClass",               // WNDCLASS name
               "InvisibleWindowForNotifications",      // Window title
               WS_DISABLED,                // Window style
               CW_USEDEFAULT,              // Horizontal position
               CW_USEDEFAULT,              // Vertical position
               CW_USEDEFAULT,              // Initial width
               CW_USEDEFAULT,              // Initial height
               HWND_DESKTOP,               // Handle of parent window
               NULL,                       // Menu handle
               NULL,                       // Application's instance handle
               NULL                        // Window-creation data
           );

    UpdateWindow (hwnd);

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }

    return msg.wParam;
}

DeviceNotifier::DeviceNotifier()
{
    // note: failed when moved to RegisterForNotifications (invisible window must be created earlier?)
    DWORD dwThreadId;
    Thread = CreateThread(NULL , 0 , fnThreadFunc , NULL , 0 , &dwThreadId);
}

DeviceNotifier::~DeviceNotifier()
{
    TerminateThread(Thread, 0);
}

int DeviceNotifier::RegisterForNotifications(GUID hidGuid/*, HANDLE deviceHandle*/)
{
    if (hwnd == NULL)
        return -2;

#if 1
    DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
    memset(&DevBroadcastDeviceInterface, 0, sizeof(DevBroadcastDeviceInterface));

    DevBroadcastDeviceInterface.dbcc_size = sizeof(DevBroadcastDeviceInterface);
    DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    DevBroadcastDeviceInterface.dbcc_classguid = hidGuid;
    HDEVNOTIFY DeviceNotificationHandle = RegisterDeviceNotification(hwnd, &DevBroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);
#else
    DEV_BROADCAST_HANDLE filter1 = {0};
    filter1.dbch_size = sizeof(filter1);
    filter1.dbch_devicetype = DBT_DEVTYP_HANDLE;
    filter1.dbch_handle = deviceHandle;
    HDEVNOTIFY DeviceNotificationHandle = RegisterDeviceNotification(hwnd, &filter1, DEVICE_NOTIFY_WINDOW_HANDLE);
#endif
    if (!DeviceNotificationHandle)
        return -3;
    return 0;
}
