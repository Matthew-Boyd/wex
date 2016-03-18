#ifndef __gleasy_h
#define __gleasy_h

#include <vector>
#include <wx/window.h>
#include <wx/glcanvas.h>

struct wxGLPoint3D
{
	wxGLPoint3D() : x(0),y(0), z(0) { }
	wxGLPoint3D( const wxGLPoint3D &p ) :x(p.x), y(p.y), z(p.z) { }
	wxGLPoint3D( float _x, float _y, float _z ) : x(_x), y(_y), z(_z) { }
	float x, y, z;
};


class wxGLTrackball
{
protected:
	float m_quat[4];
	float m_lastX, m_lastY;
public:
	wxGLTrackball();

	// call this every time when mouse moves
	void Mouse( float mx, float my );

	// call this when mouse moves and you want to rotate the scene,
	void Spin( float mx, float my, float win_width, float win_height );

	// call this for a rotation matrix to use with glMultMatrixf()
	void GetRotationMatrix( GLfloat m[4][4] );
};

class wxGLEasyCanvas : public wxGLCanvas
{
public:
	wxGLEasyCanvas( wxWindow *parent, int id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );
	virtual ~wxGLEasyCanvas();

	void SetMinMax( const wxGLPoint3D &min, const wxGLPoint3D &max );
	void ShowAxes( float len );
protected:
	virtual void OnRender();

	void Color( const wxColour &c );
	void PointSize( float p );
	void Point( float x, float y, float z );
	void Point( const wxGLPoint3D &p );
	void BeginPoints(); // optional optimization for rendering lots of points
	void EndPoints();
	void Points( const std::vector<wxGLPoint3D> &list );
	void LineWidth( float w );
	void Line( const wxGLPoint3D &p1, const wxGLPoint3D &p2 );
	void Lines( const std::vector<wxGLPoint3D> &list );
	void Text( const wxGLPoint3D &p, const wxString &text );

	wxGLContext m_glContext;
	bool m_pointListMode;
	float m_lastX, m_lastY, m_scale;
	wxGLTrackball m_trackball;
	wxGLPoint3D m_offset;
	wxGLPoint3D m_min, m_max;
	float m_axesLength;
	struct {
		float left, right, top, bottom, znear, zfar;
	} m_orth;
	wxGLPoint3D m_last3D;
	
	void OnChar( wxKeyEvent & );
	void OnMouse( wxMouseEvent & );
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );
	DECLARE_EVENT_TABLE()
};


class wxGLEasyCanvasTest : public wxGLEasyCanvas
{
public:
	wxGLEasyCanvasTest( wxWindow *parent );

	void OnMenu( wxCommandEvent & );
	void OnRightDown( wxMouseEvent &evt );


protected:
	virtual void OnRender();
	std::vector<wxGLPoint3D> m_data;
	DECLARE_EVENT_TABLE();
};


#endif
