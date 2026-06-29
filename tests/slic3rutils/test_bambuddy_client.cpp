#include "slic3r/Utils/BambuddyClient.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Bambuddy API URL joins base and endpoint", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.base_url = "https://bambuddy.ezheg.xyz/";

    REQUIRE(Slic3r::BambuddyClient::build_api_url(cfg, "/library/files") ==
            "https://bambuddy.ezheg.xyz/api/v1/library/files");
}

TEST_CASE("Bambuddy Pangolin query token preserves existing query", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.base_url = "https://bambuddy.ezheg.xyz?foo=bar";
    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::PangolinQueryToken;
    cfg.proxy_auth.pangolin_query_token = "secret";

    const std::string url = Slic3r::BambuddyClient::build_api_url(cfg, "/printers/");
    REQUIRE(url.find("foo=bar") != std::string::npos);
    REQUIRE(url.find("p_token=secret") != std::string::npos);
}
