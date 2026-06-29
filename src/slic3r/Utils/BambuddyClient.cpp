#include "BambuddyClient.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

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

} // namespace Slic3r
