#include "VersionCheck.hpp"

#include "libslic3r/Semver.hpp"

#include <regex>

namespace Slic3r
{
namespace GUI
{

boost::optional<Semver> parse_release_tag_version(const std::string &raw_tag)
{
    std::string tag = raw_tag;
    if (!tag.empty() && (tag.front() == 'v' || tag.front() == 'V'))
        tag.erase(0, 1);

    static const std::regex release_version_regex(
        "[0-9]+\\.[0-9]+(\\.[0-9]+)*(?:-[A-Za-z0-9]+(?:\\.[A-Za-z0-9]+)*)?(?:\\+[A-Za-z0-9]+(?:\\.[A-Za-z0-9]+)*)?");

    if (!std::regex_match(tag, release_version_regex))
        return boost::none;

    return Semver::parse(tag);
}

}
}
