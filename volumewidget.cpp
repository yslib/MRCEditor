#include "volumewidget.h"
#include "abstractslicedatamodel.h"
#include "markmodel.h"
#include <QDebug>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <iostream>
#include "volume/Shaders/shaderprogram.h"
#define GLERROR(str)									\
	{													\
		GLenum err;										\
		while ((err = glGetError()) != GL_NO_ERROR)		\
		{												\
			std::cout<<err<<" "<<str<<std::endl;		\
		}												\
	}													\

static const char *vertexShaderSource =
"#version 150\n"
"#extension GL_ARB_explicit_attrib_location : enable\n"
"layout(location = 0) in vec4 vertex;\n"
"layout(location = 1) in vec3 normal;\n"
"layout(location = 2) in vec2 tex;"
"out vec2 texCoord;\n"
"out vec3 vert;\n"
"out vec3 vertNormal;\n"
"uniform mat4 projMatrix;\n"
"uniform mat4 mvMatrix;\n"
"uniform mat3 normalMatrix;\n"
"void main() {\n"
"   vert = vertex.xyz;\n"
"   vertNormal = normalMatrix * normal;\n"
"	texCoord = tex;\n"
"   gl_Position = projMatrix * mvMatrix * vertex;\n"
"}\n";


static const char *fragmentShaderSource =
"#version 150\n"
"in highp vec3 vert;\n"
"in highp vec3 vertNormal;\n"
"out highp vec4 fragColor;\n"
"in vec2 texCoord;\n"
"uniform highp vec3 lightPos;\n"
"uniform sampler2D tex;\n"
"void main() {\n"
"   highp vec3 L = normalize(lightPos - vert);\n"
"   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
"   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
"   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
"   fragColor = vec4(1.0,0.0,0.0,1.0);\n"
"}\n";


static const char * positionFragShaderSource =
"#version 150\n"
"in vec4 texCoord;\n"
"out vec4 fragColor;\n"
"void main(){\n"
" fragColor = texCoord;\n"
"}\n";

static const char * positionVertShaderSource =
"#version 150\n"
"in vec3 vertex;\n"
"out vec4 texCoord;\n"
"uniform mat4 projMatrix;\n"
"uniform mat4 othoMatrix;\n"
"uniform mat4 worldMatrix;\n"
"uniform mat4 viewMatrix;\n"
"void main() {\n"
"   vec4 vert = vec4(vertex,1.0);\n"
"	texCoord = vert;\n"
"   gl_Position = othoMatrix * viewMatrix * worldMatrix * vert;\n"
"}\n";

//static const float xCoord = 1.0, yCoord = 1.0, zCoord = 1.0;
//float positionVert[] = {
//	0,0,0,
//	xCoord, 0, 0 ,
//	0, yCoord, 0 ,
//	xCoord, yCoord, 0 ,
//	0, 0, zCoord ,
//	xCoord, 0, zCoord ,
//	0, yCoord, zCoord ,
//	xCoord, yCoord, zCoord
//};
const static int faceIndex[] = { 1, 3, 7, 5, 0, 4, 6, 2, 2, 6, 7, 3, 0, 1, 5, 4, 4, 5, 7, 6, 0, 2, 3, 1 };


static const float xCoord = 1.0, yCoord = 1.0, zCoord = 1.0;
float positionVert[] = {
	0,0,0,
	xCoord, 0, 0 ,
	0, yCoord, 0 ,
	xCoord, yCoord, 0 ,
	0, 0, zCoord ,
	xCoord, 0, zCoord ,
	0, yCoord, zCoord ,
	xCoord, yCoord, zCoord ,
};

static QVector<QVector2D> cubeTexCoord = {
	{ 0.f,0.f },{ 1.0f,0.f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
	{ 0.f,0.f },{ 0.f,1.0f },{ 0.f,0.f },
};

static QVector<QVector3D> cubeVert =
{
	//back

	{ 0.5f, -0.5f, -0.5f },{ 0.f,0.f,-1.f },
	{ -0.5f, -0.5f, -0.5f },{ 0.f,0.f,-1.f },
	{ 0.5f,  0.5f, -0.5f },{ 0.f,0.f,-1.f },
	{ -0.5f,  0.5f, -0.5f } ,{ 0.f,0.f,-1.f },
	{ 0.5f,  0.5f, -0.5f } ,{ 0.f,0.f,-1.f },
	{ -0.5f, -0.5f, -0.5f } ,{ 0.f,0.f,-1.f },
	//front
	{ -0.5f, -0.5f,  0.5f },{0.f,0.f,1.f},
	{ 0.5f, -0.5f,  0.5f },{ 0.f,0.f,1.f },
	{ 0.5f,  0.5f,  0.5f },{ 0.f,0.f,1.f },
	{ 0.5f,  0.5f,  0.5f },{ 0.f,0.f,1.f },
	{ -0.5f,  0.5f,  0.5f },{ 0.f,0.f,1.f },
	{ -0.5f, -0.5f,  0.5f },{ 0.f,0.f,1.f },
	//left
	{ -0.5f,  0.5f,  0.5f },{-1.0f,0.f,0.f},
	{ -0.5f,  0.5f, -0.5f },{ -1.0f,0.f,0.f },
	{ -0.5f, -0.5f, -0.5f },{ -1.0f,0.f,0.f },
	{ -0.5f, -0.5f, -0.5f },{ -1.0f,0.f,0.f },
	{ -0.5f, -0.5f,  0.5f },{ -1.0f,0.f,0.f },
	{ -0.5f,  0.5f,  0.5f },{ -1.0f,0.f,0.f },
	//right
	{ 0.5f,  0.5f, -0.5f },{ 1.0f,0.f,0.f },
	{ 0.5f,  0.5f,  0.5f },{ 1.0f,0.f,0.f },
	{ 0.5f, -0.5f, -0.5f } ,{ 1.0f,0.f,0.f },
	{ 0.5f, -0.5f,  0.5f } ,{ 1.0f,0.f,0.f },
	{ 0.5f, -0.5f, -0.5f } ,{ 1.0f,0.f,0.f },
	{ 0.5f,  0.5f,  0.5f } ,{ 1.0f,0.f,0.f },
	//bottom
	{ -0.5f, -0.5f, -0.5f },{ 0.f,-1.f,0.f },
	{ 0.5f, -0.5f, -0.5f },{ 0.f,-1.f,0.f },
	{ 0.5f, -0.5f,  0.5f },{ 0.f,-1.f,0.f },
	{ 0.5f, -0.5f,  0.5f } ,{ 0.f,-1.f,0.f },
	{ -0.5f, -0.5f,  0.5f } ,{ 0.f,-1.f,0.f },
	{ -0.5f, -0.5f, -0.5f }  ,{ 0.f,-1.f,0.f },
	//up
	{ 0.5f,  0.5f, -0.5f }  ,{ 0.f,1.f,0.f },
	{ -0.5f,  0.5f, -0.5f }  ,{ 0.f,1.f,0.f },
	{ 0.5f,  0.5f,  0.5f }  ,{ 0.f,1.f,0.f },
	{ -0.5f,  0.5f,  0.5f } ,{ 0.f,1.f,0.f },
	{ 0.5f,  0.5f,  0.5f } ,{ 0.f,1.f,0.f },
	{ -0.5f,  0.5f, -0.5f }  ,{ 0.f,1.f,0.f },
};




GradientCalculator::GradientCalculator(const AbstractSliceDataModel * slice, const MarkModel * mark, QObject * parent)
	:QObject(parent),
	m_sliceModel(slice),
	m_mark(mark),
	m_ready(false)
{
}

void GradientCalculator::setDataModel(const AbstractSliceDataModel* slice)
{
	if (m_sliceModel == slice)
		return;
	m_sliceModel = slice;
	m_ready = false;
}

void GradientCalculator::setMarkModel(const MarkModel* mark)
{
	//TODO::
}

bool GradientCalculator::ready() const
{
	return m_ready /*m_mark != nullptr*/;
}

bool GradientCalculator::hasData() const
{
	return m_sliceModel != nullptr;
}

unsigned char * GradientCalculator::data() const
{
	return m_gradient.get();
}

void GradientCalculator::init()
{
	if (hasData() == false)
		return;
	const int z = m_sliceModel->topSliceCount();
	const int y = m_sliceModel->rightSliceCount();
	const int x = m_sliceModel->frontSliceCount();
	QMutexLocker locker(&m_mutex);
	m_gradient.reset(new unsigned char[x*y*z * 4]);
}

QVector3D GradientCalculator::triCubicIntpGrad(const unsigned char* pData, double px, double py, double pz)
{
	double ulo, vlo, wlo;
	const double ut = modf(px, &ulo);
	const double vt = modf(py, &vlo);
	const double wt = modf(pz, &wlo);

	int		xlo = int(ulo);
	int		ylo = int(vlo);
	int		zlo = int(wlo);

	// We need all the voxels around the primary
	double voxels[4][4][4];
	{
		for (int z = 0; z < 4; z++) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					int px = (xlo - 1 + x);
					int py = (ylo - 1 + y);
					int pz = (zlo - 1 + z);
					voxels[z][y][x] = value(pData, px, py, pz);
				}
			}
		}
	}

	QVector3D direction;

	// Now that we have all our voxels, run the cubic interpolator in one dimension to collapse it (we choose to collapase x)
	double voxelcol[4][4];
	double voxelcol2[4];
	int x, y, z;
	for (z = 0; z < 4; z++) {
		for (y = 0; y < 4; y++) {
			voxelcol[z][y] = cubicIntpGrad(voxels[z][y][0], voxels[z][y][1], voxels[z][y][2], voxels[z][y][3], ut);
		}
	}
	// Then collapse the y dimension
	for (z = 0; z < 4; z++) {
		voxelcol2[z] = cubicIntpValue(voxelcol[z][0], voxelcol[z][1], voxelcol[z][2], voxelcol[z][3], vt);
	}

	// The collapse the z dimension to get our value
	direction.setX(cubicIntpValue(voxelcol2[0], voxelcol2[1], voxelcol2[2], voxelcol2[3], wt));

	for (z = 0; z < 4; z++) {
		for (x = 0; x < 4; x++) {
			voxelcol[z][x] = cubicIntpGrad(voxels[z][0][x], voxels[z][1][x], voxels[z][2][x], voxels[z][3][x], vt);
		}
	}
	// Then collapse the x dimension
	for (z = 0; z < 4; z++) {
		voxelcol2[z] = cubicIntpValue(voxelcol[z][0], voxelcol[z][1], voxelcol[z][2], voxelcol[z][3], ut);
	}

	// The collapse the z dimension to get our value
	direction.setY(cubicIntpValue(voxelcol2[0], voxelcol2[1], voxelcol2[2], voxelcol2[3], wt));

	for (y = 0; y < 4; y++) {
		for (x = 0; x < 4; x++) {
			voxelcol[y][x] = cubicIntpGrad(voxels[0][y][x], voxels[1][y][x], voxels[2][y][x], voxels[3][y][x], wt);
		}
	}
	// Then collapse the x dimension
	for (y = 0; y < 4; y++) {
		voxelcol2[y] = cubicIntpValue(voxelcol[y][0], voxelcol[y][1], voxelcol[y][2], voxelcol[y][3], ut);
	}

	// The collapse the z dimension to get our value
	direction.setZ(cubicIntpValue(voxelcol2[0], voxelcol2[1], voxelcol2[2], voxelcol2[3], vt));

	return direction;
}

double GradientCalculator::value(const unsigned char* pData, double x, double y, double z) const
{
	const int xiSize = m_sliceModel->topSliceCount();
	const int yiSize = m_sliceModel->rightSliceCount();
	const int ziSize = m_sliceModel->frontSliceCount();
	x = std::max(std::min(x + 0.5, xiSize - 1.0), 0.0);
	y = std::max(std::min(y + 0.5, yiSize - 1.0), 0.0);
	z = std::max(std::min(z + 0.5, ziSize - 1.0), 0.0);
	int index = (int)z * xiSize * yiSize + (int)y * xiSize + (int)x;
	return pData[index];
}

double GradientCalculator::cubicIntpGrad(double v0, double v1, double v2, double v3, double mu)
{
	const double mu2 = mu * mu;
	const double mu3 = mu2 * mu;

	const double a0 = 0.0 * v0 + 0.0 * v1 + 0.0 * v2 + 0.0 * v3;
	const double a1 = -1.0 * v0 + 3.0 * v1 - 3.0 * v2 + 1.0 * v3;
	const double a2 = 2.0 * v0 - 4.0 * v1 + 2.0 * v2 + 0.0 * v3;
	const double a3 = -1.0 * v0 + 0.0 * v1 + 1.0 * v2 + 0.0 * v3;

	return (a0 * mu3 + a1 * mu2 + a2 * mu + a3) / 2.0;
}

double GradientCalculator::cubicIntpValue(double v0, double v1, double v2, double v3, double mu)
{
	const double mu2 = mu * mu;
	const double mu3 = mu2 * mu;

	const double a0 = -1.0 * v0 + 3.0 * v1 - 3.0 * v2 + 1.0 * v3;
	const double a1 = 3.0 * v0 - 6.0 * v1 + 3.0 * v2 + 0.0 * v3;
	const double a2 = -3.0 * v0 + 0.0 * v1 + 3.0 * v2 + 0.0 * v3;
	const double a3 = 1.0 * v0 + 4.0 * v1 + 1.0 * v2 + 0.0 * v3;

	return (a0 * mu3 + a1 * mu2 + a2 * mu + a3) / 6.0;
}

void GradientCalculator::calcGradent()
{
	if (hasData() == false)
		return;
	init();
	const int ziSize = m_sliceModel->topSliceCount();
	const int yiSize = m_sliceModel->rightSliceCount();
	const int xiSize = m_sliceModel->frontSliceCount();
	const auto pOriginalData = m_sliceModel->constData();

	//QMutexLocker locker(&m_mutex);
	if (m_gradient == nullptr)
		return;
	// calculate gradient and the normalized direction
#pragma omp parallel for
	for (int i = 0; i < ziSize; ++i) {
		for (int j = 0; j < yiSize; ++j) {
			for (int k = 0; k < xiSize; ++k) {
				unsigned int index = i * xiSize * yiSize + j * xiSize + k;
				// for 16 bit, already convert to 8 bit
				m_gradient[index * 4 + 3] = pOriginalData[index];
				QVector3D gradient = triCubicIntpGrad(pOriginalData, k, j, i);		// x, y, z
				if (gradient.lengthSquared() > 1e-10) {
					gradient.normalize();
					m_gradient[index * 4 + 0] = (unsigned char)((gradient.x() + 1.0) / 2.0 * 255 + 0.5);
					m_gradient[index * 4 + 1] = (unsigned char)((gradient.y() + 1.0) / 2.0 * 255 + 0.5);
					m_gradient[index * 4 + 2] = (unsigned char)((gradient.z() + 1.0) / 2.0 * 255 + 0.5);
				}
				else {	// gradient = (0, 0, 0)
					m_gradient[index * 4 + 0] = 128;
					m_gradient[index * 4 + 1] = 128;
					m_gradient[index * 4 + 2] = 128;
				}
			}
		}
	}
	emit finished();
	m_ready = true;
}




VolumeWidget::VolumeWidget(AbstractSliceDataModel * dataModel, MarkModel * markModel, QWidget * parent)
	:QOpenGLWidget(parent),
	m_dataModel(dataModel),
	m_markModel(markModel),
	m_rayStep(0.02),
	m_camera(QVector3D(0.f, 0.f, 10.f)),
	m_trans(0.0, 0.0, 0.0),
	m_scale(1.0, 1.0, 1.0),
	m_gradCalc(dataModel, markModel),
	m_tfTexture(QOpenGLTexture::Target1D),
	m_volumeTexture(QOpenGLTexture::Target3D),
	m_fbo(nullptr),
	m_positionEBO(QOpenGLBuffer::IndexBuffer),
	m_positionVBO(QOpenGLBuffer::VertexBuffer),
	m_tfData(new float[256 * 4])
{

}

void VolumeWidget::setDataModel(AbstractSliceDataModel * model)
{
	m_dataModel = model;
	updateVolumeData();
	emit dataModelChanged();
	update();
}

void VolumeWidget::setMarkModel(MarkModel* model)
{
	m_markModel = model;
	updateMarkData();
	emit markModelChanged();
	update();
}

//ShaderDataInterface

QSize VolumeWidget::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize VolumeWidget::sizeHint() const
{
	return QSize(800, 600);
}

VolumeWidget::~VolumeWidget()
{

}

void VolumeWidget::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(1.0, 1.0, 1.0, 1.0);
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &VolumeWidget::cleanup);
	glEnable(GL_DEPTH_TEST);
	;
	// Initialize Front and back face texture
	m_positionShader.reset(new PositionShader);
	m_positionShader->link();
	Q_ASSERT_X(m_positionShader->isLinked(), "VolumeWidget::initializeGL", "positionShader linking failed.");
	m_positionVAO.create();
	Q_ASSERT_X(m_positionVAO.isCreated(), "VolumeWidget::initializeGL", "VAO is not created.");
	QOpenGLVertexArrayObject::Binder positionVAOBinder(&m_positionVAO);
	m_positionVBO.create();
	m_positionVBO.bind();
	m_positionVBO.allocate(positionVert, sizeof(positionVert));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(0));	//vertex
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(0));	//texture coord
	m_positionShader->bind();
	m_positionEBO.create();
	m_positionEBO.bind();
	m_positionEBO.allocate(faceIndex, sizeof(faceIndex));
	m_positionVBO.release();

	// Initilize ray casting shader
	m_currentShader.reset(new RayCastingShader);
	m_currentShader->link();
	Q_ASSERT_X(m_currentShader->isLinked(), "VolumeWidget::initializeGL", "currentShader linking failed.");
	m_rayCastingTextureVAO.create();
	QOpenGLVertexArrayObject::Binder binder(&m_rayCastingTextureVAO);
	m_rayCastingTextureVBO.create();
	m_rayCastingTextureVBO.bind();
	m_currentShader->bind();
	m_rayCastingTextureVBO.allocate(8 * sizeof(GLfloat));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), reinterpret_cast<void*>(0));
	m_rayCastingTextureVBO.release();

	// initialize cube 
	m_cubeShader.reset(new QOpenGLShaderProgram);
	m_cubeShader->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
	m_cubeShader->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
	m_cubeShader->link();
	m_cubeVAO.create();
	m_cubeVAO.bind();
	m_cubeVBO.create();
	m_cubeVBO.bind();
	const int c1 = cubeVert.count() * sizeof(float) * 3;
	const int c2 = cubeTexCoord.count() * sizeof(float) * 2;
	m_cubeVBO.allocate(c1 + c2);
	m_cubeVBO.write(0, cubeVert.constData(), c1);
	m_cubeVBO.write(c1, cubeTexCoord.constData(), c2);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(float)));
	m_cubeVBO.release();
	m_cubeVAO.release();

	// Default matrix parameters
	m_scale = QVector3D(1.0, 1.0, 1.0);
	m_trans = QVector3D(0, 0, 0);
	m_trans -= m_scale / 2;
	m_world.setToIdentity();
	m_world.scale(m_scale);
	m_world.translate(m_trans);

	// 3D Volume Texture
	loadDataToTextures();

	// Update transfer functions
	emit requireTransferFunction();
}

//#define EXPORT_FBO_IMG

void VolumeWidget::resizeGL(int w, int h)
{
	// Update projection matrices
	double aspect = GLfloat(w) / h;
	m_proj.setToIdentity();
	m_otho.setToIdentity();
	m_otho.ortho(-aspect * 2, aspect * 2, -2, 2, -100.0, 100.0);
	m_proj.perspective(45.f, aspect, 0.01f, 100.f);

	// Update FBO size
	m_fbo.reset(new QOpenGLFramebufferObject(w, h, QOpenGLFramebufferObject::Depth, GL_TEXTURE_RECTANGLE_NV, GL_RGBA32F_ARB));
	m_fbo->addColorAttachment(w, h);

	// Update texture size in ray casting vbo
	static QVector<QVector2D> rayCastingVB = { 
		{0.0f,0.0f},
		{0.0,static_cast<float>(h)},
		{ static_cast<float>(w),static_cast<float>(h)},
		{static_cast<float>(w),0.0}
	};
	m_rayCastingTextureVBO.bind();
	m_rayCastingTextureVBO.write(0, rayCastingVB.constData(), rayCastingVB.count() * 2 * sizeof(GLfloat));
	m_rayCastingTextureVBO.release();
}
void VolumeWidget::paintGL()
{
	m_fbo->bind();
	m_positionShader->load(this);
	QOpenGLVertexArrayObject::Binder binder1(&m_positionVAO);
	// Draw Front to fbo 
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
	// Draw Back to fbo 
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_GREATER);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
	m_positionShader->release();
	m_fbo->release();

	// Ray casting in shader
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glEnable(GL_TEXTURE_3D);
	m_currentShader->load(this);
	QOpenGLVertexArrayObject::Binder binder2(&m_rayCastingTextureVAO);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisable(GL_TEXTURE_RECTANGLE_NV);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_BLEND);
	m_currentShader->release();

	//glClear(GL_DEPTH_BUFFER_BIT);
	//QOpenGLVertexArrayObject::Binder binder3(&m_cubeVAO);
	//m_cubeShader->bind();
	//m_cubeShader->setUniformValue("projMatrix",m_proj);
	//m_cubeShader->setUniformValue("mvMatrix",m_camera.view()*m_world);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	//m_cubeShader->release();

}

void VolumeWidget::mousePressEvent(QMouseEvent* event)
{
	m_lastPos = event->pos();
	update();
}
void VolumeWidget::mouseMoveEvent(QMouseEvent* event)
{
	const auto & p = event->pos();
	float dx = p.x() - m_lastPos.x();
	float dy = m_lastPos.y() - p.y();
	if ((event->buttons() &Qt::LeftButton) && (event->buttons() & Qt::RightButton))
	{
		auto direction = m_camera.up()*dy + m_camera.right()*dx;
		m_camera.movement(direction, 0.01);
	}
	else if (event->buttons() & Qt::LeftButton)
	{
		m_camera.rotation(dx, dy, QVector3D(0, 0, 0));
	}
	else if (event->buttons() == Qt::RightButton)
	{
		auto direction = m_camera.front()*dy;
		m_camera.movement(direction, 0.01);
	}
	m_lastPos = p;
	update();
}


void VolumeWidget::updateTransferFunction(const float * func, bool updated)
{
	makeCurrent();
	if (m_tfTexture.isCreated() == false)
	{
		m_tfTexture.setMagnificationFilter(QOpenGLTexture::Linear);
		m_tfTexture.setMinificationFilter(QOpenGLTexture::Linear);
		m_tfTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
		m_tfTexture.setSize(256);
		m_tfTexture.setFormat(QOpenGLTexture::RGBA16_UNorm);		//Equivalent to GL_RGBA16
		m_tfTexture.allocateStorage();
	}
	m_tfTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, func);	//Equivalent to GL_RGBA and GL_FLOAT
	doneCurrent();
	if (updated)
		update();
}

void VolumeWidget::updateVolumeData()
{
	m_gradCalc.setDataModel(m_dataModel);
	//// update volume side length, position vertex vbo
	const auto z = (float)m_dataModel->topSliceCount()*0.01;
	const auto y = (float)m_dataModel->rightSliceCount()*0.01;
	const auto x = (float)m_dataModel->frontSliceCount()*0.01;
	m_scale = QVector3D(x, y, z);
	m_scale.normalize();
	m_trans = QVector3D(0, 0, 0);
	m_trans -= m_scale / 2;
	m_world.setToIdentity();
	m_world.scale(m_scale);
	m_world.translate(m_trans);

	loadDataToTextures();
}

/**
 * \fn	void VolumeWidget::updateMarkData()
 *
 * \brief	Updates the mark data
 *
 * \author	Ysl
 * \date	2018.07.19
 */

void VolumeWidget::updateMarkData()
{
	return;
}

void VolumeWidget::initializeShaders()
{

}

void VolumeWidget::initiliazeTextures()
{

}

void VolumeWidget::loadDataToTextures()
{
	if (m_gradCalc.hasData() == false)
		return;
	if (m_gradCalc.ready() == false)
		m_gradCalc.calcGradent();		//Time-consuming
	const auto d = m_gradCalc.data();
	const auto z = m_dataModel->topSliceCount();
	const auto y = m_dataModel->rightSliceCount();
	const auto x = m_dataModel->frontSliceCount();
	makeCurrent();
	m_volumeTexture.destroy();
	m_volumeTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_volumeTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_volumeTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_volumeTexture.setSize(x, y, z);
	m_volumeTexture.setFormat(QOpenGLTexture::RGBA8_UNorm);
	m_volumeTexture.allocateStorage();
	m_volumeTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, d);
	doneCurrent();
}

void VolumeWidget::cleanup()
{
	makeCurrent();
	m_cubeVBO.destroy();
	m_cubeVAO.destroy();
	m_positionVAO.destroy();
	m_positionVBO.destroy();
	m_positionEBO.destroy();
	m_rayCastingTextureVAO.destroy();
	m_rayCastingTextureVBO.destroy();

	m_tfTexture.destroy();
	m_volumeTexture.destroy();
	m_fbo->~QOpenGLFramebufferObject();

	doneCurrent();
}
