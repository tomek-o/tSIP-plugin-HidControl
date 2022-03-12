/** \file
    \brief Simplified interface for HID device
    \note Module was tested with Generic HID from MCHPUSB 2.5 demo project running on PIC18F2550.
    Feature report reading/writing was not tested.

    \note Communicating with mouse/keyboard (quote from http://www.lvr.com/hidfaq.htm)

    Why do I receive "Access denied" when attempting to access my HID?

    Windows 2000 and later have exclusive read/write access to HIDs that are configured as a system keyboards or mice.
    An application can obtain a handle to a system keyboard or mouse by not requesting READ or WRITE access with CreateFile.
    Applications can then use HidD_SetFeature and HidD_GetFeature (if the device supports Feature reports).
*/

#ifndef HIDDEVICE_H_INCLUDED
#define HIDDEVICE_H_INCLUDED

#include <windef.h>
#include <string>

namespace nsHidDevice {

    class HidDevice {
    private:
        HANDLE handle;
        HANDLE readHandle;
        HANDLE writeHandle;
        HANDLE hEventObject;
        void* pOverlapped;
        GUID hidGuid;        /* GUID for HID driver */
        std::string path;

        int CreateReadWriteHandles(std::string path);

        HidDevice(const HidDevice& source) {};
        HidDevice& operator=(const HidDevice&);
        static void UnicodeToAscii(char *buffer);

    public:
        enum E_ERROR
        {
            E_ERR_NONE = 0,
            E_ERR_INV_PARAM,
            E_ERR_NOTFOUND,
            E_ERR_IO,
            E_ERR_TIMEOUT,
            E_ERR_OTHER,
            E_ERR_LIMIT // dummy
        };
        typedef struct
        {
            int nCode;
            const char *lpName;
        } SERROR;
        static SERROR tabErrorsName[E_ERR_LIMIT];
        static std::string GetErrorDesc(int ErrorCode);

        enum E_REPORT_TYPE
        {
            E_REPORT_IN,
            E_REPORT_OUT,
            E_REPORT_FEATURE
        };

        HidDevice();
        ~HidDevice();

        void GetHidGuid(GUID *guid);
        HANDLE GetHandle(void) {
            return handle;
        }

        std::string GetPath(void) {
            return path;
        }


        /** \brief Search and open device with specified parameters
            \param VID required Vendor ID
            \param PID required Product ID
            \param vendorName required vendor name string, ignored if NULL
            \param productName required product name string, ignored if NULL
            \return 0 on success
        */
        int Open(int VID, int PID, char *vendorName, char *productName, int usagePage);

        /** \brief Dump device capabilities as text
            \return 0 on success
        */
        int DumpCapabilities(std::string &dump);

        /** \brief Write report to device
            \return 0 on success
        */
        int WriteReport(enum E_REPORT_TYPE type, int id, unsigned char *buffer, int len);

        /** \brief Read report from device
            \param timeout operation timeout [ms]
            \return 0 on success
        */
        int ReadReport(enum E_REPORT_TYPE type, int id, unsigned char *buffer, int *len, int timeout);

        /** \brief Close connection to device
        */
        void Close(void);
    };

};

#endif // HIDDEVICE_H_INCLUDED
