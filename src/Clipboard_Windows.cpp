#include "Clipboard.h"

#include <windows.h>

#include <vector>

namespace ccc {

bool setClipboardText(const std::string& utf8Text) {
    // The Windows clipboard's text format (CF_UNICODETEXT) is UTF-16, so
    // we need to convert from the UTF-8 we use internally.
    const int wideLen =
        MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, nullptr, 0);
    if (wideLen <= 0) {
        return false;
    }

    std::vector<wchar_t> wideBuffer(static_cast<std::size_t>(wideLen));
    MultiByteToWideChar(CP_UTF8, 0, utf8Text.c_str(), -1, wideBuffer.data(), wideLen);

    if (!OpenClipboard(nullptr)) {
        return false;
    }

    EmptyClipboard();

    const SIZE_T byteSize = static_cast<SIZE_T>(wideLen) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, byteSize);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    void* dest = GlobalLock(hMem);
    if (!dest) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    memcpy(dest, wideBuffer.data(), byteSize);
    GlobalUnlock(hMem);

    // Ownership of hMem transfers to the system on success; we must NOT
    // free it ourselves in that case.
    const bool ok = SetClipboardData(CF_UNICODETEXT, hMem) != nullptr;
    if (!ok) {
        GlobalFree(hMem);
    }

    CloseClipboard();
    return ok;
}

}  // namespace ccc
