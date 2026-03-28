#ifndef slic3r_GUI_VersionCheck_hpp_
#define slic3r_GUI_VersionCheck_hpp_

#include <boost/optional.hpp>

#include <string>

namespace Slic3r
{

class Semver;

namespace GUI
{

boost::optional<Semver> parse_release_tag_version(const std::string &tag);

}
}

#endif
