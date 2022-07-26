#include "pch.h"
#include "CMyRaytraceRenderer.h"

void CMyRaytraceRenderer::SetWindow(CWnd* p_window)
{
    m_window = p_window;
}

bool CMyRaytraceRenderer::RendererStart()
{
	m_intersection.Initialize();

	m_mstack.clear();


	// We have to do all of the matrix work ourselves.
	// Set up the matrix stack.
	CGrTransform t;
	/*
	t.SetLookAt(Eye().X(), Eye().Y(), Eye().Z(),
		Center().X(), Center().Y(), Center().Z(),
		Up().X(), Up().Y(), Up().Z());
	*/
	t[0][1] = t[0][2] = t[0][3] = 0.;  t[0][0] = 1.0;
	t[1][0] = t[1][2] = t[1][3] = 0.;  t[1][1] = 1.0;
	t[2][0] = t[2][1] = t[2][3] = 0.;  t[2][2] = 1.0;
	t[3][0] = t[3][1] = t[3][2] = 0.;  t[3][3] = 1.0;
	m_mstack.push_back(t);

	m_material = NULL;

	return true;
}

void CMyRaytraceRenderer::RendererMaterial(CGrMaterial* p_material)
{
	m_material = p_material;
}

void CMyRaytraceRenderer::RendererPushMatrix()
{
	m_mstack.push_back(m_mstack.back());
}

void CMyRaytraceRenderer::RendererPopMatrix()
{
	m_mstack.pop_back();
}

void CMyRaytraceRenderer::RendererRotate(double a, double x, double y, double z)
{
	CGrTransform r;
	r.SetRotate(a, CGrPoint(x, y, z));
	m_mstack.back() *= r;
}

void CMyRaytraceRenderer::RendererTranslate(double x, double y, double z)
{
	CGrTransform r;
	r.SetTranslate(x, y, z);
	m_mstack.back() *= r;
}

//
// Name : CMyRaytraceRenderer::RendererEndPolygon()
// Description : End definition of a polygon. The superclass has
// already collected the polygon information
//

void CMyRaytraceRenderer::RendererEndPolygon()
{
    const std::list<CGrPoint>& vertices = PolyVertices();
    const std::list<CGrPoint>& normals = PolyNormals();
    const std::list<CGrPoint>& tvertices = PolyTexVertices();

    // Allocate a new polygon in the ray intersection system
    m_intersection.PolygonBegin();
    m_intersection.Material(m_material);

    if (PolyTexture())
    {
        m_intersection.Texture(PolyTexture());
    }

    std::list<CGrPoint>::const_iterator normal = normals.begin();
    std::list<CGrPoint>::const_iterator tvertex = tvertices.begin();

    for (std::list<CGrPoint>::const_iterator i = vertices.begin(); i != vertices.end(); i++)
    {
        if (normal != normals.end())
        {
            m_intersection.Normal(m_mstack.back() * *normal);
            normal++;
        }

        if (tvertex != tvertices.end())
        {
            m_intersection.TexVertex(*tvertex);
            tvertex++;
        }

        m_intersection.Vertex(m_mstack.back() * *i);
    }

    m_intersection.PolygonEnd();
}

bool CMyRaytraceRenderer::RendererEnd()
{
	m_intersection.LoadingComplete();

	/*
	double ymin = -tan(ProjectionAngle() / 2 * GR_DTOR);
	double yhit = -ymin * 2;

	double xmin = ymin * ProjectionAspect();
	double xwid = -xmin * 2;
	*/

	int i = 0;
	for (std::vector<CGrPoint>::iterator it = m_light.begin(); it != m_light.end(); it++)
	{
		CGrPoint Lpos = *it;
		CGrTransform view = m_mstack.front();
		CGrPoint L = view * Lpos;


		for (int r = 0; r < m_rayimageheight; r++)
		{
			for (int c = 0; c < m_rayimagewidth; c++)
			{
				double colorTotal[3] = { 0, 0, 0 };

				//double x = xmin + (c + 0.5) / m_rayimagewidth * xwid;
				//double y = ymin + (r + 0.5) / m_rayimageheight * yhit;
				//double x = c + 1 - 119.834671;
				//double y = 256 - r - 126.307991;
				double x = c + 1 - m_translation[0];
				double y = 256 - r - m_translation[1];

				//Construct a Ray
				//CRay ray(CGrPoint(0, 0, 0), Normalize3(CGrPoint(x, y, -1, 0)));
				
				CGrPoint L1 = CGrPoint(0, 0, 1, 0);
				CRay ray(CGrPoint(x, y, -1000), L1);

				double t;                                   // Will be distance to intersection
				CGrPoint intersect;                         // Will by x,y,z location of intersection
				const CRayIntersection::Object* nearest;    // Pointer to intersecting object
				if (m_intersection.Intersect(ray, 1e20, NULL, nearest, t, intersect))
				{
					// We hit something...
					// for lighting 0 and 19, the face should just be black, no need for shadowfeeler
					/*
					if (Lpos[3] == 0)
					{
						m_rayimage[r][c * 3] = 0;
						m_rayimage[r][c * 3 + 1] = 0;
						m_rayimage[r][c * 3 + 2] = 0;
						continue;
					}
					*/

					// Determine information about the intersection
					CGrPoint N;
					CGrMaterial* material;
					CGrTexture* texture;
					CGrPoint texcoord;

					m_intersection.IntersectInfo(ray, nearest, t,
						N, material, texture, texcoord);

					//detect self shadow. If so, just go to next iteration.
					//L = Lpos - intersect;
					L = Lpos;
					L.Normalize3();

					CGrPoint V = CGrPoint(0,0,1);
					CGrPoint H = V + L;
					H.Normalize3();

					double shininess = 20;
					double dot_spec = Dot3(N, H);

					double ks = pow(max(dot_spec,0),shininess);
					m_rayimage[r][c * 3] = floor(255*ks);
					m_rayimage[r][c * 3 + 1] = floor(255*ks);
					m_rayimage[r][c * 3 + 2] = floor(255*ks);
 					

					/*
					double dot = Dot3(N, L);
					if (dot < 0)
					{
						m_rayimage[r][c * 3] = 0;
						m_rayimage[r][c * 3 + 1] = 0;
						m_rayimage[r][c * 3 + 2] = 0;
						continue;
					}

					//cast a ray from the interestion to the light source
					CRay  shadowfeeler(intersect, L);
					const CRayIntersection::Object* shadownearest;  // Pointer to intersecting object
					double shadowt;   // Will be distance to shadow intersection
					CGrPoint shadowintersect;  // Will by x,y,z location of shadow intersection
					bool shadowed = false;
					
					
					if (m_intersection.Intersect(shadowfeeler, 1e20, nearest, shadownearest, shadowt, shadowintersect))
					{
				
						// We intersected something.  Is it nearer than the light?
						// If the lights a vector, it clearly will be.  Otherwise, if
						// t is less than the distance to the light, we are blocking
						// the light, so we ignored this lights contribution.
						double length = (Lpos - intersect).Length3();
						if (shadowt < length)
						{
							shadowed = true;
							m_rayimage[r][c * 3] = 0;
							m_rayimage[r][c * 3 + 1] = 0;
							m_rayimage[r][c * 3 + 2] = 0;
						}
					}
					else
					{

						
						m_rayimage[r][c * 3] = 255;
						m_rayimage[r][c * 3 + 1] = 255;
						m_rayimage[r][c * 3 + 2] = 255;

					}
				*/
					
				}
				else
				{
					// We hit nothing...
					/*
					m_rayimage[r][c * 3] = 159;
					m_rayimage[r][c * 3 + 1] = 29;
					m_rayimage[r][c * 3 + 2] = 53;
					*/
					m_rayimage[r][c * 3] = 128;
					m_rayimage[r][c * 3 + 1] = 128;
					m_rayimage[r][c * 3 + 2] = 128;
				}
			}


			if ((r % 50) == 0)
			{
				m_window->Invalidate();
				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					DispatchMessage(&msg);
			}
		}
	
		std::string index = std::to_string(i);
		std::string outputname = ".\\FFHQhighmap\\" + imagename + "_" + index + ".bmp";
		SaveImage(outputname);
		i++;
	}

	return true;
}

typedef unsigned short      WORD;
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
void CMyRaytraceRenderer::SaveImage(const std::string& imagename)
{

	int width = m_rayimagewidth;
	int	height = m_rayimageheight;
	
	const int DIB_PADSIZE = 4;

	int usewidth3 = (width * 3 + (DIB_PADSIZE - 1)) / DIB_PADSIZE;
	usewidth3 *= DIB_PADSIZE;

	BITMAPFILEHEADER bmfHdr;   // Header for Bitmap file

	// Fill in the bitmap file header
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"
	bmfHdr.bfSize = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER) + height * usewidth3;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Fill in a BITMAPINFOHEADER
	BITMAPINFOHEADER bmi;

	bmi.biSize = sizeof(bmi);
	bmi.biWidth = width;
	bmi.biHeight = height;
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;
	bmi.biXPelsPerMeter = 1;
	bmi.biYPelsPerMeter = 1;
	bmi.biClrUsed = 0;
	bmi.biClrImportant = 0;

	std::string output = imagename;
	std::ofstream file(output, std::ios::binary);
	file.write((const char*)&bmfHdr, sizeof(BITMAPFILEHEADER));
	file.write((const char*)&bmi, sizeof(bmi));

	if (!file)
	{
		AfxMessageBox(TEXT("Unable to write output file file"), IDOK);
		return;
	}

	for (int i = 0; i < height; i++)
	{
		//write in BGR order
		file.write((const char*)&m_rayimage[i][0], usewidth3); 
	}

	file.close();


}
