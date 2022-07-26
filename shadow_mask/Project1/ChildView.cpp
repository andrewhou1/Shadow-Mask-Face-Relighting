
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include <fstream>      // For input streams from files
#include <string>       // For the string type
#include <sstream>      // For streams from strings
#include "framework.h"
#include "Project1.h"
#include "ChildView.h"
#include "graphics/OpenGLRenderer.h"
#include "CMyRaytraceRenderer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	m_camera.Set(0., 0., -0.5, 0., 0., 0., 1., 0. , 0.);
	m_camera.FieldOfView(27);

	m_raytrace = false;
	m_rayimage = NULL;

	//create the scene
	/*
	CGrPtr<CGrComposite> scene = new CGrComposite;
	m_scene = scene;
	*/

	// A red box
	/*
	CGrPtr<CGrMaterial> redpaint = new CGrMaterial;
	redpaint->AmbientAndDiffuse(0.8f, 0.0f, 0.0f);
	scene->Child(redpaint);

	CGrPtr<CGrComposite> redbox = new CGrComposite;
	redpaint->Child(redbox);
	redbox->Box(-5, -5, -5, 5, 5, 5);
	*/
	
	/*
	// A white human face
	CGrPtr<CGrMaterial> whitepaint = new CGrMaterial;
	whitepaint->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	scene->Child(whitepaint);	
	
	CGrPtr<CGrComposite> humanface = new CGrComposite;
	whitepaint->Child(humanface);
	//humanface->LoadOBJ("models\\345_03_01_face_valid_view3.obj");
	humanface->LoadOBJ("models\\345_03_01_face_valid_view3.obj");
	//humanface->LoadOBJ("models\\cube.obj");
	*/
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, COpenGLWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_RENDER_RAYTRACE, &CChildView::OnRenderRaytrace)
	ON_UPDATE_COMMAND_UI(ID_RENDER_RAYTRACE, &CChildView::OnUpdateRenderRaytrace)
	ON_COMMAND(ID_RENDER_RUNEXPERIMENT, &CChildView::OnRenderRunexperiment)
	ON_COMMAND(ID_RENDER_DPR, &CChildView::OnRenderDpr)
END_MESSAGE_MAP()



// CChildView message handlers
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!COpenGLWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnGLDraw(CDC* pDC)
{
	if (m_raytrace)
	{
		// Clear the color buffer
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up for parallel projection
		int width, height;
		GetSize(width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// If we got it, draw it
		if (m_rayimage)
		{
			glRasterPos3i(0, 0, 0);
			glDrawPixels(m_rayimagewidth, m_rayimageheight,
				GL_RGB, GL_UNSIGNED_BYTE, m_rayimage[0]);
		}

		glFlush();
	}
	/*
	else
	{
		//
		// Instantiate a renderer
		//

		COpenGLRenderer renderer;

		// Configure the renderer
		ConfigureRenderer(&renderer);

		//
		// Render the scene
		//

		renderer.Render(m_scene);
	}
	*/
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_camera.MouseDown(point.x, point.y);

	COpenGLWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_camera.MouseMove(point.x, point.y, nFlags))
		Invalidate();

	COpenGLWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_camera.MouseDown(point.x, point.y, 2);

	COpenGLWnd::OnRButtonDown(nFlags, point);
}

//
// Name :         CChildView::ConfigureRenderer()
// Description :  Configures our renderer so it is able to render the scene.
//                Indicates how we'll do our projection, where the camera is,
//                and where any lights are located.
//

void CChildView::ConfigureRenderer(CGrRenderer* p_renderer)
{
	// Determine the screen size so we can determine the aspect ratio
	int width, height;
	GetSize(width, height);
	//double aspectratio = double(width) / double(height);
	double aspectratio = 1;
	//
	// Set up the camera in the renderer
	//

	p_renderer->Perspective(m_camera.FieldOfView(),
		aspectratio, // The aspect ratio.
		20., // Near clipping
		1000.); // Far clipping

	// m_camera.FieldOfView is the vertical field of view in degrees.

	//
	// Set the camera location
	//

	//p_renderer->LookAt(m_camera.Eye()[0], m_camera.Eye()[1], m_camera.Eye()[2],
		//m_camera.Center()[0], m_camera.Center()[1], m_camera.Center()[2],
		//m_camera.Up()[0], m_camera.Up()[1], m_camera.Up()[2]);
		p_renderer->LookAt(0., 0., -0.5, 0., 0., 0., 0., -1., 0.);
	//
	// Set the light locations and colors
	//

	float dimd = 0.5f;
	GLfloat dim[] = { dimd, dimd, dimd, 1.0f };
	GLfloat brightwhite[] = { 1.f, 1.f, 1.f, 1.0f };

	p_renderer->AddLight(CGrPoint(1, 0.5, 1.2, 0),
		dim, brightwhite, brightwhite);
}


void CChildView::OnRenderRaytrace()
{
	m_raytrace = !m_raytrace;
	Invalidate();
	if (!m_raytrace)
		return;

	//set up the scene
	CGrPtr<CGrComposite> scene = new CGrComposite;
	m_scene = scene;
	// A white human face
	CGrPtr<CGrMaterial> whitepaint = new CGrMaterial;
	whitepaint->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	scene->Child(whitepaint);
	CGrPtr<CGrComposite> humanface = new CGrComposite;
	whitepaint->Child(humanface);
	//humanface->LoadOBJ("models\\345_03_01_face_valid_view3.obj");
	humanface->LoadOBJ("models\\345_03_01_face_valid_view3.obj");

	//GetSize(m_rayimagewidth, m_rayimageheight);
	//set up the rayimage
	m_rayimagewidth = 128;
	m_rayimageheight = 128;

	m_rayimage = new BYTE * [m_rayimageheight];

	int rowwid = m_rayimagewidth * 3;
	while (rowwid % 4)
		rowwid++; //add padding to the array

	m_rayimage[0] = new BYTE[m_rayimageheight * rowwid];
	for (int i = 1; i < m_rayimageheight; i++)
	{
		m_rayimage[i] = m_rayimage[0] + i * rowwid;
	}

	for (int i = 0; i < m_rayimageheight; i++)
	{
		// Fill the image with blue
		for (int j = 0; j < m_rayimagewidth; j++)
		{
			m_rayimage[i][j * 3] = 0;               // red
			m_rayimage[i][j * 3 + 1] = 0;           // green
			m_rayimage[i][j * 3 + 2] = BYTE(255);   // blue
		}
	}
	
	// Instantiate a raytrace object
	CMyRaytraceRenderer raytrace;

	// Generic configurations for all renderers
	ConfigureRenderer(&raytrace);

	//
	// Render the Scene
	//
	std::string imagename = ".\\output1\\345_03_01_face_valid_view3";
	raytrace.SetImage(m_rayimage, m_rayimagewidth, m_rayimageheight);
	raytrace.SetWindow(this);
	raytrace.SetImageName(imagename);
	raytrace.Render(m_scene);
	Invalidate();
}


void CChildView::OnUpdateRenderRaytrace(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_raytrace);
}

//get all the files' names under one certain directionary
#include <io.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
void GetAllFiles(string path, vector<string>& files)
{

	long   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}

}

void CChildView::OnRenderRunexperiment()
{
	//string filePath = ".\\models\\valid_views";
	string filePath = "C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\BSPnet\\EdgeDetection\\result";
	vector<string> files;
	//files.push_back(".\\models\\valid_views\\001_03_01_face_valid_view3.obj");
	GetAllFiles(filePath, files);

	//set up the 20 lighting
	vector<CGrPoint> lighting;
	lighting.push_back(CGrPoint(0., 0., 0., 0.));//f0
	lighting.push_back(CGrPoint(3.75, 0.25, 0., 1.0)); //f-1
	lighting.push_back(CGrPoint(3.75, 0.25, 1., 1.0)); //f2
	lighting.push_back(CGrPoint(3.75, 0.25, 2., 1.0)); //f3
	lighting.push_back(CGrPoint(2.85, 0.25, 2.5, 1.0)); //f4
	lighting.push_back(CGrPoint(1.5, 0.25, 2.5, 1.0)); //f5
	lighting.push_back(CGrPoint(0.75, 0.25, 2.5, 1.0)); //f6
	lighting.push_back(CGrPoint(0, 0.25, 2.5, 1.0)); //f7
	lighting.push_back(CGrPoint(-0.875, 0.25, 2.5, 1.0));//f8
	lighting.push_back(CGrPoint(-1.8, 0.25, 2.5, 1.0));//f9
	lighting.push_back(CGrPoint(-2.9, 0.25, 2.5, 1.0));//f10
	lighting.push_back(CGrPoint(-3.25, 0.25, 2, 1.0));//f11
	lighting.push_back(CGrPoint(-3.25, 0.25, 1, 1.0));//f12
	lighting.push_back(CGrPoint(-3.25, 0.25, 0, 1.0));//f13
	lighting.push_back(CGrPoint(2.85, 1.9, 2.5, 1.0));//f14
	lighting.push_back(CGrPoint(1.5, 1.9, 2.5, 1.0));//f15
	lighting.push_back(CGrPoint(0, 1.9, 2.5, 1.0));//f16
	lighting.push_back(CGrPoint(-1.8, 1.9, 2.5, 1.0));//f17
	lighting.push_back(CGrPoint(-3, 1.9, 2.5, 1.0));//f18
	lighting.push_back(CGrPoint(0., 0., 0., 0.));//f19

    //set up the rayimage
	m_rayimagewidth = 128;
	m_rayimageheight = 128;

	m_rayimage = new BYTE * [m_rayimageheight];

	int rowwid = m_rayimagewidth * 3;
	while (rowwid % 4)
		rowwid++; //add padding to the array

	m_rayimage[0] = new BYTE[m_rayimageheight * rowwid];
	for (int i = 1; i < m_rayimageheight; i++)
	{
		m_rayimage[i] = m_rayimage[0] + i * rowwid;
	}

	for (int i = 0; i < m_rayimageheight; i++)
	{
		// Fill the image with blue
		for (int j = 0; j < m_rayimagewidth; j++)
		{
			m_rayimage[i][j * 3] = 0;               // red
			m_rayimage[i][j * 3 + 1] = 0;           // green
			m_rayimage[i][j * 3 + 2] = BYTE(255);   // blue
		}
	}

	// Instantiate a raytrace object
	CMyRaytraceRenderer raytrace;
	
	// Generic configurations for all renderers
	ConfigureRenderer(&raytrace);
	raytrace.SetLighting(lighting);
	raytrace.SetImage(m_rayimage, m_rayimagewidth, m_rayimageheight);
	raytrace.SetWindow(this);

	//set up the scene
	CGrPtr<CGrComposite> scene = new CGrComposite;
	m_scene = scene;

	CGrPtr<CGrMaterial> whitepaint = new CGrMaterial;
	whitepaint->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	scene->Child(whitepaint);

	CGrPtr<CGrComposite> humanface;
	
	for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
	{
		humanface = new CGrComposite;
		whitepaint->Child(humanface);
		string filename = *it;
		humanface->LoadOBJ(filename);
		string imagename = filename.substr(filename.rfind("\\"));
		imagename = imagename.substr(0, imagename.length()- 4);
		raytrace.SetImageName(imagename);
		raytrace.Render(m_scene);

		/*
		std::list<CGrPtr<CGrObject> > poly = humanface->GetChildren();
		for (std::list<CGrPtr<CGrObject> >::iterator it = poly.begin(); it != poly.end(); it++)
		{
			if(*it != NULL)
				delete (*it);
			//poly.erase(it);
		}
		*/
		delete humanface;
		humanface = NULL;
	}
}



void CChildView::OnRenderDpr()
{
	string filePath = "C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\FaceRelight\RayTracing\\Project1\\Project1\\highlight_msp_model";
	vector<string> files;
	//files.push_back("C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\FaceRelight\\dataset\\DPR_3DMM_fitting_part1_txt\\imgHQ00001.mat.txt");
	//files.push_back("C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\FaceRelight\\dataset\\DPR_3DMM_fitting_part1_txt\\imgHQ00000.mat.txt");
	//files.push_back("C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\FaceRelight\\dataset\\DPR_3DMM_fitting_part1_txt\\imgHQ00006.mat.txt");
	//files.push_back("C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\FaceRelight\\dataset\\DPR_3DMM_fitting_part1_txt\\imgHQ00089.mat.txt");
	
	GetAllFiles(filePath, files);

	m_rayimagewidth = 256;
	m_rayimageheight = 256;

	m_rayimage = new BYTE * [m_rayimageheight];

	int rowwid = m_rayimagewidth * 3;
	while (rowwid % 4)
		rowwid++; //add padding to the array

	m_rayimage[0] = new BYTE[m_rayimageheight * rowwid];
	for (int i = 1; i < m_rayimageheight; i++)
	{
		m_rayimage[i] = m_rayimage[0] + i * rowwid;
	}

	for (int i = 0; i < m_rayimageheight; i++)
	{
		// Fill the image with blue
		for (int j = 0; j < m_rayimagewidth; j++)
		{
			m_rayimage[i][j * 3] = BYTE(255);               // red
			m_rayimage[i][j * 3 + 1] = BYTE(255);           // green
			m_rayimage[i][j * 3 + 2] = BYTE(255);   // blue
		}
	}

	// Instantiate a raytrace object
	CMyRaytraceRenderer raytrace;

	// Generic configurations for all renderers
	ConfigureRenderer(&raytrace);
	//raytrace.SetLighting(Lighting);
	raytrace.SetImage(m_rayimage, m_rayimagewidth, m_rayimageheight);
	raytrace.SetWindow(this);

	//set up the scene
	CGrPtr<CGrComposite> scene = new CGrComposite;
	m_scene = scene;

	CGrPtr<CGrMaterial> whitepaint = new CGrMaterial;
	whitepaint->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	scene->Child(whitepaint);

	//DPR dataset 
	/*
	string lightingfolder = "C:\\Users\\zhang\\OneDrive - Michigan State University\\Ze\\research\\FaceRelight\\code\\LightingDir";

	for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
	{
		CGrPtr<CGrComposite> humanface = new CGrComposite;
		string filename = *it;
		CGrPoint t = humanface->LoadOBJ(filename);
		whitepaint->Child(humanface);
		string imagename = filename.substr(filename.rfind("\\"));
		imagename = imagename.substr(0, imagename.length() - 8);
		//lightin file name: imgHQ00000_light_00.txt
		vector<string> lightingnamevector;
		string file1 = "_light_00.txt";
		string file2 = "_light_01.txt";
		string file3 = "_light_02.txt";
		string file4 = "_light_03.txt";
		string file5 = "_light_04.txt";
		string lightingname = lightingfolder;
		lightingname = lightingname.append(imagename);
		//lightingname = lightingname.substr(0, lightingname.length() - 4);
		string lightingname1 = lightingname;
		lightingname1 = lightingname1.append(file1);
		lightingnamevector.push_back(lightingname1);
		string lightingname2 = lightingname;
		lightingname2 = lightingname2.append(file2);
		lightingnamevector.push_back(lightingname2);
		string lightingname3 = lightingname;
		lightingname3 = lightingname3.append(file3);
		lightingnamevector.push_back(lightingname3);
		string lightingname4 = lightingname;
		lightingname4 = lightingname4.append(file4);
		lightingnamevector.push_back(lightingname4);
		string lightingname5 = lightingname;
		lightingname5 = lightingname5.append(file5);
		lightingnamevector.push_back(lightingname5);

		std::vector<CGrPoint> Lighting;

		for (vector<string>::iterator num = lightingnamevector.begin(); num != lightingnamevector.end(); num++)
		{
			string name = *num;
			ifstream str(name);
			if (!str)
			{
				AfxMessageBox(L"File not found");
				return;
			}

			string line;
			double x, y, z;
			while (getline(str, line))
			{

				istringstream lstr(line);
				//string code;
				//lstr >> code;
				lstr >> x >> y >> z;
			}

			CGrPoint lighting1 = CGrPoint(z,x,y, 0.0);
			Lighting.push_back(lighting1);

		}

		delete humanface;
		humanface = NULL;
	}
	*/

	//FFHQ lighting
	std::vector<CGrPoint> Lighting;
	Lighting.push_back(CGrPoint(-0.5696, 0.6466, 0.5073, 1.0)); //lighting0
	Lighting.push_back(CGrPoint(0.2328, 0.9722, -0.024, 1.0)); //lighting1
	Lighting.push_back(CGrPoint(-0.4195, 0.2160, -0.8817, 1.0)); //lighting2
	Lighting.push_back(CGrPoint(-0.6290, 0.7659, -0.1334, 1.0));//lighting3
	Lighting.push_back(CGrPoint(-0.3043, -0.3276, 0.8945, 1.0));//lighting4
	Lighting.push_back(CGrPoint(-0.3043, -0.3276, 0.8945, 1.0));//lighting5
	Lighting.push_back(CGrPoint(-0.3043, -0.3276, 0.8945, 1.0));//lighting6
	Lighting.push_back(CGrPoint(-0.3043, -0.3276, 0.8945, 1.0));//lighting7


	for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
	{
		CGrPtr<CGrComposite> humanface = new CGrComposite;
		string filename = *it;
		CGrPoint t = humanface->LoadOBJ(filename);
		whitepaint->Child(humanface);
		string imagename = filename.substr(filename.rfind("\\"));
		imagename = imagename.substr(0, imagename.length() - 8);
		raytrace.SetTranslationVector(t);
		raytrace.SetLighting(Lighting);
		raytrace.SetImageName(imagename);
		raytrace.Render(m_scene);

		delete humanface;
		humanface = NULL;
	}

}
