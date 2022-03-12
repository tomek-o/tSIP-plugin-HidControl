#ifndef CustomSettingsH
#define CustomSettingsH

#include <vector>
#include <string>

namespace Json
{
    class Value;
}

struct ReportAction
{
    std::vector<unsigned char> report;
    std::string script;
};

class CustomSettings
{
public:
    CustomSettings(void);
    void fromJson(const Json::Value &jv);
    void toJson(Json::Value &jv);

    unsigned int usbVid;
    unsigned int usbPid;
    int usbUsagePage;
    bool logReceivedHidReports;
    std::vector<ReportAction> reportActions;
};

extern CustomSettings customSettings;

#endif // CustomSettingsH
