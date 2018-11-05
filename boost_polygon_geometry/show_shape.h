#include <wx/wx.h>



#ifdef SHOW_GEOM
#include <geom_poly_types.h>
typedef multi_polygon DisplayedType;
#else
#include <gtl_poly_types.h>
#define POLY_NO_HOLES 1
typedef PolygonSet_NoHoles DisplayedType;
#endif

class MyCanvas: public wxScrolled<wxWindow>
{
  public:
    MyCanvas(wxWindow *parent, std::vector<DisplayedType> &);

    static const int WIDTH = 292;
    static const int HEIGHT = 297;

    void OnPaint(wxPaintEvent& event);
    void OnScaleUp(wxCommandEvent& event) { scale *= 1.25; Refresh(); };
    void OnScaleDown(wxCommandEvent& event) { scale /= 1.25; Refresh(); };

    void OnFit(wxCommandEvent& event);
  private:
    std::vector<DisplayedType> &polys;
#ifdef SHOW_GEOM
    box  bbox;
#else
    Rect bbox;
#endif
    double scale = .01;

    // depending on the range of values of the polys, I need to upscale the numbers.
    // this is because wx uses integers. The integers need to be large enough to not
    // just round to one or two values.
    double upscale = 1000;
    wxCoord xoffset = 0;
    wxCoord yoffset = 0;

    wxBrush background_brush;
    wxBrush foreground_brush;
    wxPen   foreground_pen;

    const wxPen*   pens[4]    = {wxRED_PEN,   wxCYAN_PEN,   wxYELLOW_PEN,   wxGREEN_PEN};
    const wxBrush* brushes[4] = {wxRED_BRUSH, wxCYAN_BRUSH, wxYELLOW_BRUSH, wxGREEN_BRUSH};
};



class MyFrame: public wxFrame
{
public:
    MyFrame(std::vector<DisplayedType> &polys,
            const wxString& title,
            const wxPoint& pos,
            const wxSize& size);
private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();

private:
    MyCanvas* canvas;
};


class MyApp: public wxApp
{
  public:
    MyApp(std::vector<DisplayedType> &polys) : polys(polys) { /* empty */ }
    virtual bool OnInit();

  private:
    std::vector<DisplayedType> &polys;
};
