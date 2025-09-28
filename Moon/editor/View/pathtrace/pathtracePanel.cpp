#include <QMouseEvent>
#include "pathtracePanel.h"
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
	PathTracePanel::PathTracePanel(QWidget* parent) :
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
		COPROVITE(PathTracePanel, *this);
	}
	PathTracePanel::~PathTracePanel()
	{
		PathTraceRender::instance().Ret();

	}
	void PathTracePanel::initializeGL()
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

		auto& tree = OVSERVICE(TreeViewPanel);
		connect(this,&PathTracePanel::sceneChange,&tree,&TreeViewPanel::updateTreeViewPathRoot);
		emit sceneChange();
	}
	void  PathTracePanel::timerEvent(QTimerEvent* e)
	{
		this->update();
	}
	void  PathTracePanel::paintGL()
	{
		PathTraceRender::instance().Update();
		PathTraceRender::instance().Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		PathTraceRender::instance().Present();
	}
	void  PathTracePanel::leaveEvent(QEvent* event)
	{
	}
	void  PathTracePanel::resizeEvent(QResizeEvent* event)
	{
		QOpenGLWidget::resizeEvent(event);
		viewW = event->size().width();
		viewH = event->size().height();
		if (initFlag && viewH > 0 && viewH > 0) {
			PathTrace::PathTraceRender::instance().Resize(viewW, viewH);
		}

	}
	void  PathTracePanel::mousePressEvent(QMouseEvent* e)
	{
	}

	void  PathTracePanel::mouseMoveEvent(QMouseEvent* event)
	{
	}
	void  PathTracePanel::mouseReleaseEvent(QMouseEvent* e)
	{
	}
	void  PathTracePanel::wheelEvent(QWheelEvent* event)
	{
	}
	void  PathTracePanel::keyPressEvent(QKeyEvent* event)
	{
	}
	void  PathTracePanel::keyReleaseEvent(QKeyEvent* event)
	{
	}

	bool PathTracePanel::event(QEvent* e)
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
	void PathTracePanel::onUpdateEntityTreeView()
	{
		emit sceneChange();
	}
	void PathTracePanel::onSceneChange(const QString& path) {
		PathTraceRender::instance().onSwitchScene(path.toStdString());
	}
}