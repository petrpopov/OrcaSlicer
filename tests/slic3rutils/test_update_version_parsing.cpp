#include <catch2/catch_all.hpp>

#include "slic3r/GUI/VersionCheck.hpp"

#include "libslic3r/Semver.hpp"

using namespace Slic3r;

TEST_CASE("Release version parser accepts PE tags with and without numeric suffix", "[Updater][Version]")
{
    const auto base_release = GUI::parse_release_tag_version("2.3.2-pe");
    REQUIRE(base_release.has_value());
    REQUIRE(base_release->to_string_sf() == "2.3.2-pe");

    const auto suffixed_release = GUI::parse_release_tag_version("2.3.2-pe.7");
    REQUIRE(suffixed_release.has_value());
    REQUIRE(suffixed_release->to_string_sf() == "2.3.2-pe.7");

    const auto prefixed_release = GUI::parse_release_tag_version("v2.3.2-pe.7");
    REQUIRE(prefixed_release.has_value());
    REQUIRE(prefixed_release->to_string_sf() == "2.3.2-pe.7");

    REQUIRE(*suffixed_release > *base_release);
}

TEST_CASE("Release version parser rejects invalid PE tags", "[Updater][Version]")
{
    REQUIRE_FALSE(GUI::parse_release_tag_version("2.3").has_value());
    REQUIRE_FALSE(GUI::parse_release_tag_version("2.3.2-pe.").has_value());
    REQUIRE_FALSE(GUI::parse_release_tag_version("release-2.3.2-pe.7").has_value());
}
