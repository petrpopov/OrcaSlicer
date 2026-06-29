#include "I18N.hpp"
#include "BambuddySettingsDialog.hpp"

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "MsgDialog.hpp"

#include "libslic3r/AppConfig.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>
#include <wx/windowptr.h>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace Slic3r {
namespace GUI {
namespace {

std::string trim_copy(std::string value)
{
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string to_utf8(const wxString &value) { return value.utf8_string(); }

wxString from_std(const std::string &value) { return wxString::FromUTF8(value.c_str()); }

std::map<std::string, std::string> parse_header_lines(const wxString &text)
{
    std::map<std::string, std::string> headers;
    std::stringstream                  stream(to_utf8(text));
    std::string                        line;
    while (std::getline(stream, line)) {
        line = trim_copy(line);
        if (line.empty() || line.front() == '#')
            continue;

        const size_t separator = line.find(':');
        if (separator == std::string::npos)
            continue;

        const std::string name = trim_copy(line.substr(0, separator));
        if (!name.empty())
            headers[name] = trim_copy(line.substr(separator + 1));
    }
    return headers;
}

wxString format_header_lines(const std::map<std::string, std::string> &headers)
{
    wxString text;
    for (const auto &[name, value] : headers) {
        text += from_std(name);
        text += ": ";
        text += from_std(value);
        text += "\n";
    }
    return text;
}

wxTextCtrl *add_text_row(wxWindow *parent, wxFlexGridSizer *grid, const wxString &label, long style = 0)
{
    auto label_ctrl = new wxStaticText(parent, wxID_ANY, label);
    grid->Add(label_ctrl, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 8);

    auto text = new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(360, -1), style);
    grid->Add(text, 1, wxEXPAND | wxBOTTOM, 8);
    return text;
}

} // namespace

BambuddySettingsDialog::BambuddySettingsDialog(wxWindow *parent)
    : DPIDialog(parent, wxID_ANY, _L("Bambuddy Settings"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    if (wxGetApp().app_config)
        m_config = BambuddyClient::load_from_app_config(*wxGetApp().app_config);

    build_ui();
    transfer_config_to_controls();
    update_proxy_controls();
    wxGetApp().UpdateDlgDarkUI(this);
}

void BambuddySettingsDialog::build_ui()
{
    SetBackgroundColour(*wxWHITE);

    auto main_sizer = new wxBoxSizer(wxVERTICAL);
    auto grid       = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1, 1);

    m_enabled = new wxCheckBox(this, wxID_ANY, _L("Enable Bambuddy"));
    grid->Add(new wxStaticText(this, wxID_ANY, wxEmptyString), 0, wxBOTTOM, 8);
    grid->Add(m_enabled, 0, wxEXPAND | wxBOTTOM, 8);

    m_base_url = add_text_row(this, grid, _L("Bambuddy URL"));
    m_base_url->SetToolTip(_L("Example: https://bambuddy.ezheg.xyz"));

    m_api_key = add_text_row(this, grid, _L("API token"), wxTE_PASSWORD);
    m_api_key->SetToolTip(_L("Bambuddy API token. Leave empty only if your Bambuddy instance allows it."));

    grid->Add(new wxStaticText(this, wxID_ANY, _L("Default printer")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 8);
    auto printer_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_printer_choice   = new wxChoice(this, wxID_ANY);
    m_printer_id       = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(80, -1));
    auto refresh_btn   = new wxButton(this, wxID_ANY, _L("Refresh"));
    printer_sizer->Add(m_printer_choice, 1, wxEXPAND | wxRIGHT, 6);
    printer_sizer->Add(new wxStaticText(this, wxID_ANY, _L("ID")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    printer_sizer->Add(m_printer_id, 0, wxRIGHT, 6);
    printer_sizer->Add(refresh_btn, 0);
    grid->Add(printer_sizer, 1, wxEXPAND | wxBOTTOM, 8);

    grid->Add(new wxStaticText(this, wxID_ANY, _L("Proxy auth")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 8);
    m_proxy_mode = new wxChoice(this, wxID_ANY);
    m_proxy_mode->Append(_L("None"));
    m_proxy_mode->Append(_L("Pangolin headers"));
    m_proxy_mode->Append(_L("Pangolin query token"));
    m_proxy_mode->Append(_L("Custom headers"));
    grid->Add(m_proxy_mode, 1, wxEXPAND | wxBOTTOM, 8);

    m_pangolin_token_id     = add_text_row(this, grid, _L("Pangolin token ID"));
    m_pangolin_token_secret = add_text_row(this, grid, _L("Pangolin token"), wxTE_PASSWORD);
    m_pangolin_query_token  = add_text_row(this, grid, _L("Pangolin p_token"), wxTE_PASSWORD);

    grid->Add(new wxStaticText(this, wxID_ANY, _L("Custom headers")), 0, wxALIGN_TOP | wxRIGHT | wxBOTTOM, 8);
    m_custom_headers = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(360, 90), wxTE_MULTILINE);
    m_custom_headers->SetToolTip(_L("One header per line, for example:\nX-Forwarded-User: orca"));
    grid->Add(m_custom_headers, 1, wxEXPAND | wxBOTTOM, 8);

    main_sizer->Add(grid, 1, wxEXPAND | wxALL, 16);

    auto buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto test_btn      = new wxButton(this, wxID_ANY, _L("Test connection"));
    auto ok_btn        = new wxButton(this, wxID_OK, _L("OK"));
    auto cancel_btn    = new wxButton(this, wxID_CANCEL, _L("Cancel"));
    buttons_sizer->Add(test_btn, 0, wxRIGHT, 8);
    buttons_sizer->AddStretchSpacer();
    buttons_sizer->Add(ok_btn, 0, wxRIGHT, 8);
    buttons_sizer->Add(cancel_btn, 0);
    main_sizer->Add(buttons_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    refresh_btn->Bind(wxEVT_BUTTON, &BambuddySettingsDialog::on_refresh_printers, this);
    test_btn->Bind(wxEVT_BUTTON, &BambuddySettingsDialog::on_test_connection, this);
    ok_btn->Bind(wxEVT_BUTTON, &BambuddySettingsDialog::on_ok, this);
    m_proxy_mode->Bind(wxEVT_CHOICE, [this](wxCommandEvent &) { update_proxy_controls(); });
    m_printer_choice->Bind(wxEVT_CHOICE, [this](wxCommandEvent &) {
        const int selection = m_printer_choice->GetSelection();
        if (selection >= 0 && static_cast<size_t>(selection) < m_printers.size())
            m_printer_id->SetValue(wxString::Format("%d", m_printers[selection].id));
    });

    SetSizer(main_sizer);
    main_sizer->SetSizeHints(this);
    CenterOnParent();
}

void BambuddySettingsDialog::transfer_config_to_controls()
{
    m_enabled->SetValue(m_config.enabled);
    m_base_url->SetValue(from_std(m_config.base_url));
    m_api_key->SetValue(from_std(m_config.api_key));
    m_printer_id->SetValue(m_config.default_printer_id > 0 ? wxString::Format("%d", m_config.default_printer_id) : wxString{});

    switch (m_config.proxy_auth.mode) {
    case BambuddyProxyAuthMode::PangolinHeaders:    m_proxy_mode->SetSelection(1); break;
    case BambuddyProxyAuthMode::PangolinQueryToken: m_proxy_mode->SetSelection(2); break;
    case BambuddyProxyAuthMode::CustomHeaders:      m_proxy_mode->SetSelection(3); break;
    case BambuddyProxyAuthMode::None:               m_proxy_mode->SetSelection(0); break;
    }

    m_pangolin_token_id->SetValue(from_std(m_config.proxy_auth.pangolin_token_id));
    m_pangolin_token_secret->SetValue(from_std(m_config.proxy_auth.pangolin_token_secret));
    m_pangolin_query_token->SetValue(from_std(m_config.proxy_auth.pangolin_query_token));
    m_custom_headers->SetValue(format_header_lines(m_config.proxy_auth.custom_headers));

    if (m_config.default_printer_id > 0) {
        BambuddyPrinter printer;
        printer.id   = m_config.default_printer_id;
        printer.name = m_config.default_printer_name.empty() ? "Printer " + std::to_string(m_config.default_printer_id) : m_config.default_printer_name;
        populate_printers({printer});
    }
}

void BambuddySettingsDialog::transfer_controls_to_config()
{
    m_config.enabled            = m_enabled->GetValue();
    m_config.base_url           = trim_copy(to_utf8(m_base_url->GetValue()));
    m_config.api_key            = trim_copy(to_utf8(m_api_key->GetValue()));
    m_config.default_printer_id = 0;

    const std::string printer_id = trim_copy(to_utf8(m_printer_id->GetValue()));
    if (!printer_id.empty()) {
        try {
            m_config.default_printer_id = std::stoi(printer_id);
        } catch (...) {
            m_config.default_printer_id = 0;
        }
    }

    m_config.default_printer_name.clear();
    const int selection = m_printer_choice->GetSelection();
    if (selection >= 0 && static_cast<size_t>(selection) < m_printers.size())
        m_config.default_printer_name = m_printers[selection].name;

    switch (m_proxy_mode->GetSelection()) {
    case 1:  m_config.proxy_auth.mode = BambuddyProxyAuthMode::PangolinHeaders; break;
    case 2:  m_config.proxy_auth.mode = BambuddyProxyAuthMode::PangolinQueryToken; break;
    case 3:  m_config.proxy_auth.mode = BambuddyProxyAuthMode::CustomHeaders; break;
    default: m_config.proxy_auth.mode = BambuddyProxyAuthMode::None; break;
    }

    m_config.proxy_auth.pangolin_token_id     = trim_copy(to_utf8(m_pangolin_token_id->GetValue()));
    m_config.proxy_auth.pangolin_token_secret = trim_copy(to_utf8(m_pangolin_token_secret->GetValue()));
    m_config.proxy_auth.pangolin_query_token  = trim_copy(to_utf8(m_pangolin_query_token->GetValue()));
    m_config.proxy_auth.custom_headers        = parse_header_lines(m_custom_headers->GetValue());
}

void BambuddySettingsDialog::update_proxy_controls()
{
    const int selection = m_proxy_mode->GetSelection();
    m_pangolin_token_id->Enable(selection == 1);
    m_pangolin_token_secret->Enable(selection == 1);
    m_pangolin_query_token->Enable(selection == 2);
    m_custom_headers->Enable(selection == 3);
}

void BambuddySettingsDialog::populate_printers(const std::vector<BambuddyPrinter> &printers)
{
    m_printers = printers;
    m_printer_choice->Clear();
    int selection = wxNOT_FOUND;
    for (size_t i = 0; i < m_printers.size(); ++i) {
        const auto &printer = m_printers[i];
        wxString    label   = from_std(printer.name);
        if (!printer.model.empty())
            label += " (" + from_std(printer.model) + ")";
        label += wxString::Format(" [%d]", printer.id);
        m_printer_choice->Append(label);
        if (printer.id == m_config.default_printer_id)
            selection = static_cast<int>(i);
    }
    if (selection == wxNOT_FOUND && !m_printers.empty())
        selection = 0;
    if (selection != wxNOT_FOUND) {
        m_printer_choice->SetSelection(selection);
        m_printer_id->SetValue(wxString::Format("%d", m_printers[selection].id));
    }
}

void BambuddySettingsDialog::on_refresh_printers(wxCommandEvent &)
{
    transfer_controls_to_config();
    BambuddyClient              client(m_config);
    std::vector<BambuddyPrinter> printers;
    std::string                 error;
    wxBusyCursor                wait;
    if (!client.list_printers(printers, error)) {
        show_error(this, from_std(error), false);
        return;
    }
    populate_printers(printers);
    show_info(this, wxString::Format(_L("Loaded %zu Bambuddy printers."), printers.size()), _L("Bambuddy"));
}

void BambuddySettingsDialog::on_test_connection(wxCommandEvent &)
{
    transfer_controls_to_config();
    BambuddyClient client(m_config);
    std::string    error;
    wxBusyCursor   wait;
    if (!client.test_connection(error)) {
        show_error(this, from_std(error), false);
        return;
    }
    show_info(this, _L("Bambuddy connection succeeded."), _L("Bambuddy"));
}

void BambuddySettingsDialog::on_ok(wxCommandEvent &)
{
    transfer_controls_to_config();
    if (m_config.enabled && m_config.base_url.empty()) {
        show_error(this, _L("Please enter a Bambuddy URL."), false);
        return;
    }
    if (m_config.enabled && m_config.default_printer_id <= 0) {
        show_error(this, _L("Please choose a Bambuddy printer or enter its ID."), false);
        return;
    }

    if (wxGetApp().app_config) {
        BambuddyClient::save_to_app_config(*wxGetApp().app_config, m_config);
        wxGetApp().app_config->save();
    }
    EndModal(wxID_OK);
}

} // namespace GUI
} // namespace Slic3r
