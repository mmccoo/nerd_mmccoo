
#include <show_shape.h>

#include <wx/string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <limits>

#ifdef SHOW_GEOM
#include <geom_poly_serialize.h>
#else
#include <gtl_poly_serialize.h>
#endif

#include <poly_utils.h>

MyCanvas::MyCanvas(wxWindow *parent, std::vector<DisplayedType> &polys)
    : wxScrolled<wxWindow>(parent, wxID_ANY, wxDefaultPosition, wxSize(1450, 1340)),
      polys(polys) {

    DisplayedType allpolys;
    for (DisplayedType ps : polys) {
        allpolys.insert(allpolys.end(), ps.begin(), ps.end());
    }

#ifdef SHOW_GEOM
    boost::geometry::envelope(allpolys, bbox);
#else
    gtl::extents(bbox, allpolys);
#endif

    SetScrollRate( 10, 10 );

    background_brush = *wxTRANSPARENT_BRUSH;
    foreground_brush = *wxMEDIUM_GREY_BRUSH;
    foreground_pen   = *wxRED_PEN;

    //SetVirtualSize( WIDTH, HEIGHT );

    // when using object methods, the instance of the handler has to match the
    // instance of the wx object.
    Bind(wxEVT_PAINT, &MyCanvas::OnPaint, this);
}

void MyCanvas::OnFit(wxCommandEvent& event)
{
    //boost::geometry::model::box<Point> box;
    //boost::geometry::envelope(polys, box);
    //double width = box.max_corner().get<0>() - box.min_corner().get<0>();
    //double height = box.max_corner().get<1>() - box.min_corner().get<1>();

#ifdef SHOW_GEOM
    double width  = bbox.max_corner().get<0>() - bbox.min_corner().get<0>();
    double height = bbox.max_corner().get<1>() - bbox.min_corner().get<1>();
    double xl = bbox.min_corner().get<0>();
    double yh = bbox.max_corner().get<1>();
#else
    double width  = gtl::delta(bbox, gtl::HORIZONTAL);
    double height = gtl::delta(bbox, gtl::VERTICAL);
    double xl = gtl::xl(bbox);
    double yh = gtl::yh(bbox);
#endif

    wxSize size = GetSize();

    double scale_width  = size.GetWidth()/width;
    double scale_height = size.GetHeight()/height;

    scale = std::min(scale_width, scale_height) * .9;

    // don't need to scale this number. DC will do it.
    //xoffset = (box.min_corner().get<0>()-width*0.05  );



    xoffset = (xl-width*0.05  );

    // want max corner because we're gonna flip the ys.
    //yoffset = (box.max_corner().get<1>()+height*0.05 );
    yoffset = (yh+height*0.05 );

    Refresh();
}

void MyCanvas::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);

  // https://github.com/wxWidgets/wxWidgets/blob/master/samples/scroll/scroll.cpp
  // this call is vital: it adjusts the dc to account for the current
  // scroll offset
  PrepareDC(dc);

  dc.SetBackground(background_brush);
  dc.Clear();

  //dc.SetBrush( *wxTRANSPARENT_BRUSH );
  //wxBrush brushHatch(*wxRED, wxBRUSHSTYLE_FDIAGONAL_HATCH);
  //dc.SetBrush(brushHatch);
  //dc.SetBrush(*wxMEDIUM_GREY_BRUSH);

  dc.SetUserScale(scale, scale);

  wxSize size = GetSize();

  // x/yoffset are wxcoords, ie int. these are already scaled.
  dc.SetLogicalOrigin(xoffset, -yoffset);

  int maxx= -1 * std::numeric_limits<int>::max();
  int maxy= -1 * std::numeric_limits<int>::max();
  int minx=      std::numeric_limits<int>::max();
  int miny=      std::numeric_limits<int>::max();

  int layernum = -1;
  for(DisplayedType ps : polys) {
      layernum++;
      assert(layernum < sizeof(pens)/sizeof(wxPen*));
      dc.SetPen(*pens[layernum]);
      //dc.SetBrush(*brushes[layernum]);
      dc.SetBrush(*wxTRANSPARENT_BRUSH);

      for(/*Polygon_Holes*/ auto poly : ps) {
          {
              wxPointList pts;
              std::vector<wxPoint> wpts;
#ifdef SHOW_GEOM
              for(point pt : poly.outer()) {
                  int x = pt.get<0>();
                  int y = pt.get<1>();
#else
              for(Point pt : poly) {
                  int x = pt.x();
                  int y = pt.y();
#endif
                  wpts.push_back(wxPoint(x,-y));
                  minx = std::min(minx,  x);
                  miny = std::min(miny, -y);
                  maxx = std::max(maxx,  x);
                  maxy = std::max(maxy, -y);
              }
              //dc.DrawLines(wpts.size(), wpts.data());
              dc.DrawPolygon(wpts.size(), wpts.data());
          }

#ifndef POLY_NO_HOLES

#ifdef SHOW_GEOM
          for(auto inner : poly.inners()) {
              const ring &hole = inner;
#else
          for(auto iter=poly.begin_holes(); iter!=poly.end_holes(); ++iter) {
              const Polygon_NoHoles &hole = *iter;
#endif

              dc.SetBrush(background_brush);
              wxPointList pts;

              std::vector<wxPoint> wpts;
              // pt can be either point (ie geom) or Point (ie gtl)
              for(auto pt : hole) {
#ifdef SHOW_GEOM
                  int x = pt.get<0>();
                  int y = pt.get<1>();
#else
                  int x = pt.x();
                  int y = pt.y();
#endif
                  wpts.push_back(wxPoint(x,-y));
              }
              dc.DrawPolygon(wpts.size(), wpts.data());

              dc.SetBrush(foreground_brush);
          }

#endif // POLY_NO_HOLES
      }


  }

  int vsize = std::max((maxx-minx)*scale, (maxy-miny)*scale);
  SetVirtualSize(vsize, vsize);
}


enum
{
    ID_Hello = 1
};


wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Hello,   MyFrame::OnHello)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()


MyFrame::MyFrame(std::vector<DisplayedType> &polys,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size)
: wxFrame(NULL, wxID_ANY, title, pos, size)
{

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    wxBoxSizer* buttonssizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonssizer);

    wxButton* scaleUp = new wxButton(this, -1, "+");
    buttonssizer->Add(scaleUp);

    wxButton* scaleDown = new wxButton(this, -1, "-");
    buttonssizer->Add(scaleDown);

    wxButton* fit = new wxButton(this, -1, "Fit");
    buttonssizer->Add(fit);

    canvas = new MyCanvas(this, polys);
    sizer->Add(canvas, 1, wxEXPAND|wxFIXED_MINSIZE);

    scaleUp->Bind( wxEVT_BUTTON, &MyCanvas::OnScaleUp, canvas);
    scaleDown->Bind( wxEVT_BUTTON, &MyCanvas::OnScaleDown, canvas);
    fit->Bind( wxEVT_BUTTON, &MyCanvas::OnFit, canvas);

    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );

    SetMenuBar( menuBar );
    CreateStatusBar();
    SetStatusText( "Welcome to wxWidgets!" );

    Layout();
    wxCommandEvent dummyevt(0, wxEVT_BUTTON);
    canvas->OnFit(dummyevt);

}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}


void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "This is a wxWidgets' Hello world sample",
                  "About Hello World", wxOK | wxICON_INFORMATION );
}


void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}





bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( polys,
                                  "Hello World",
                                  wxPoint(50, 50),
                                  wxSize(900, 900) );
    frame->Show( true );

    return true;
}


int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf("please give some paths\n");
        exit(-1);
    }


    std::vector<DisplayedType> polys;

    for(int i=1; i<argc; i++) {
#if SHOW_GEOM
        DisplayedType ps;
        read_polys(argv[i], ps);
#else
        PolygonSet polysholes;
        read_polys(argv[i], polysholes);

        // I always read with holes but displayed may be without.
        DisplayedType ps;
        for (auto poly : polysholes) {
            DisplayedType tmp;
            gtl::assign(tmp, poly);
            ps.insert(ps.end(), tmp.begin(), tmp.end());
        }
#endif
        //std::cout << ps << "\n";
        #if 0
            PolygonSet_NoHoles polys_noholes;
            gtl::assign(polys_noholes, ps);
            polys.push_back(polys_noholes);
        #else
            polys.push_back(ps);
            // PolygonSet simplified;
            // for (auto poly : ps) {
            //     simplified.push_back(simplify_polys(poly, 1000));
            // }
            //polys.push_back(simplified);
        #endif
    }



    MyApp* app = new MyApp(polys);
    wxApp::SetInstance(app);
    wxEntry(argc, argv);

    return 0;

}
