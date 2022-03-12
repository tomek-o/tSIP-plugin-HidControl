#include "CustomSettings.h"
#include <json/json.h>

CustomSettings customSettings;


CustomSettings::CustomSettings(void):
    usbVid(0x1395),
    usbPid(0x0026),
    usbUsagePage(0xb),
    logReceivedHidReports(true)
{
    {
        // answer/hangup button
        struct ReportAction ra;
        ra.report.push_back(0x01);
        ra.script =
            "local callState = GetCallState()\r\n"
            "if callState == 1 then\t-- INCOMING\r\n"
            "\tAnswer()\r\n"
            "else\r\n"
            "\tHangup()\r\n"
            "end";
        reportActions.push_back(ra);
    }
}

void CustomSettings::fromJson(const Json::Value &jv)
{
    jv.getUInt("usbVid", usbVid);
    jv.getUInt("usbPid", usbPid);
    jv.getInt("usbUsagePage", usbUsagePage);
    jv.getBool("logReceivedHidReports", logReceivedHidReports);
}

void CustomSettings::toJson(Json::Value &jv)
{
    jv["usbVid"] = usbVid;
    jv["usbPid"] = usbPid;
    jv["usbUsagePage"] = usbUsagePage;
    jv["logReceivedHidReports"] = logReceivedHidReports;
    {
        Json::Value &jReportActions = jv["reportActions"];
        jReportActions = Json::Value(Json::arrayValue);
        for (unsigned int i=0; i<reportActions.size(); i++)
        {
            const struct ReportAction &r = reportActions[i];
            Json::Value &jr = jReportActions.append(Json::Value(Json::objectValue));
            jr["report"] = Json::Value(Json::arrayValue);
            for (unsigned int j=0; j<r.report.size(); j++)
            {
                jr["report"].append(r.report[j]);
            }
            jr["script"] = r.script;
        }
    }
}
