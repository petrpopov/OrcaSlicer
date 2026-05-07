#include <catch2/catch_all.hpp>

#include "slic3r/GUI/DownloadPathUtils.hpp"

#include <wx/filename.h>

using namespace Slic3r::GUI;

TEST_CASE("Download path helper appends Downloads to the home directory", "[GUI][Downloads]")
{
    const wxString home_dir = wxFileName::GetHomeDir();
    REQUIRE(!home_dir.empty());

    wxFileName expected_path(home_dir, "");
    expected_path.AppendDir("Downloads");
    const std::string expected = expected_path.GetPath().ToStdString();
    REQUIRE(build_downloads_path_from_home(home_dir) == expected);
}

TEST_CASE("Default download path avoids wxStandardPaths Downloads lookup on macOS", "[GUI][Downloads]")
{
#ifdef __APPLE__
    REQUIRE(default_downloads_path() == build_downloads_path_from_home(wxFileName::GetHomeDir()));
#else
    REQUIRE_FALSE(default_downloads_path().empty());
#endif
}
