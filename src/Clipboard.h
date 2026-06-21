#pragma once

#include <string>

namespace ccc {

// Copies `utf8Text` to the system clipboard as plain text.
// Returns true on success, false if the clipboard could not be accessed
// (e.g. no clipboard tool available on Linux, or the API call failed on
// Windows). Implemented separately per-platform - see Clipboard_Windows.cpp
// and Clipboard_Linux.cpp.
bool setClipboardText(const std::string& utf8Text);

}  // namespace ccc
