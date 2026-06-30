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

TEST_CASE("Bambuddy page URL joins base and page with Pangolin query token", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.base_url = "https://bambuddy.ezheg.xyz/";
    REQUIRE(Slic3r::BambuddyClient::build_page_url(cfg, "archives") == "https://bambuddy.ezheg.xyz/archives");

    cfg.base_url = "https://bambuddy.ezheg.xyz?foo=bar";
    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::PangolinQueryToken;
    cfg.proxy_auth.pangolin_query_token = "id.token";

    const std::string url = Slic3r::BambuddyClient::build_page_url(cfg, "/queue");
    REQUIRE(url == "https://bambuddy.ezheg.xyz/queue?foo=bar");
    REQUIRE(url.find("p_token=") == std::string::npos);

    cfg.base_url = "https://bambuddy.ezheg.xyz/";
    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::PangolinHeaders;
    cfg.proxy_auth.pangolin_token_id = "id";
    cfg.proxy_auth.pangolin_token_secret = "secret";
    REQUIRE(Slic3r::BambuddyClient::build_page_url(cfg, "queue") ==
            "https://bambuddy.ezheg.xyz/queue");
}

TEST_CASE("Bambuddy headers include API key and Pangolin header token", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.api_key = "bb_secret";
    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::PangolinHeaders;
    cfg.proxy_auth.pangolin_token_id = "id1";
    cfg.proxy_auth.pangolin_token_secret = "token1";

    const auto headers = Slic3r::BambuddyClient::build_headers(cfg);
    REQUIRE(headers.at("X-API-Key") == "bb_secret");
    REQUIRE(headers.at("P-Access-Token-Id") == "id1");
    REQUIRE(headers.at("P-Access-Token") == "token1");
}

TEST_CASE("Bambuddy custom headers are applied only in custom header mode", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.proxy_auth.custom_headers.emplace("CF-Access-Client-Id", "client-id");

    REQUIRE(Slic3r::BambuddyClient::build_headers(cfg).count("CF-Access-Client-Id") == 0);

    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::CustomHeaders;
    const auto headers = Slic3r::BambuddyClient::build_headers(cfg);
    REQUIRE(headers.at("CF-Access-Client-Id") == "client-id");
}

TEST_CASE("Bambuddy detects reverse proxy HTML login", "[BambuddyClient]")
{
    REQUIRE(Slic3r::BambuddyClient::looks_like_html_login("<!DOCTYPE html><html><body>login</body></html>", "text/html"));
    REQUIRE(Slic3r::BambuddyClient::looks_like_html_login("   <html><body>login</body></html>", ""));
    REQUIRE_FALSE(Slic3r::BambuddyClient::looks_like_html_login("{\"id\":1}", "application/json"));
}

TEST_CASE("Bambuddy parses printer list", "[BambuddyClient]")
{
    const std::string body = R"([{"id":5,"name":"X1C","model":"X1C","is_active":true}])";
    std::vector<Slic3r::BambuddyPrinter> printers;
    std::string error;

    REQUIRE(Slic3r::BambuddyClient::parse_printers_response(body, printers, error));
    REQUIRE(printers.size() == 1);
    REQUIRE(printers[0].id == 5);
    REQUIRE(printers[0].name == "X1C");
    REQUIRE(printers[0].model == "X1C");
    REQUIRE(printers[0].is_active);
}

TEST_CASE("Bambuddy parses upload response id", "[BambuddyClient]")
{
    Slic3r::BambuddyUploadResult result;
    std::string error;

    REQUIRE(Slic3r::BambuddyClient::parse_upload_response("{\"id\":123,\"filename\":\"a.gcode.3mf\"}", result, error));
    REQUIRE(result.library_file_id == 123);
    REQUIRE(result.filename == "a.gcode.3mf");
}

TEST_CASE("Bambuddy queue payload uses library file and printer", "[BambuddyClient]")
{
    Slic3r::BambuddyPrintOptions options;
    options.timelapse = true;

    const std::string body = Slic3r::BambuddyClient::build_queue_body(123, 5, options);
    REQUIRE(body.find("\"library_file_id\":123") != std::string::npos);
    REQUIRE(body.find("\"printer_id\":5") != std::string::npos);
    REQUIRE(body.find("\"manual_start\":false") != std::string::npos);
    REQUIRE(body.find("\"timelapse\":true") != std::string::npos);
}

TEST_CASE("Bambuddy upload filename normalizes Orca 3MF export", "[BambuddyClient]")
{
    REQUIRE(Slic3r::BambuddyClient::upload_filename_for_path("/tmp/v_ahue_plate_1.3mf") == "v_ahue_plate_1.gcode.3mf");
    REQUIRE(Slic3r::BambuddyClient::upload_filename_for_path("/tmp/v_ahue_plate_1.gcode.3mf") == "v_ahue_plate_1.gcode.3mf");
}

TEST_CASE("Bambuddy custom headers serialize round trip", "[BambuddyClient]")
{
    std::map<std::string, std::string> headers;
    headers.emplace("CF-Access-Client-Id", "client-id");
    headers.emplace("CF-Access-Client-Secret", "client-secret");

    const std::string serialized = Slic3r::BambuddyClient::serialize_custom_headers(headers);
    const auto parsed = Slic3r::BambuddyClient::parse_custom_headers(serialized);

    REQUIRE(parsed.at("CF-Access-Client-Id") == "client-id");
    REQUIRE(parsed.at("CF-Access-Client-Secret") == "client-secret");
}
