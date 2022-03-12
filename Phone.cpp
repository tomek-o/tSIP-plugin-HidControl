//---------------------------------------------------------------------------

#define _EXPORTING
#include "..\tSIP\tSIP\phone\Phone.h"
#include "..\tSIP\tSIP\phone\PhoneSettings.h"
#include "..\tSIP\tSIP\phone\PhoneCapabilities.h"
#include "CommThread.h"
#include "HidDevice.h"
#include "CustomSettings.h"
#include "Log.h"
#include <assert.h>
#include <algorithm>	// needed by Utils::in_group
#include "Utils.h"
#include <string>
#include <fstream>
#include <json/json.h>

//---------------------------------------------------------------------------

static const struct S_PHONE_DLL_INTERFACE dll_interface =
{DLL_INTERFACE_MAJOR_VERSION, DLL_INTERFACE_MINOR_VERSION};

// callback ptrs
CALLBACK_LOG lpLogFn = NULL;
CALLBACK_CONNECT lpConnectFn = NULL;
CALLBACK_KEY lpKeyFn = NULL;
CALLBACK_RUN_SCRIPT_ASYNC lpRunScriptAsyncFn = NULL;

void *callbackCookie;	///< used by upper class to distinguish library instances when receiving callbacks

extern "C" __declspec(dllexport) void GetPhoneInterfaceDescription(struct S_PHONE_DLL_INTERFACE* interf) {
    interf->majorVersion = dll_interface.majorVersion;
    interf->minorVersion = dll_interface.minorVersion;
}

void Log(char* txt) {
    if (lpLogFn)
        lpLogFn(callbackCookie, txt);
}

void Connect(int state, char *szMsgText) {
    if (lpConnectFn)
        lpConnectFn(callbackCookie, state, szMsgText);
}

void Key(int keyCode, int state) {
    if (lpKeyFn)
        lpKeyFn(callbackCookie, keyCode, state);
}

int RunScriptAsync(const char* script) {
	if (lpRunScriptAsyncFn) {
		return lpRunScriptAsyncFn(callbackCookie, script);
	}
	return -1;
}

void SetCallbacks(void *cookie, CALLBACK_LOG lpLog, CALLBACK_CONNECT lpConnect, CALLBACK_KEY lpKey) {
    assert(cookie && lpLog && lpConnect && lpKey);
    lpLogFn = lpLog;
    lpConnectFn = lpConnect;
    lpKeyFn = lpKey;
    callbackCookie = cookie;
    lpLogFn(callbackCookie, "Phone DLL for EX03 loaded\n");
}

void GetPhoneCapabilities(struct S_PHONE_CAPABILITIES **caps) {
    static struct S_PHONE_CAPABILITIES capabilities = {
        0
    };
    *caps = &capabilities;
}

void ShowSettings(HANDLE parent) {
    MessageBox((HWND)parent, "No additional settings.", "Device DLL", MB_ICONINFORMATION);
}

int Connect(void) {
    CommThreadStart();
    return 0;
}

int Disconnect(void) {
    CommThreadStop();
    return 0;
}

static bool bSettingsReaded = false;

static int GetDefaultSettings(struct S_PHONE_SETTINGS* settings) {
    settings->ring = 1;

    bSettingsReaded = true;
    return 0;
}

int GetPhoneSettings(struct S_PHONE_SETTINGS* settings) {
    //settings->iTriggerSrcChannel = 0;

    std::string path = Utils::GetDllPath();
    path = Utils::ReplaceFileExtension(path, ".cfg");
    if (path == "")
        return GetDefaultSettings(settings);

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;

    std::ifstream ifs(path.c_str());
    std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    bool parsingSuccessful = reader.parse( strConfig, root );
    if ( !parsingSuccessful )
        return GetDefaultSettings(settings);

    GetDefaultSettings(settings);


	//int mode = root.get("TriggerMode", TRIGGER_MODE_AUTO).asInt();
    settings->ring = root.get("ring", settings->ring).asInt();

    customSettings.fromJson(root["customSettings"]);

    bSettingsReaded = true;
    return 0;
}

int SavePhoneSettings(struct S_PHONE_SETTINGS* settings) {
    Json::Value root;
    Json::StyledWriter writer;

    root["ring"] = settings->ring;

    customSettings.toJson(root["customSettings"]);

    std::string outputConfig = writer.write( root );

    std::string path = Utils::GetDllPath();
    path = Utils::ReplaceFileExtension(path, ".cfg");
    if (path == "")
        return -1;

    std::ofstream ofs(path.c_str());
    ofs << outputConfig;
    ofs.close();

    return 0;
}

int SetRegistrationState(int state) {
    UpdateRegistrationState(state);
    return 0;
}

int SetCallState(int state, const char* display) {
    UpdateCallState(state, display);
    return 0;
}

int Ring(int state) {
    UpdateRing(state);
    return 0;
}

void SetRunScriptAsyncCallback(CALLBACK_RUN_SCRIPT_ASYNC lpRunScriptAsync) {
	lpRunScriptAsyncFn = lpRunScriptAsync;
}
