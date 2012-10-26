#include <limits>
#include <numeric>

#include <wx/wx.h>
#include <wx/tokenzr.h>

#include "plot/plplotctrl.h"
#include "plot/plscatterplot.h"

#include "dview/dvtimeseriesdataset.h"
#include "dview/dvselectionlist.h"
#include "dview/dvscatterplotctrl.h"

static const wxString NO_UNITS("ThereAreNoUnitsForThisAxis.");

class wxDVScatterPlot : public wxPLScatterPlot
{
private:
	wxDVTimeSeriesDataSet *m_x, *m_y;
public:
	wxDVScatterPlot( wxDVTimeSeriesDataSet *x, wxDVTimeSeriesDataSet *y )
		: m_x(x), m_y(y)
	{
	}

	virtual wxRealPoint At( size_t i ) const
	{
		double xx = i < m_x->Length() ? m_x->At(i).y : std::numeric_limits<double>::quiet_NaN();
		double yy = i < m_y->Length() ? m_y->At(i).y : std::numeric_limits<double>::quiet_NaN();
		return wxRealPoint( xx, yy );
	}

	virtual size_t Len() const
	{
		size_t xlen = m_x->Length();
		size_t ylen = m_y->Length();
		return xlen < ylen ? xlen : ylen;
	}
};


enum { wxID_SCATTER_DATA_SELECTOR = wxID_HIGHEST + 1 };

BEGIN_EVENT_TABLE(wxDVScatterPlotCtrl, wxPanel)
	EVT_DVSELECTIONLIST( wxID_SCATTER_DATA_SELECTOR, wxDVScatterPlotCtrl::OnChannelSelection )
END_EVENT_TABLE()

wxDVScatterPlotCtrl::wxDVScatterPlotCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, 
	const wxSize& size, long style, const wxString& name)
	: wxPanel(parent, id, pos, size, style, name)
{
	m_plotSurface = new wxPLPlotCtrl(this, wxID_ANY);
	m_plotSurface->ShowTitle( false );
	m_plotSurface->ShowLegend( false );

	m_dataSelectionList = new wxDVSelectionListCtrl(this, wxID_SCATTER_DATA_SELECTOR,2,true);

	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(m_plotSurface, 1, wxEXPAND|wxALL, 10);
	topSizer->Add(m_dataSelectionList, 0, wxEXPAND, 0);
	SetSizer(topSizer);

	m_xDataIndex = -1;
}

//*** DATA SET HANDLING ***
void wxDVScatterPlotCtrl::AddDataSet(wxDVTimeSeriesDataSet* d, const wxString& group, bool update_ui)
{
	m_dataSets.push_back( d );
	m_dataSelectionList->Append( d->GetTitleWithUnits(), group );

	if (update_ui)
		Layout();
}

void wxDVScatterPlotCtrl::RemoveDataSet(wxDVTimeSeriesDataSet* d)
{
	int index = -1;
	for (size_t i=0;i<m_dataSets.size();i++)
		if ( m_dataSets[i] == d )
			index = i;

	if (index == m_xDataIndex)
		m_xDataIndex = -1;

	m_yDataIndices.erase( std::find( m_yDataIndices.begin(), m_yDataIndices.end(), index ) );

	m_dataSets.erase( m_dataSets.begin() + index);
	m_dataSelectionList->RemoveAt(index);
	m_dataSelectionList->Refresh();
	m_dataSelectionList->Update();

	UpdatePlotWithChannelSelections();

	Layout();
	Refresh();
}

void wxDVScatterPlotCtrl::RemoveAllDataSets()
{
	m_dataSets.clear();
	m_dataSelectionList->RemoveAll();

	m_yDataIndices.clear();

	m_xDataIndex = -1;

	UpdatePlotWithChannelSelections();

	Layout();
	Refresh();
}


//*** MEMBER FUNCTIONS ***//
void wxDVScatterPlotCtrl::SetXAxisChannel(int index)
{
	m_xDataIndex = index;
	UpdatePlotWithChannelSelections();
}

void wxDVScatterPlotCtrl::AddYAxisChannel(int index)
{
	m_yDataIndices.push_back(index);
	UpdatePlotWithChannelSelections();
	RefreshDisabledCheckBoxes();
}

void wxDVScatterPlotCtrl::RemoveYAxisChannel(int index)
{
	m_yDataIndices.erase( std::find( m_yDataIndices.begin(), m_yDataIndices.end(), index ) );
	UpdatePlotWithChannelSelections();
	RefreshDisabledCheckBoxes();
}

void wxDVScatterPlotCtrl::UpdatePlotWithChannelSelections()
{
	m_plotSurface->DeleteAllPlots();
	m_plotSurface->DeleteAxes();

	if (m_xDataIndex < 0 || (size_t)m_xDataIndex >= m_dataSets.size())
		return;

	for (size_t i=0; i<m_yDataIndices.size(); i++)
	{
		if ( (size_t)m_yDataIndices[i] < m_dataSets.size() )
		{
			wxDVScatterPlot *p = new wxDVScatterPlot(m_dataSets[m_xDataIndex], m_dataSets[m_yDataIndices[i]]);
			p->SetLabel(m_dataSets[m_yDataIndices[i]]->GetSeriesTitle());
			p->SetSize( 2 );
			p->SetColour( m_dataSelectionList->GetColourForIndex(m_yDataIndices[i]) );

			wxString units = m_dataSets[m_yDataIndices[i]]->GetUnits();


			wxPLPlotCtrl::AxisPos yap = wxPLPlotCtrl::Y_LEFT;
			wxString y1Units = NO_UNITS, y2Units = NO_UNITS;

			if ( m_plotSurface->GetYAxis1() )
				y1Units = m_plotSurface->GetYAxis1()->GetLabel();

			if ( m_plotSurface->GetYAxis2() )
				y2Units = m_plotSurface->GetYAxis2()->GetLabel();

			if ( m_plotSurface->GetYAxis1() && y1Units == units )
				yap = wxPLPlotCtrl::Y_LEFT;
			else if ( m_plotSurface->GetYAxis2() && y2Units == units )
				yap = wxPLPlotCtrl::Y_RIGHT;
			else if ( m_plotSurface->GetYAxis1() == 0 )
				yap = wxPLPlotCtrl::Y_LEFT;
			else
				yap = wxPLPlotCtrl::Y_RIGHT;

			m_plotSurface->AddPlot( p, wxPLPlotCtrl::X_BOTTOM, yap );
			m_plotSurface->GetAxis( yap )->SetLabel( units );
		}
	}

	if (m_plotSurface->GetXAxis1())
		m_plotSurface->GetXAxis1()->SetLabel( m_dataSets[m_xDataIndex]->GetSeriesTitle() + 
			" (" + m_dataSets[m_xDataIndex]->GetUnits() + ")" );
}

void wxDVScatterPlotCtrl::RefreshDisabledCheckBoxes()
{
	wxString axis1Label = NO_UNITS;
	wxString axis2Label = NO_UNITS;

	if (m_plotSurface->GetYAxis1())
		axis1Label = m_plotSurface->GetYAxis1()->GetLabel();
	if (m_plotSurface->GetYAxis2())
		axis2Label = m_plotSurface->GetYAxis2()->GetLabel();

	if (axis1Label != NO_UNITS
		&& axis2Label != NO_UNITS
		&& axis1Label != axis2Label)
	{
		for (int i=0; i<m_dataSelectionList->Length(); i++)
			m_dataSelectionList->Enable(i, 1, axis1Label == m_dataSets[i]->GetUnits() 
				|| axis2Label == m_dataSets[i]->GetUnits());
	}
	else
	{
		for (int i=0; i<m_dataSelectionList->Length(); i++)
			m_dataSelectionList->Enable(i, 1, true);
	}
}

wxDVSelectionListCtrl* wxDVScatterPlotCtrl::GetScatterSelectionList()
{
	return m_dataSelectionList;
}

void wxDVScatterPlotCtrl::SelectXDataAtIndex(int index)
{
	if (m_dataSelectionList->IsSelected(index, 0)) return;

	m_dataSelectionList->SelectRowInCol(index, 0);
	SetXAxisChannel(index);
}

void wxDVScatterPlotCtrl::SelectYDataAtIndex(int index)
{
	if (m_dataSelectionList->IsSelected(index, 1)) return;

	m_dataSelectionList->SelectRowInCol(index, 1);
	AddYAxisChannel(index);
}

void wxDVScatterPlotCtrl::SetXSelectedName(const wxString& name)
{
	int index = m_dataSelectionList->SelectRowWithNameInCol(name, 0);
	if (index != -1)
		SetXAxisChannel(index);

	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}

void wxDVScatterPlotCtrl::SetYSelectedNames(const wxString& names)
{
	//ClearAllChannelSelections();

	wxStringTokenizer tkz(names, ";");

	while(tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		int index = m_dataSelectionList->SelectRowWithNameInCol(token, 1);
		if (index != -1)
			AddYAxisChannel(index);
	}
}

void wxDVScatterPlotCtrl::OnChannelSelection( wxCommandEvent & )
{
	int row, col;
	bool selected;

	m_dataSelectionList->GetLastEventInfo(&row, &col, &selected);

	if (col == 0)
	{
		SetXAxisChannel(row);
	}
	else if (col == 1)
	{
		if (selected)
			AddYAxisChannel(row);
		else
			RemoveYAxisChannel(row);
	}

	m_plotSurface->Invalidate();
	m_plotSurface->Refresh();
}
