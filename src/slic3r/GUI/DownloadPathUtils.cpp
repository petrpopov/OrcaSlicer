#include "DownloadPathUtils.hpp"

#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace Slic3r::GUI {

std::string build_downloads_path_from_home(const wxString& home_dir)
{
    wxFileName downloads_dir(home_dir, "");
    downloads_dir.AppendDir("Downloads");
    return downloads_dir.GetPath().ToUTF8().data();
}

std::string default_downloads_path()
{
#ifdef __APPLE__
    return build_downloads_path_from_home(wxFileName::GetHomeDir());
#else
    return wxStandardPaths::Get().GetUserDir(wxStandardPaths::Dir_Downloads).ToUTF8().data();
#endif
}

} // namespace Slic3r::GUI
