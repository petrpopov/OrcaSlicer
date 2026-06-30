#include "BambuddyClient.hpp"

#include "Http.hpp"

#include "libslic3r/AppConfig.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <nlohmann/json.hpp>

namespace Slic3r {
namespace {

std::string trim_copy(std::string value)
{
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string lowercase_copy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool ends_with(const std::string &value, const std::string &suffix)
{
    return value.size() >= suffix.size() && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool starts_with(const std::string &value, const std::string &prefix)
{
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}


void apply_headers(Http &http, const BambuddyConfig &config)
{
    for (const auto &[name, value] : BambuddyClient::build_headers(config))
        http.header(name, value);
}

std::string http_error_message(const std::string &body, const std::string &error, unsigned status)
{
    if (BambuddyClient::looks_like_html_login(body))
        return "Reverse proxy authentication failed; check Pangolin/custom auth settings.";
    if (status == 401 || status == 403)
        return "Bambuddy rejected the API key or permissions.";
    if (!error.empty())
        return error;
    if (status != 0)
        return "HTTP " + std::to_string(status) + ": " + body;
    return body.empty() ? std::string("Bambuddy request failed.") : body;
}

std::string percent_encode(const std::string &value)
{
    std::ostringstream out;
    out << std::uppercase << std::hex;
    for (const unsigned char c : value) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            out << static_cast<char>(c);
        } else {
            out << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
    }
    return out.str();
}

} // namespace

std::string BambuddyClient::build_api_url(const BambuddyConfig &config, const std::string &api_path)
{
    std::string url = trim_copy(config.base_url);

    std::string fragment;
    if (const size_t hash_pos = url.find('#'); hash_pos != std::string::npos) {
        fragment = url.substr(hash_pos);
        url.erase(hash_pos);
    }

    std::string query;
    if (const size_t query_pos = url.find('?'); query_pos != std::string::npos) {
        query = url.substr(query_pos + 1);
        url.erase(query_pos);
    }

    while (!url.empty() && url.back() == '/')
        url.pop_back();

    if (!ends_with(url, "/api/v1"))
        url += "/api/v1";

    if (!api_path.empty() && api_path.front() == '/')
        url += api_path;
    else
        url += "/" + api_path;

    if (config.proxy_auth.mode == BambuddyProxyAuthMode::PangolinQueryToken && !config.proxy_auth.pangolin_query_token.empty()) {
        if (!query.empty())
            query += '&';
        query += "p_token=" + percent_encode(config.proxy_auth.pangolin_query_token);
    }

    if (!query.empty())
        url += "?" + query;
    url += fragment;
    return url;
}

std::string BambuddyClient::build_page_url(const BambuddyConfig &config, const std::string &page_path)
{
    std::string url = trim_copy(config.base_url);

    std::string fragment;
    if (const size_t hash_pos = url.find('#'); hash_pos != std::string::npos) {
        fragment = url.substr(hash_pos);
        url.erase(hash_pos);
    }

    std::string query;
    if (const size_t query_pos = url.find('?'); query_pos != std::string::npos) {
        query = url.substr(query_pos + 1);
        url.erase(query_pos);
    }

    while (!url.empty() && url.back() == '/')
        url.pop_back();

    if (!page_path.empty() && page_path.front() == '/')
        url += page_path;
    else
        url += "/" + page_path;

    std::string browser_query_token;
    if (config.proxy_auth.mode == BambuddyProxyAuthMode::PangolinQueryToken) {
        browser_query_token = config.proxy_auth.pangolin_query_token;
    } else if (config.proxy_auth.mode == BambuddyProxyAuthMode::PangolinHeaders &&
               !config.proxy_auth.pangolin_token_id.empty() && !config.proxy_auth.pangolin_token_secret.empty()) {
        browser_query_token = config.proxy_auth.pangolin_token_id + "." + config.proxy_auth.pangolin_token_secret;
    }

    if (!browser_query_token.empty()) {
        if (!query.empty())
            query += '&';
        query += "p_token=" + percent_encode(browser_query_token);
    }

    if (!query.empty())
        url += "?" + query;
    url += fragment;
    return url;
}

std::map<std::string, std::string> BambuddyClient::build_headers(const BambuddyConfig &config)
{
    std::map<std::string, std::string> headers;
    if (!config.api_key.empty())
        headers.emplace("X-API-Key", config.api_key);

    switch (config.proxy_auth.mode) {
    case BambuddyProxyAuthMode::PangolinHeaders:
        if (!config.proxy_auth.pangolin_token_id.empty())
            headers.emplace("P-Access-Token-Id", config.proxy_auth.pangolin_token_id);
        if (!config.proxy_auth.pangolin_token_secret.empty())
            headers.emplace("P-Access-Token", config.proxy_auth.pangolin_token_secret);
        break;
    case BambuddyProxyAuthMode::CustomHeaders:
        for (const auto &[name, value] : config.proxy_auth.custom_headers)
            if (!name.empty())
                headers[name] = value;
        break;
    case BambuddyProxyAuthMode::None:
    case BambuddyProxyAuthMode::PangolinQueryToken:
        break;
    }

    return headers;
}

bool BambuddyClient::looks_like_html_login(const std::string &body, const std::string &content_type)
{
    const std::string content_type_lc = lowercase_copy(content_type);
    if (content_type_lc.find("text/html") != std::string::npos)
        return true;

    const std::string trimmed = lowercase_copy(trim_copy(body));
    return starts_with(trimmed, "<!doctype html") || starts_with(trimmed, "<html");
}



std::string BambuddyClient::serialize_custom_headers(const std::map<std::string, std::string> &headers)
{
    nlohmann::json json = nlohmann::json::object();
    for (const auto &[name, value] : headers) {
        if (!name.empty())
            json[name] = value;
    }
    return json.dump();
}

std::map<std::string, std::string> BambuddyClient::parse_custom_headers(const std::string &serialized)
{
    std::map<std::string, std::string> headers;
    if (trim_copy(serialized).empty())
        return headers;

    auto json = nlohmann::json::parse(serialized, nullptr, false);
    if (json.is_discarded() || !json.is_object())
        return headers;

    for (auto it = json.begin(); it != json.end(); ++it) {
        if (it.value().is_string())
            headers.emplace(it.key(), it.value().get<std::string>());
    }
    return headers;
}

std::string BambuddyClient::proxy_auth_mode_to_string(BambuddyProxyAuthMode mode)
{
    switch (mode) {
    case BambuddyProxyAuthMode::PangolinHeaders:    return "pangolin_headers";
    case BambuddyProxyAuthMode::PangolinQueryToken: return "pangolin_query";
    case BambuddyProxyAuthMode::CustomHeaders:      return "custom_headers";
    case BambuddyProxyAuthMode::None:               return "none";
    }
    return "none";
}

BambuddyProxyAuthMode BambuddyClient::proxy_auth_mode_from_string(const std::string &value)
{
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "pangolin_headers")
        return BambuddyProxyAuthMode::PangolinHeaders;
    if (normalized == "pangolin_query")
        return BambuddyProxyAuthMode::PangolinQueryToken;
    if (normalized == "custom_headers")
        return BambuddyProxyAuthMode::CustomHeaders;
    return BambuddyProxyAuthMode::None;
}

BambuddyConfig BambuddyClient::load_from_app_config(const AppConfig &app_config)
{
    constexpr const char *section = "bambuddy";
    BambuddyConfig config;
    config.enabled = app_config.get_bool(section, "enabled");
    config.base_url = app_config.get(section, "base_url");
    config.api_key = app_config.get(section, "api_key");
    config.default_printer_name = app_config.get(section, "default_printer_name");

    const std::string default_printer_id = app_config.get(section, "default_printer_id");
    if (!default_printer_id.empty()) {
        try {
            config.default_printer_id = std::stoi(default_printer_id);
        } catch (...) {
            config.default_printer_id = 0;
        }
    }

    config.proxy_auth.mode = proxy_auth_mode_from_string(app_config.get(section, "proxy_auth_mode"));
    config.proxy_auth.pangolin_token_id = app_config.get(section, "pangolin_token_id");
    config.proxy_auth.pangolin_token_secret = app_config.get(section, "pangolin_token_secret");
    config.proxy_auth.pangolin_query_token = app_config.get(section, "pangolin_query_token");
    config.proxy_auth.custom_headers = parse_custom_headers(app_config.get(section, "custom_headers_json"));
    return config;
}

void BambuddyClient::save_to_app_config(AppConfig &app_config, const BambuddyConfig &config)
{
    constexpr const char *section = "bambuddy";
    app_config.set(section, "enabled", config.enabled);
    app_config.set(section, "base_url", config.base_url);
    app_config.set(section, "api_key", config.api_key);
    app_config.set(section, "default_printer_id", config.default_printer_id > 0 ? std::to_string(config.default_printer_id) : std::string{});
    app_config.set(section, "default_printer_name", config.default_printer_name);
    app_config.set(section, "proxy_auth_mode", proxy_auth_mode_to_string(config.proxy_auth.mode));
    app_config.set(section, "pangolin_token_id", config.proxy_auth.pangolin_token_id);
    app_config.set(section, "pangolin_token_secret", config.proxy_auth.pangolin_token_secret);
    app_config.set(section, "pangolin_query_token", config.proxy_auth.pangolin_query_token);
    app_config.set(section, "custom_headers_json", serialize_custom_headers(config.proxy_auth.custom_headers));
}


bool BambuddyClient::parse_printers_response(const std::string &body, std::vector<BambuddyPrinter> &printers, std::string &error)
{
    printers.clear();
    auto json = nlohmann::json::parse(body, nullptr, false);
    if (json.is_discarded() || !json.is_array()) {
        error = "Bambuddy returned an invalid printer list.";
        return false;
    }

    for (const auto &item : json) {
        if (!item.is_object() || !item.contains("id"))
            continue;

        BambuddyPrinter printer;
        printer.id = item.value("id", 0);
        printer.name = item.value("name", std::string{});
        printer.model = item.value("model", std::string{});
        printer.is_active = item.value("is_active", item.value("active", true));
        if (printer.name.empty())
            printer.name = "Printer " + std::to_string(printer.id);
        printers.push_back(std::move(printer));
    }

    return true;
}

bool BambuddyClient::parse_upload_response(const std::string &body, BambuddyUploadResult &result, std::string &error)
{
    result = BambuddyUploadResult{};
    auto json = nlohmann::json::parse(body, nullptr, false);
    if (json.is_discarded() || !json.is_object()) {
        error = "Bambuddy returned an invalid upload response.";
        return false;
    }

    result.library_file_id = json.value("id", 0);
    result.filename = json.value("filename", std::string{});
    if (result.library_file_id <= 0) {
        error = "Bambuddy upload response did not contain a library file id.";
        return false;
    }

    return true;
}

std::string BambuddyClient::build_queue_body(int library_file_id, int printer_id, const BambuddyPrintOptions &options)
{
    nlohmann::json body;
    body["library_file_id"] = library_file_id;
    body["printer_id"] = printer_id;
    body["insert_at_top"] = options.insert_at_top;
    body["manual_start"] = options.manual_start;
    body["bed_levelling"] = options.bed_levelling;
    body["flow_cali"] = options.flow_cali;
    body["vibration_cali"] = options.vibration_cali;
    body["layer_inspect"] = options.layer_inspect;
    body["timelapse"] = options.timelapse;
    body["use_ams"] = options.use_ams;
    return body.dump();
}

std::string BambuddyClient::upload_filename_for_path(const boost::filesystem::path &path)
{
    const std::string filename = path.filename().string();
    const std::string lower_filename = lowercase_copy(filename);
    if (ends_with(lower_filename, ".gcode.3mf"))
        return filename;
    if (ends_with(lower_filename, ".3mf"))
        return filename.substr(0, filename.size() - 4) + ".gcode.3mf";
    return {};
}

std::string BambuddyClient::upload_filename_for_path(const std::string &path)
{
    return upload_filename_for_path(boost::filesystem::path(path));
}

BambuddyClient::BambuddyClient(BambuddyConfig config) : m_config(std::move(config)) {}

bool BambuddyClient::test_connection(std::string &error) const
{
    std::vector<BambuddyPrinter> printers;
    return list_printers(printers, error);
}

bool BambuddyClient::list_printers(std::vector<BambuddyPrinter> &printers, std::string &error) const
{
    bool        success = false;
    std::string response_body;
    std::string url = build_api_url(m_config, "/printers/");

    auto http = Http::get(url);
    apply_headers(http, m_config);
    http.on_complete([&](std::string body, unsigned) {
            response_body = std::move(body);
            if (looks_like_html_login(response_body)) {
                error = "Reverse proxy authentication failed; check Pangolin/custom auth settings.";
                success = false;
                return;
            }
            success = parse_printers_response(response_body, printers, error);
        })
        .on_error([&](std::string body, std::string http_error, unsigned status) {
            error = http_error_message(body, http_error, status);
            success = false;
        })
        .perform_sync();

    return success;
}

bool BambuddyClient::upload_file(const boost::filesystem::path &path, BambuddyUploadResult &result, std::string &error, ProgressFn progress_fn) const
{
    result = BambuddyUploadResult{};
    if (path.empty() || !boost::filesystem::exists(path)) {
        error = "Print file does not exist.";
        return false;
    }

    const std::string filename = upload_filename_for_path(path);
    if (filename.empty()) {
        error = "Bambuddy requires a sliced .3mf or .gcode.3mf file.";
        return false;
    }

    bool        success = false;
    std::string url = build_api_url(m_config, "/library/files");

    auto http = Http::post(url);
    apply_headers(http, m_config);
    http.form_add_file("file", path, filename)
        .on_progress([&](Http::Progress progress, bool &) {
            if (progress_fn && progress.ultotal > 0) {
                const auto raw_percent = progress.ulnow * 100 / progress.ultotal;
                const int percent = static_cast<int>(std::min<decltype(raw_percent)>(100, raw_percent));
                progress_fn(percent);
            }
        })
        .on_complete([&](std::string body, unsigned) {
            if (looks_like_html_login(body)) {
                error = "Reverse proxy authentication failed; check Pangolin/custom auth settings.";
                success = false;
                return;
            }
            success = parse_upload_response(body, result, error);
        })
        .on_error([&](std::string body, std::string http_error, unsigned status) {
            error = http_error_message(body, http_error, status);
            success = false;
        })
        .perform_sync();

    return success;
}

bool BambuddyClient::enqueue_print(int library_file_id, int printer_id, const BambuddyPrintOptions &options, BambuddyQueueResult &result,
                                   std::string &error) const
{
    result = BambuddyQueueResult{};
    if (library_file_id <= 0) {
        error = "Missing Bambuddy library file id.";
        return false;
    }
    if (printer_id <= 0) {
        error = "Missing Bambuddy printer id.";
        return false;
    }

    bool        success = false;
    std::string url = build_api_url(m_config, "/queue/");
    std::string body = build_queue_body(library_file_id, printer_id, options);

    auto http = Http::post(url);
    apply_headers(http, m_config);
    http.header("Content-Type", "application/json")
        .set_post_body(body)
        .on_complete([&](std::string response_body, unsigned) {
            if (looks_like_html_login(response_body)) {
                error = "Reverse proxy authentication failed; check Pangolin/custom auth settings.";
                success = false;
                return;
            }

            auto json = nlohmann::json::parse(response_body, nullptr, false);
            if (!json.is_discarded() && json.is_object())
                result.queue_item_id = json.value("id", 0);
            success = true;
        })
        .on_error([&](std::string response_body, std::string http_error, unsigned status) {
            error = http_error_message(response_body, http_error, status);
            success = false;
        })
        .perform_sync();

    return success;
}

} // namespace Slic3r
