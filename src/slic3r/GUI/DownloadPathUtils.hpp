#ifndef slic3r_GUI_DownloadPathUtils_hpp_
#define slic3r_GUI_DownloadPathUtils_hpp_

#include <string>

class wxString;

namespace Slic3r::GUI {

std::string build_downloads_path_from_home(const wxString& home_dir);
std::string default_downloads_path();

} // namespace Slic3r::GUI

#endif // slic3r_GUI_DownloadPathUtils_hpp_
