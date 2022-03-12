#include "CommThread.h"
#include "..\tSIP\tSIP\phone\Phone.h"
#include "HidDevice.h"
#include "HidReportHandler.h"
#include "CustomSettings.h"
#include "Log.h"
#include <windows.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>

void Key(int keyCode, int state);

using namespace nsHidDevice;

namespace {
volatile bool connected = false;
volatile bool exited = false;


int regState = 0;
int callState = 0;
int ringState = 0;
std::string callDisplay;
bool displayUpdateFlag = false;
bool ringUpdateFlag = false;


int UpdateDisplay(HidDevice &dev) {
    int status = 0;

    displayUpdateFlag = false;
#if 0
    char sendbuf[33] =  {
        0x03, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00
    };

    FillDisplayBuf(sendbuf, line1, line2, extras);

    status = dev.WriteReport(HidDevice::E_REPORT_OUT, 0, sendbuf, sizeof(sendbuf));
#endif
    return status;
}

int UpdateRing(HidDevice &dev) {
    int status;

    ringUpdateFlag = false;

    unsigned char sendbuf[33] = { 0x01, 0xFF, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    LOG("UpdateRing, ringState = %d", ringState);

    // ring ON
    if (ringState) {

    } else {
        sendbuf[1] = 0x00;
    }

    // something fishy - device often not responding correctly without pause
    Sleep(30);
    status = dev.WriteReport(HidDevice::E_REPORT_OUT, 0, sendbuf, sizeof(sendbuf));
    Sleep(30);
    return status;
}

};

DWORD WINAPI CommThreadProc(LPVOID data) {
    //ThreadComm *thrRead = (ThreadComm*)data;
    HidDevice hidDevice;
    bool devConnected = false;
    unsigned int loopCnt = 0;

    unsigned char rcvbuf[17];
    LOG("Running comm thread");

    while (connected) {
        if (devConnected == false) {
            if (loopCnt % 100 == 0) {
                int status = hidDevice.Open(customSettings.usbVid, customSettings.usbPid, NULL, NULL, customSettings.usbUsagePage);
                if (status == 0) {
                    devConnected = true;
                    LOG("Device connected (USB VID 0x%04X, PID 0x%04X, usage page 0x%X)",
                        static_cast<int>(customSettings.usbVid), static_cast<int>(customSettings.usbPid), static_cast<int>(customSettings.usbUsagePage));
                    //LOG("  devConnected: %d", (int)devConnected);
                    std::string dump;
                    hidDevice.DumpCapabilities(dump);
                    LOG("USB HID capabilities: %s\n", dump.c_str());
                } else {
                    LOG("Error opening USB device (USB VID 0x%04X, PID 0x%04X, usage page 0x%X): %s",
                        static_cast<int>(customSettings.usbVid), static_cast<int>(customSettings.usbPid), static_cast<int>(customSettings.usbUsagePage),
                        HidDevice::GetErrorDesc(status).c_str());
                }
            }
        } else {
        #if 0
            int status = 0;
            if (callState == 0) {
                if ((loopCnt & 0x03) == 0) {
                    displayUpdateFlag = true;
                }
            }
            if (displayUpdateFlag) {
                status = UpdateDisplay(hidDevice);
            }
            if (status == 0 && ringUpdateFlag) {
                status = UpdateRing(hidDevice);
            }
            if (status) {
                LOG("Device: error updating, %s", HidDevice::GetErrorDesc(status).c_str());
                hidDevice.Close();
                devConnected = false;
                lastRcvFilled = false;
            } else
        #endif
            {
                int size = sizeof(rcvbuf);
                memset(rcvbuf, 0, sizeof(rcvbuf));
                //LOG("%03d  devConnected: %d, size = %d", __LINE__, (int)devConnected, size);
                int status = hidDevice.ReadReport(HidDevice::E_REPORT_IN, 0, rcvbuf, &size, 100);
                //LOG("%03d  devConnected: %d, size = %d", __LINE__, (int)devConnected, size);
                if (status == 0) {
                    if (customSettings.logReceivedHidReports) {
                        std::string hex;
                        for (int i=0; i<size; i++) {
                            char str[16];
                            snprintf(str, sizeof(str), "0x%02X ", rcvbuf[i]);
                            hex += str;
                        }
                        LOG("HID REPORT_IN received %d B: %s", size, hex.c_str());
                    }
                    ProcessReceivedReport(rcvbuf, size);
                } else if (status != HidDevice::E_ERR_TIMEOUT) {
                    LOG("USB device: error reading report");
                    hidDevice.Close();
                    devConnected = false;
                    //lastRcvFilled = false;
                }
            }
        }
        loopCnt++;
        Sleep(50);
    }

    // clear display
    if (devConnected) {
        unsigned char sendbuf[33] =  {
            0x03, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00
        };
        hidDevice.WriteReport(HidDevice::E_REPORT_OUT, 0, sendbuf, sizeof(sendbuf));
    }

    hidDevice.Close();
    exited = true;
    return 0;
}


int CommThreadStart(void) {
    DWORD dwtid;
    exited = false;
    connected = true;
    HANDLE CommThread = CreateThread(NULL, 0, CommThreadProc, /*this*/NULL, 0, &dwtid);
    if (CommThread == NULL) {
        connected = false;
        exited = true;
    }

    return 0;
}

int CommThreadStop(void) {
    connected = false;
    while (!exited) {
        Sleep(50);
    }
    return 0;
}

void UpdateRegistrationState(int state) {
    regState = state;
    //LOG("regState = %d", regState);
    displayUpdateFlag = true;
}

void UpdateCallState(int state, const char* display) {
    callState = state;
    callDisplay = display;
    displayUpdateFlag = true;
    //LOG("updateCallState: state = %d, display = %s", state, display);
}

void UpdateRing(int state) {
    if (ringState != state) {
        ringState = state;
        ringUpdateFlag = true;
    }
    //LOG("ringState = %d", ringState);
}
