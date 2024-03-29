/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 * $Revision: 6781 $
 * $Id: app.cpp 6781 2010-10-28 11:55:52Z loaden $
 * $HeadURL: svn://svn.berlios.de/codeblocks/trunk/src/plugins/codecompletion/parser/app.cpp $
 */
#include "wx_pch.h"


//(*AppHeaders
#include <wx/xrc/xmlres.h>
#include <wx/image.h>
//*)

#include "frame.h"

class TestApp : public wxApp
{
public:
    virtual bool OnInit();
    Frame* GetFrame() const { return m_frame; }

private:
    Frame* m_frame;
};

IMPLEMENT_APP(TestApp)
DECLARE_APP(TestApp)

bool TestApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    wxsOK = wxsOK && wxXmlResource::Get()->Load(_T("resource.rc"));
    //*)

    m_frame = new Frame;
    m_frame->Center();
    m_frame->Show();
    m_frame->Start(_T("test.h"));

    return wxsOK;
}

void ParserTrace(const wxChar* format, ...)
{
    va_list ap;
    va_start(ap, format);
    wxString log = wxString::FormatV(format, ap);
    va_end(ap);

    // Convert '\r' to "\r", '\n' to "\n"
    for (size_t i = 0; i < log.Len(); ++i)
    {
        if (log.GetChar(i) == _T('\r'))
        {
            log.SetChar(i, _T('\\'));
            log.insert(++i, 1, _T('r'));
        }
        else if (log.GetChar(i) == _T('\n'))
        {
            log.SetChar(i, _T('\\'));
            log.insert(++i, 1, _T('n'));
        }
    }

    wxGetApp().GetFrame()->Log(log);
}

