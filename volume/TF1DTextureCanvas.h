#ifndef TF1DTEXTURECANVAS_H
#define TF1DTEXTURECANVAS_H

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer> 

class ModelData;
class TF1DMappingCanvas;



class TF1DTextureCanvas : public QOpenGLWidget,protected QOpenGLFunctions
{
    Q_OBJECT

public:
    TF1DTextureCanvas(ModelData *model,TF1DMappingCanvas * tf, QWidget *parent = 0);
    ~TF1DTextureCanvas();

protected:
    void initializeGL();
	void resizeGL(int width, int height);
    void paintGL();

private:

	ModelData *modelData;		///< model data
	TF1DMappingCanvas * m_transferFunction;

	//Background Primitive
	QOpenGLShaderProgram m_bgShader;
	QOpenGLVertexArrayObject m_bgVAO;
	QOpenGLBuffer m_bgVBO;
	unsigned int m_bgVertPos;
	unsigned int m_bgColorPos;

	//Transfer Function Texture
	QOpenGLShaderProgram m_tfShader;
	QOpenGLVertexArrayObject m_tfVAO;
	QOpenGLBuffer m_tfVBO;
	unsigned int m_tfVertPos;
	unsigned int m_texture;

	QMatrix4x4 m_othoMat;
					// 2d drawing
};

#endif