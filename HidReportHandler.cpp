#include "HidReportHandler.h"
#include "CustomSettings.h"
#include "Log.h"
#include <string.h>

int RunScriptAsync(const char* script);

void ProcessReceivedReport(unsigned char* buffer, int size)
{
    if (size <= 0)
        return;
    for (unsigned int i=0; i<customSettings.reportActions.size(); i++)
    {
        const struct ReportAction &ra = customSettings.reportActions[i];
        if (size >= static_cast<int>(ra.report.size())) {
            if (memcmp(buffer, &ra.report[0], ra.report.size()) == 0) {
                LOG("Running script #%d:\n%s\n", i, ra.script.c_str());
                RunScriptAsync(ra.script.c_str());
                break;
            }
        }
    }
}
