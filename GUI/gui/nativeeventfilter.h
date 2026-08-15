#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QByteArray>
#include <windows.h>

class WindowsEventFilter : public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        if (eventType == "windows_generic_MSG") {
            MSG *msg = static_cast<MSG *>(message);

            // Systemmenü per Einfachklick auf Icon
            if (msg->message == WM_SYSCOMMAND && (msg->wParam & 0xFFF0) == SC_MOUSEMENU) {
                if (result) *result = 0;
                return true;
            }

            // Doppelklick auf Icon -> würde sonst schließen
            if (msg->message == WM_NCLBUTTONDBLCLK && msg->wParam == HTSYSMENU) {
                if (result) *result = 0;
                return true;
            }
        }
        return false;
    }
};

#endif // NATIVEEVENTFILTER_H