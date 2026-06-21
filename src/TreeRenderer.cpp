#include "TreeRenderer.h"

#include <sstream>

namespace ccc {

namespace {

void renderChildren(const std::vector<FileNode>& children, const std::string& prefix,
                     std::ostringstream& out) {
    for (std::size_t i = 0; i < children.size(); ++i) {
        const bool last = (i + 1 == children.size());
        const FileNode& child = children[i];

        out << prefix << (last ? "`-- " : "|-- ") << child.name << (child.isDirectory ? "/" : "")
            << "\n";

        if (child.isDirectory) {
            renderChildren(child.children, prefix + (last ? "    " : "|   "), out);
        }
    }
}

}  // namespace

std::string renderTree(const FileNode& root, const std::string& rootLabel) {
    std::ostringstream out;
    out << rootLabel << "/\n";
    renderChildren(root.children, "", out);
    return out.str();
}

}  // namespace ccc
