/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <map>
#include <sstream>
#include <string>

#include <wx/button.h>
#include <wx/colour.h>
#include <wx/choice.h>
#include <wx/log.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "cat_settings.h"
#include "ocpn_utils.h"
#include "PluginHandler.h"

wxDEFINE_EVENT(EVT_CATALOG_CHANGE, wxCommandEvent);
wxDEFINE_EVENT(EVT_COMPAT_OS_CHANGE, wxCommandEvent);

wxDEFINE_EVENT(EVT_CUSTOM_INIT, wxCommandEvent);
wxDEFINE_EVENT(EVT_CUSTOM_CLEAR, wxCommandEvent);

extern wxString       g_catalog_channel;
extern wxString       g_catalog_custom_url;
extern wxString       g_compatOS;
extern wxString       g_compatOsVersion;


/** The custom URL text entry. */
class CustomCatalogCtrl: public wxTextCtrl
{
    public:
        CustomCatalogCtrl(wxWindow* parent): wxTextCtrl(parent, wxID_ANY, "")
        {
            Bind(wxEVT_TEXT,
                [&](wxCommandEvent& e){ g_catalog_custom_url = GetValue(); });
            Bind(EVT_CUSTOM_INIT,
                [&](wxCommandEvent& e){ ChangeValue(g_catalog_custom_url); });
            Bind(EVT_CUSTOM_CLEAR,
                [&](wxCommandEvent& e){ ChangeValue(""); });
        }
};


/** Select compatibility drop-down. */
class PlatformChoice: public wxChoice
{
    public:
        void  OnChoice(wxCommandEvent&)
        {
            if (GetSelection() == 0) {
                return;
            }
            if (GetSelection() == 1) {
                g_compatOS = "";
                g_compatOsVersion = "";
                auto compOS = CompatOs::getInstance();
                std::stringstream ss;
                ss << compOS->name() << ":" << compOS->version();
                m_selected->SetLabel(ss.str().c_str());
            }
            else {
                auto current = GetString(GetSelection());
                auto os = ocpn::split(current, " ")[0];
                m_selected->SetLabel(os);
                g_compatOS = ocpn::split(os.c_str(), ":")[0];
                g_compatOsVersion = ocpn::split(os.c_str(), ":")[1];
            }
            wxCommandEvent event(EVT_COMPAT_OS_CHANGE, GetId());
            ::wxPostEvent(this, event);
        }

        PlatformChoice(wxWindow* parent, wxStaticText* selected)
            :wxChoice(), m_selected(selected)
        {
            Bind(wxEVT_CHOICE, &PlatformChoice::OnChoice, this);
            Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, getLabels());
            SetSelection(0);
            Layout();
        }

    private:
        wxStaticText* m_selected;

        wxArrayString getLabels()
        {
            auto plug_handler = PluginHandler::getInstance();
            wxArrayString labels;
            labels.Add(_("Select new flavour"));
            labels.Add(_("Default setting"));
            for (const auto& c: plug_handler->getCountByTarget()) {
                std::stringstream ss;
                ss << c.first << "   ("  << c.second << ")";
                labels.Add(ss.str());
            }
            return labels;
         }
};


/** Select master, beta, alpha, custom drop-down. */
class CatalogChoice: public wxChoice
{
    public:
        CatalogChoice(wxWindow* parent, wxWindow* custom_ctrl)
            : wxChoice(), m_custom_ctrl(custom_ctrl)
        {
            std::vector<std::string>
                labels({"master", "beta", "alpha", "custom"});
            wxArrayString wxLabels;
            for (const auto& l: labels) wxLabels.Add(l);
            Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLabels);

            m_custom_ctrl->Enable(false);
            for (auto l = labels.begin(); l != labels.end(); l++) {
                if (g_catalog_channel == *l) {
                    SetSelection(l - labels.begin());
                }
            }
            wxCommandEvent ev; 
            OnChoice(ev);
            Layout();
            Bind(wxEVT_CHOICE, &CatalogChoice::OnChoice, this);
        }

     private:
        wxWindow* m_custom_ctrl;

        void  OnChoice(wxCommandEvent &ev)
        {
            auto selected = GetString(GetSelection());
            m_custom_ctrl->Enable(selected == "custom");
            if (selected == "custom") {
                m_custom_ctrl->Show();
                GetParent()->Layout();
                m_custom_ctrl->SetFocus();
                wxCommandEvent event(EVT_CUSTOM_INIT, GetId());
                ::wxPostEvent(m_custom_ctrl, event);
            }
            else {
                wxCommandEvent event(EVT_CUSTOM_CLEAR, GetId());
                ::wxPostEvent(m_custom_ctrl, event);
                m_custom_ctrl->Hide();
            }
            wxCommandEvent event(EVT_CATALOG_CHANGE, GetId());
            ::wxPostEvent(GetGrandParent(), event);

            g_catalog_channel = selected;
            Layout();
        }
};


/** Current selected compatibility. */
class StatusText: public wxStaticText
{
    public:
        StatusText(wxWindow* parent): wxStaticText(parent, wxID_ANY, "")
        {
            auto compatOs = CompatOs::getInstance();
            auto os = compatOs->name() + ":" + compatOs->version();
            SetLabel(os);
        }
};


/** Catalog channel selection panel. */
class CatalogSizer: public wxStaticBoxSizer
{
    public:
        CatalogSizer(wxWindow* parent)
           : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Active catalog"))
        {
            auto flags = wxSizerFlags().Border();
            Add(new wxStaticText(parent, wxID_ANY, _("Select plugin catalog")),
                flags.Center());
            auto custom_ctrl = new CustomCatalogCtrl(parent);
            Add(new CatalogChoice(parent, custom_ctrl), flags.Expand());
            Add(custom_ctrl, flags.Expand().Proportion(1));
            Layout();
        }
};


/** Plugin compatibility panel. */
class CompatSizer: public wxStaticBoxSizer
{
    public:
        CompatSizer(wxWindow* parent)
           : wxStaticBoxSizer(wxHORIZONTAL, parent, _("Compatibility"))
        {
            auto flags = wxSizerFlags().Border();
            Add(new wxStaticText(parent, wxID_ANY, _("Active setting:")),
                flags.Center());
            auto status_text = new StatusText(parent);
            Add(status_text, flags.Center().Proportion(1));
            Add(new PlatformChoice(parent, status_text), flags);
        }
};


/** The Dismiss button. */
class ButtonsSizer: public wxStdDialogButtonSizer
{
    public:
        ButtonsSizer(wxWindow* parent): wxStdDialogButtonSizer()
        {
            auto button = new wxButton(parent, wxID_OK);
            button->SetLabel(_("Dismiss"));
            SetAffirmativeButton(button);
            Realize();
        }
};


/** Top-level plugin settings dialog.  */
CatalogSettingsDialog::CatalogSettingsDialog(wxWindow* parent)
    :wxDialog(parent, wxID_ANY, _("Plugin Catalog Settings"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(new CatalogSizer(this), wxSizerFlags().Expand().Border());
    vbox->Add(new CompatSizer(this), wxSizerFlags().Expand().DoubleBorder());
    vbox->Add(new ButtonsSizer(this), wxSizerFlags().Expand().DoubleBorder());
 
    SetSizer(vbox);
    Fit();
    Layout();
    SetMinSize(GetSize());
    /** Forward otherwise dropped event (this is a Dialog). */
    Bind(EVT_COMPAT_OS_CHANGE,
         [&](wxCommandEvent& e) { ::wxPostEvent(GetParent(), e); });
}
