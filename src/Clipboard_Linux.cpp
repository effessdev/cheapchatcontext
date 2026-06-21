#include "Clipboard.h"

#include <csignal>
#include <cstdio>

namespace ccc {

namespace {

// Pipes `text` into `command`'s stdin. Returns true if the command was
// found and exited successfully. stderr is silenced so that probing for
// missing tools (e.g. no xclip installed) doesn't spam the console.
bool pipeToCommand(const std::string& command, const std::string& text) {
    // If `command` doesn't exist, the shell still opens the pipe and
    // exits immediately. Writing to that already-closed pipe raises
    // SIGPIPE, whose default action kills our whole process - not just
    // this attempt. Ignore it for the duration of the write so a failed
    // attempt just shows up as a write error instead.
    struct sigaction oldAction{};
    struct sigaction ignoreAction{};
    ignoreAction.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &ignoreAction, &oldAction);

    const std::string fullCommand = command + " 2>/dev/null";
    FILE* pipe = popen(fullCommand.c_str(), "w");
    bool ok = false;
    if (pipe) {
        bool writeOk = true;
        if (!text.empty()) {
            writeOk = std::fwrite(text.data(), 1, text.size(), pipe) == text.size();
        }
        const int rc = pclose(pipe);
        ok = writeOk && (rc == 0);
    }

    sigaction(SIGPIPE, &oldAction, nullptr);
    return ok;
}

}  // namespace

bool setClipboardText(const std::string& utf8Text) {
    // Try, in order: Wayland's clipboard tool, then the two most common
    // X11 ones. Whichever is installed wins; we don't try to detect the
    // session type ourselves.
    if (pipeToCommand("wl-copy", utf8Text)) return true;
    if (pipeToCommand("xclip -selection clipboard", utf8Text)) return true;
    if (pipeToCommand("xsel --clipboard --input", utf8Text)) return true;
    return false;
}

}  // namespace ccc
