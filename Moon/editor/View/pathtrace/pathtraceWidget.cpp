#include <QMouseEvent>
#include "pathtraceWidget.h"
#include "glloader.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Scene.h"
#include "pathtrace/Camera.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "OvCore/Global/ServiceLocator.h"
namespace MOON {
	static float viewW;
	static float viewH;
	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	PathTraceWidget::PathTraceWidget(QWidget* parent) :
		QOpenGLWidget(parent)
	{
		//设置可以捕获鼠标移动消息
		this->setMouseTracking(true);
		//this->grabKeyboard();
		//反锯齿
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
		setFocusPolicy(Qt::StrongFocus);  // 允许通过点击或Tab键获取焦点
		setFocus();                      // 主动获取焦点（可选）
		RegService(PathTraceWidget, *this);
	}
	PathTraceWidget::~PathTraceWidget()
	{
		PathTraceRender::instance().Ret();

	}
	void PathTraceWidget::initializeGL()
	{
		// opengl funcs
		bool flag = initializeOpenGLFunctions();
		OpenGLProcAddressHelper::ctx = context();
		//CUSTOM_GL_API::CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		GlLoader::CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);

		//开启计时器
		this->startTimer(0);
		PathTraceRender::instance().GetSceneFiles();
		PathTraceRender::instance().GetEnvMaps();
		PathTraceRender::instance().LoadDefaultScene();
		if (!PathTraceRender::instance().InitRenderer()) {
			std::cout << "error" << std::endl;
		}
		initFlag = true;

		auto& tree = GetService(TreeViewPanel);
		connect(this, &PathTraceWidget::sceneChange, &tree, &TreeViewPanel::updateTreeViewPathRoot);
		emit sceneChange();
	}
	void  PathTraceWidget::timerEvent(QTimerEvent* e)
	{
		this->update();
	}
	void  PathTraceWidget::paintGL()
	{
		PathTraceRender::instance().Update();
		PathTraceRender::instance().Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		PathTraceRender::instance().Present();
	}
	void  PathTraceWidget::leaveEvent(QEvent* event)
	{
	}
	void  PathTraceWidget::resizeEvent(QResizeEvent* event)
	{
		QOpenGLWidget::resizeEvent(event);
		viewW = event->size().width();
		viewH = event->size().height();
		if (initFlag && viewH > 0 && viewH > 0) {
			PathTrace::PathTraceRender::instance().Resize(viewW, viewH);
		}

	}
	void  PathTraceWidget::mousePressEvent(QMouseEvent* e)
	{
	}

	void  PathTraceWidget::mouseMoveEvent(QMouseEvent* event)
	{
	}
	void  PathTraceWidget::mouseReleaseEvent(QMouseEvent* e)
	{
	}
	void  PathTraceWidget::wheelEvent(QWheelEvent* event)
	{
	}
	void  PathTraceWidget::keyPressEvent(QKeyEvent* event)
	{
	}
	void  PathTraceWidget::keyReleaseEvent(QKeyEvent* event)
	{
	}

	bool PathTraceWidget::event(QEvent* e)
	{
		const QEvent::Type t = e->type();
		if (t == QEvent::Resize) {
			QResizeEvent* event = static_cast<QResizeEvent*>(e);
			if (initFlag && event->size().width() > 0 && event->size().height() > 0) {
				PathTraceRender::instance().ReceiveEvent(e);
			}
		}
		else
		{
			PathTraceRender::instance().ReceiveEvent(e);
		}
		return QOpenGLWidget::event(e);
	}
	void PathTraceWidget::onUpdateEntityTreeView()
	{
		emit sceneChange();
	}
	void PathTraceWidget::onSceneChange(const QString& path) {
		PathTraceRender::instance().onSwitchScene(path.toStdString());
	}
}