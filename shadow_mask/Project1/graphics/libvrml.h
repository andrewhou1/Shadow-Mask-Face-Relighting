//
// Name :        libvrml.h
// Description : Main header file for the libvrml DLL
//

#pragma once


#ifdef LIBVRMLDLL
#define LIBVRMLEXPORT  __declspec( dllexport )
#else
#define LIBVRMLEXPORT  __declspec( dllimport )
#endif

class CVRMLP;

class LIBVRMLEXPORT CVRML
{
public:
    CVRML(void);
    ~CVRML(void);

    bool FileLoad(const char * m_filename);
    void Close();
    void glRender(const char *p_name=NULL);
    void glRenderDL(const char *p_name=NULL);
    const char *Error() const;

    bool UseMipmapping() const;
    bool UseMipmapping(bool s);

    const char *Directory() const;

    void Transparency(float p_transparency);
    float Transparency() const;

    class LIBVRMLEXPORT Renderer
    {
    public:
        virtual void PolygonBegin() = 0;
	    virtual void PolygonEnd() = 0;
	    virtual void Vertex(float x, float y, float z) = 0;
        virtual void Normal(float x, float y, float z) = 0;
        virtual void TexCoord(float s, float t) = 0;
        virtual void PushMatrix() = 0;
        virtual void PopMatrix() = 0;
        virtual void Translate(float x, float y, float z) = 0;
        virtual void Rotate(float a, float x, float y, float z) = 0;
        virtual void Scale(float x, float y, float z) = 0;
		virtual void MultMatrix(const double *m) = 0;
        virtual void Texture(int index);
        virtual void Material(const float *ambient, const float *diffuse, const float *specular, const float *emissive, float shininess);


        inline void Vertex3dv(const double *p_vertex) {Vertex((float)p_vertex[0], (float)p_vertex[1], (float)p_vertex[2]);}
        inline void Vertex3fv(const float *p_vertex) {Vertex(p_vertex[0], p_vertex[1], p_vertex[2]);}
        inline void Normal3dv(const double *p_normal) {Normal((float)p_normal[0], (float)p_normal[1], (float)p_normal[2]);}
        inline void Normal3fv(const float *p_normal) {Normal(p_normal[0], p_normal[1], p_normal[2]);}
        inline void TexCoord2dv(const double *p_coord) {TexCoord((float)p_coord[0], (float)p_coord[1]);}
        inline void TexCoord2fv(const float *p_coord) {TexCoord(p_coord[0], p_coord[1]);}
    };

    void Render(Renderer *p_renderer, const char *p_name=NULL);
    int GetTextureCount() const;
    bool GetTexture(int index, const BYTE *&image, int &width, int &height, 
        int &colpitch, int &rowpitch, bool &repeatS, bool &repeatT, bool &transparency) const;

private:
    CVRMLP  *m_vrml;
};


