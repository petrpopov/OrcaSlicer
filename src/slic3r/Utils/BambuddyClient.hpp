#ifndef slic3r_BambuddyClient_hpp_
#define slic3r_BambuddyClient_hpp_

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

namespace Slic3r {

enum class BambuddyProxyAuthMode { None, PangolinHeaders, PangolinQueryToken, CustomHeaders };

struct BambuddyProxyAuthConfig
{
    BambuddyProxyAuthMode            mode{BambuddyProxyAuthMode::None};
    std::string                      pangolin_token_id;
    std::string                      pangolin_token_secret;
    std::string                      pangolin_query_token;
    std::map<std::string, std::string> custom_headers;
};

struct BambuddyConfig
{
    bool                      enabled{false};
    std::string               base_url;
    std::string               api_key;
    int                       default_printer_id{0};
    std::string               default_printer_name;
    BambuddyProxyAuthConfig   proxy_auth;
};

struct BambuddyPrinter
{
    int         id{0};
    std::string name;
    std::string model;
    bool        is_active{true};
};

struct BambuddyUploadResult
{
    int         library_file_id{0};
    std::string filename;
};

struct BambuddyQueueResult
{
    int queue_item_id{0};
};

struct BambuddyPrintOptions
{
    bool insert_at_top{true};
    bool manual_start{false};
    bool bed_levelling{true};
    bool flow_cali{false};
    bool vibration_cali{true};
    bool layer_inspect{false};
    bool timelapse{false};
    bool use_ams{true};
};

class BambuddyClient
{
public:
    explicit BambuddyClient(BambuddyConfig config);

    static std::string build_api_url(const BambuddyConfig &config, const std::string &api_path);
    static std::map<std::string, std::string> build_headers(const BambuddyConfig &config);
    static bool looks_like_html_login(const std::string &body, const std::string &content_type = {});

    bool test_connection(std::string &error) const;
    bool list_printers(std::vector<BambuddyPrinter> &printers, std::string &error) const;
    bool upload_file(const boost::filesystem::path &path, BambuddyUploadResult &result, std::string &error) const;
    bool enqueue_print(int library_file_id, int printer_id, const BambuddyPrintOptions &options, BambuddyQueueResult &result,
                       std::string &error) const;

private:
    BambuddyConfig m_config;
};

} // namespace Slic3r

#endif // slic3r_BambuddyClient_hpp_
