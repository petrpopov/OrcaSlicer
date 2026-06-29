#ifndef slic3r_GUI_BambuddySettingsDialog_hpp_
#define slic3r_GUI_BambuddySettingsDialog_hpp_

#include "GUI_Utils.hpp"

#include "slic3r/Utils/BambuddyClient.hpp"

#include <wx/dialog.h>

class wxCheckBox;
class wxChoice;
class wxTextCtrl;

namespace Slic3r {
namespace GUI {

class BambuddySettingsDialog : public DPIDialog
{
public:
    explicit BambuddySettingsDialog(wxWindow *parent);

    const BambuddyConfig &config() const { return m_config; }

private:
    void build_ui();
    void transfer_config_to_controls();
    void transfer_controls_to_config();
    void update_proxy_controls();
    void populate_printers(const std::vector<BambuddyPrinter> &printers);

    void on_refresh_printers(wxCommandEvent &event);
    void on_test_connection(wxCommandEvent &event);
    void on_ok(wxCommandEvent &event);

    BambuddyConfig              m_config;
    std::vector<BambuddyPrinter> m_printers;

    wxCheckBox *m_enabled{nullptr};
    wxTextCtrl *m_base_url{nullptr};
    wxTextCtrl *m_api_key{nullptr};
    wxChoice *  m_printer_choice{nullptr};
    wxTextCtrl *m_printer_id{nullptr};
    wxChoice *  m_proxy_mode{nullptr};
    wxTextCtrl *m_pangolin_token_id{nullptr};
    wxTextCtrl *m_pangolin_token_secret{nullptr};
    wxTextCtrl *m_pangolin_query_token{nullptr};
    wxTextCtrl *m_custom_headers{nullptr};
};

} // namespace GUI
} // namespace Slic3r

#endif // slic3r_GUI_BambuddySettingsDialog_hpp_
