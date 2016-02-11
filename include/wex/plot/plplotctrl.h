#ifndef __pl_plotctrl_h
#define __pl_plotctrl_h

#include <vector>
#include <wx/window.h>
#include <wx/menu.h>
#include <wx/stream.h>

#ifdef __WXOSX__
#define PL_USE_OVERLAY 1
#include <wx/overlay.h>
#endif

#include "wex/plot/plaxis.h"

class wxPLPlotCtrl;

class wxPLDeviceMapping
{
public:
	wxPLDeviceMapping() { }
	virtual ~wxPLDeviceMapping() { }
	virtual wxPoint ToDevice( double x, double y ) const = 0;
	virtual wxRect GetDeviceExtents( ) const = 0;
	virtual wxRealPoint GetWorldMinimum() const = 0;
	virtual wxRealPoint GetWorldMaximum() const = 0;
	virtual wxPLAxis* GetXAxis() const = 0;
	virtual wxPLAxis* GetYAxis() const = 0;
	virtual wxPLPlotCtrl *GetPlotCtrl() const = 0;
	
	inline wxPoint ToDevice( const wxRealPoint &p ) const { return ToDevice(p.x, p.y); }
};

class wxPLOutputDevice
{
public:
	virtual ~wxPLOutputDevice() { }

	enum Style { NONE, SOLID, DOT, DASH, DOTDASH, MITER, BEVEL, ROUND, BUTT, HATCH, ODDEVEN, WIND };

	// Pure virtuals, to be implemented
	virtual void Clip( int x, int y, int width, int height ) = 0;
	virtual void Unclip() = 0;
	virtual void Pen( const wxColour &c = *wxBLACK, int size=1, Style line = SOLID, Style join = MITER, Style cap = BUTT ) = 0;
	virtual void Brush( const wxColour &c = *wxBLACK, Style sty = SOLID ) = 0;
	
	virtual void Point( int x, int y ) = 0;
	virtual void Line( int x1, int y1, int x2, int y2 ) = 0;
	virtual void Lines( size_t n, const wxPoint *pts ) = 0;
	virtual void Polygon( size_t n, const wxPoint *pts, Style winding = ODDEVEN ) = 0;
	virtual void Rect( int x, int y, int width, int height )  = 0;	
	virtual void Circle( int x, int y, int radius ) = 0;
	
	virtual void Font( int relpt = 0, bool bold = false ) = 0;
	virtual void Text( const wxString &text, int x, int y,  int angle=0 ) = 0;
	virtual void Measure( const wxString &text, int *width, int *height ) = 0;
	virtual int CharHeight() = 0;


	// API variants
	virtual void Point( const wxPoint &p ) { Point(p.x,p.y); }
	virtual void Line( const wxPoint &p1, const wxPoint &p2 ) { Line( p1.x, p1.y, p2.x, p2.y ); }
	virtual void Rect( const wxRect &r ) { Rect( r.x, r.y, r.width, r.height ); }
	virtual void Circle( const wxPoint &p, int radius ) { Circle(p.x, p.y, radius); }
	virtual void Text( const wxString &text, const wxPoint &p, int angle=0 ) { Text( text, p.x, p.y, angle ); }

};

class wxPLPlottable
{
protected:
	wxString m_label;
	wxString m_xLabel;
	wxString m_yLabel;
	bool m_showInLegend;
	bool m_antiAliasing;

public:
	wxPLPlottable() : m_label(wxEmptyString), m_showInLegend(true), m_antiAliasing(true) {  }
	wxPLPlottable( const wxString &label ) : m_label(label), m_showInLegend(true), m_antiAliasing(true) {  }
	virtual ~wxPLPlottable() {  }

	// pure virtuals

	virtual wxRealPoint At( size_t i ) const = 0;
	virtual size_t Len() const = 0;
	virtual void Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map ) = 0;
	virtual void DrawInLegend( wxPLOutputDevice &dc, const wxRect &rct ) = 0;

	// properties

	virtual wxString GetLabel() const { return m_label; }
	virtual wxString GetXDataLabel() const { return m_xLabel; }
	virtual wxString GetYDataLabel() const { return m_yLabel; }
	virtual void SetLabel( const wxString &label ) { m_label = label; }
	virtual void SetXDataLabel( const wxString &label ) { m_xLabel = label; }
	virtual void SetYDataLabel( const wxString &label ) { m_yLabel = label; }
	virtual void ShowInLegend( bool b ) { m_showInLegend = b; }
	virtual bool IsShownInLegend() { return m_showInLegend; }

	// display

	void SetAntiAliasing( bool aa ) { m_antiAliasing = aa; }
	bool GetAntiAliasing() { return m_antiAliasing; }


	// helpers

	virtual wxPLAxis *SuggestXAxis() const;
	virtual wxPLAxis *SuggestYAxis() const;

	virtual bool GetMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax) const;
	virtual bool ExtendMinMax(double *pxmin, double *pxmax, double *pymin, double *pymax, bool extendToNice = false) const;
	virtual std::vector<wxString> GetExportableDatasetHeaders( wxUniChar sep ) const;
	virtual std::vector<wxRealPoint> GetExportableDataset(double Xmin, double Xmax, bool visible_only) const;
};

class wxPLSideWidgetBase
{
public:
	wxPLSideWidgetBase();
	virtual ~wxPLSideWidgetBase();

	virtual void Render( wxPLOutputDevice &, const wxRect & ) = 0;
	virtual wxSize CalculateBestSize() = 0;

	void InvalidateBestSize();
	wxSize GetBestSize();

private:
	wxSize m_bestSize;
};

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE( wxEVT_PLOT_LEGEND, 0)
	DECLARE_EVENT_TYPE( wxEVT_PLOT_ZOOM, 0)
	DECLARE_EVENT_TYPE( wxEVT_PLOT_HIGHLIGHT, 0)
	DECLARE_EVENT_TYPE( wxEVT_PLOT_DRAGGING, 0)
	DECLARE_EVENT_TYPE( wxEVT_PLOT_DRAG_START, 0)
	DECLARE_EVENT_TYPE( wxEVT_PLOT_DRAG_END, 0)
END_DECLARE_EVENT_TYPES()

#define EVT_PLOT_LEGEND(id, func) EVT_COMMAND(id, wxEVT_PLOT_LEGEND, func)
#define EVT_PLOT_ZOOM(id, func) EVT_COMMAND(id, wxEVT_PLOT_ZOOM, func)
#define EVT_PLOT_HIGHLIGHT(id, func) EVT_COMMAND(id, wxEVT_PLOT_HIGHLIGHT, func)
#define EVT_PLOT_DRAGGING(id, func) EVT_COMMAND(id, wxEVT_PLOT_DRAGGING, func)
#define EVT_PLOT_DRAG_START(id, func) EVT_COMMAND(id, wxEVT_PLOT_DRAG_START, func)
#define EVT_PLOT_DRAG_END(id, func) EVT_COMMAND(id, wxEVT_PLOT_DRAG_END, func)

class wxPLPlotCtrl : public wxWindow
{
public:
	wxPLPlotCtrl( wxWindow *parent, int id, 
		const wxPoint &pos = wxDefaultPosition, 
		const wxSize &size = wxDefaultSize );
	virtual ~wxPLPlotCtrl();

	enum AxisPos { X_BOTTOM, X_TOP, Y_LEFT, Y_RIGHT };
	enum PlotPos { PLOT_TOP, PLOT_BOTTOM, NPLOTPOS };
	enum LegendPos { FLOATING, NORTHWEST, SOUTHWEST, NORTHEAST, SOUTHEAST, NORTH, SOUTH, EAST, WEST, BOTTOM, RIGHT  };

	void AddPlot( wxPLPlottable *p, AxisPos xap = X_BOTTOM, AxisPos yap = Y_LEFT, PlotPos ppos = PLOT_TOP, bool update_axes = true );
	wxPLPlottable *RemovePlot(wxPLPlottable *p, PlotPos plotPosition = NPLOTPOS);
	bool ContainsPlot(wxPLPlottable *p, PlotPos plotPosition = NPLOTPOS);
	void DeleteAllPlots();
	size_t GetPlotCount();
	wxPLPlottable *GetPlot( size_t i );
	wxPLPlottable *GetPlotByLabel( const wxString &series );
	bool GetPlotPosition( wxPLPlottable *p, 
		AxisPos *xap, AxisPos *yap, PlotPos *ppos );

	wxPLAxis *GetXAxis1() { return m_x1.axis; }
	wxPLAxis &X1() { return Axis(X_BOTTOM); }
	void SetXAxis1( wxPLAxis *a ) { m_x1.set( a ); }
	
	wxPLAxis *GetXAxis2() { return m_x2.axis; }
	wxPLAxis &X2() { return Axis(X_TOP); }
	void SetXAxis2( wxPLAxis *a ) { return m_x2.set( a ); }
	
	wxPLAxis *GetYAxis1( PlotPos ppos = PLOT_TOP ) { return m_y1[ppos].axis; }
	wxPLAxis &Y1( PlotPos ppos = PLOT_TOP ) { return Axis(Y_LEFT,ppos); }
	void SetYAxis1( wxPLAxis *a, PlotPos ppos = PLOT_TOP ) { m_y1[ppos].set( a ); }
	
	wxPLAxis *GetYAxis2( PlotPos ppos = PLOT_TOP ) { return m_y2[ppos].axis; }
	wxPLAxis &Y2( PlotPos ppos = PLOT_TOP ) { return Axis(Y_RIGHT,ppos); }
	void SetYAxis2( wxPLAxis *a, PlotPos ppos = PLOT_TOP ) { m_y2[ppos].set( a ); }

	wxPLAxis *GetAxis( AxisPos axispos, PlotPos ppos = PLOT_TOP );
	wxPLAxis &Axis( AxisPos axispos, PlotPos ppos = PLOT_TOP );
	void SetAxis( wxPLAxis *a, AxisPos axispos, PlotPos ppos = PLOT_TOP );
	
	void SetScaleTextSize( bool scale ) { m_scaleTextSize = scale; }
	
	void ShowGrid( bool coarse, bool fine ) { m_showCoarseGrid = coarse; m_showFineGrid = fine; }
	void ShowCoarseGrid( bool coarse ) { m_showCoarseGrid = coarse; }
	void ShowFineGrid( bool fine ) { m_showFineGrid = fine; }
	void ShowTitle( bool show ) { m_showTitle = show; }
	void SetTitle( const wxString &title );
	wxString GetTitle() { return m_title; }

	void SetGridColour( const wxColour &col ) { m_gridColour = col; }
	void SetPlotAreaColour( const wxColour &col ) { m_plotAreaColour = col; }
	void SetAxisColour( const wxColour &col ) { m_axisColour = col; }
	void SetTickTextColour( const wxColour &col ) { m_tickTextColour = col; }
	
	void ShowLegend( bool show ) { m_showLegend = show; }
	void SetLegendReversed( bool reverse ) { m_reverseLegend = reverse; }
	void SetLegendLocation( LegendPos pos, double xpercent = -1.0, double ypercent = -1.0 );
	bool SetLegendLocation( const wxString &spos );
	void SetIncludeLegendOnExport( bool b ) { m_includeLegendOnExport = b; }
	wxRealPoint GetLegendLocation() { return m_legendPosPercent; }

	enum HighlightMode { HIGHLIGHT_DISABLE, 
		HIGHLIGHT_RECT, 
		HIGHLIGHT_ZOOM,
		HIGHLIGHT_SPAN };

	void SetHighlightMode( HighlightMode highlighting ) { m_highlighting = highlighting; }
	void GetHighlightBounds( double *left, double *right, double *top=0, double *bottom=0 );

	wxMenu &GetContextMenu() { return m_contextMenu; }

	void SetSideWidget( wxPLSideWidgetBase *sw, AxisPos pos = Y_RIGHT );
	wxPLSideWidgetBase *GetSideWidget( AxisPos pos );
	wxPLSideWidgetBase *ReleaseSideWidget( AxisPos pos );


	void WriteDataAsText( wxUniChar sep, wxOutputStream &os, 
		bool visible_only = true, bool include_x = true );

	wxString ShowExportDialog();

	// handle pdf,svg,bmp,jpg,png,xpm,tiff
	bool Export( const wxString &file, int width=-1, int height=-1 );
	bool ExportPdf( const wxString &file );
	bool ExportSvg( const wxString &file );
	wxBitmap GetBitmap( int width=-1, int height=-1 );
	
	void UpdateAxes( bool recalculate_all = false );
	void RescaleAxes();
	void DeleteAxes();

	void Invalidate(); // erases all cached positions and layouts, but does not issue refresh
	void Render( wxDC &dc, wxRect geom ); // note: does not draw the background.  DC should be cleared with desired bg color already

	class text_layout;
	class axis_layout;

protected:
	void OnPopupMenu( wxCommandEvent & );
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );
	void OnLeftDown( wxMouseEvent & );
	void OnLeftUp( wxMouseEvent & );
	void OnLeftDClick( wxMouseEvent & );
	void OnRightDown( wxMouseEvent & );
	void OnMotion( wxMouseEvent & );
	void OnMouseCaptureLost( wxMouseCaptureLostEvent & );

	void DrawGrid( wxDC &dc, wxPLAxis::TickData::TickSize size );
	void DrawPolarGrid(wxDC &dc, wxPLAxis::TickData::TickSize size);
	void DrawLegend(wxDC &dc, wxPLOutputDevice &odev, const wxRect &geom);

	void UpdateHighlightRegion();
	void DrawLegendOutline();

private:
	bool m_showLegend;
	bool m_showCoarseGrid;
	bool m_showFineGrid;
	bool m_showTitle;
	bool m_scaleTextSize;
	wxString m_title;
	wxColour m_gridColour;
	wxColour m_axisColour;
	wxColour m_tickTextColour;
	wxColour m_plotAreaColour;
	wxRect m_legendRect;
	LegendPos m_legendPos;
	wxRealPoint m_legendPosPercent;
	bool m_reverseLegend;
	bool m_includeLegendOnExport;
	bool m_moveLegendMode;
	bool m_moveLegendErase;
	wxPoint m_anchorPoint;
	wxPoint m_currentPoint;
	wxMenu m_contextMenu;
	bool m_highlightMode;

#ifdef PL_USE_OVERLAY
	wxOverlay m_overlay;
#endif
	bool m_highlightErase;
	double m_highlightLeftPercent;
	double m_highlightRightPercent;
	double m_highlightTopPercent;
	double m_highlightBottomPercent;
	HighlightMode m_highlighting;
	std::vector< wxRect > m_plotRects;

	wxPLSideWidgetBase *m_sideWidgets[4];

	struct plot_data
	{
		plot_data() : plot(0), ppos(PLOT_TOP), xap(X_BOTTOM), yap(Y_LEFT) {  }
		wxPLPlottable *plot;
		PlotPos ppos;
		AxisPos xap;
		AxisPos yap;
	};

	std::vector<plot_data> m_plots;


	struct legend_item
	{
		legend_item( wxDC &dc, wxPLPlottable *p );
		~legend_item();
		wxPLPlottable *plot;
		wxString text;
		text_layout *layout;
	};

	bool m_legendInvalidated;
	std::vector< legend_item* > m_legendItems;
	void CalcLegendTextLayout( wxDC &dc );

	struct axis_data
	{
		axis_data();
		~axis_data();
		void set( wxPLAxis *a );
		void invalidate();

		wxPLAxis *axis;
		axis_layout *layout;
		text_layout *label;
	};
	
	text_layout *m_titleLayout;
	axis_data m_x1, m_x2, m_y1[NPLOTPOS], m_y2[NPLOTPOS]; // m_x1 used for angular axis on polar plots, m_y1[0] used for radial axis

	DECLARE_EVENT_TABLE();

};


class TextLayoutDemo : public wxWindow
{
public:
	TextLayoutDemo( wxWindow *parent );

	std::vector<wxCoord> Draw( wxDC &dc, const wxRect &geom );
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );

	DECLARE_EVENT_TABLE();
};

#endif

