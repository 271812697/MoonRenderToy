#include <QMouseEvent>
#include "viewerwindow.h"
#include "nodedatas/SurfaceMeshData.h"
#include "glloader.h"
#include "renderer/grid_renderer.h"
#include "renderer/Guizmo.h"
#include "core/read_mesh.h"
#include "algorithm/mesh_triangulate.h"
#include "test/TestInstance.h"
#include "test/CommandStream.h"
#include "Guizmo/RenderWindowInteractor.h"
#include "Guizmo/ExecuteCommand.h"
#include "Guizmo/BoxWidget2.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Renderer.h"
#include "pathtrace/Scene.h"
#include "pathtrace/Camera.h"


static const char* AsciiToKeySymTable[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, "Tab", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, "space", "exclam", "quotedbl", "numbersign", "dollar",
  "percent", "ampersand", "quoteright", "parenleft", "parenright", "asterisk", "plus", "comma",
  "minus", "period", "slash", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "colon",
  "semicolon", "less", "equal", "greater", "question", "at", "A", "B", "C", "D", "E", "F", "G", "H",
  "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
  "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", "quoteleft", "a", "b",
  "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u",
  "v", "w", "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "Delete", nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
const char* ascii_to_key_sym(int i)
{
	if (i >= 0)
	{
		return AsciiToKeySymTable[i];
	}
	return nullptr;
}
const char* qt_key_to_key_sym(Qt::Key i, Qt::KeyboardModifiers modifiers)
{


	const char* ret = nullptr;
	switch (i)
	{
		// Cancel
	case Qt::Key_Backspace: ret = "BackSpace"; break;
	case Qt::Key_Tab: ret = "Tab"; break;
	case Qt::Key_Backtab: ret = "Tab"; break;
	case Qt::Key_Clear: ret = "Clear"; break;
	case Qt::Key_Return: ret = "Return"; break;
	case Qt::Key_Enter: ret = "Return"; break;
	case Qt::Key_Shift: ret = "Shift_L"; break;
	case Qt::Key_Control: ret = "Control_L"; break;
	case Qt::Key_Alt: ret = "Alt_L"; break;
	case Qt::Key_Pause: ret = "Pause"; break;
	case Qt::Key_CapsLock: ret = "Caps_Lock"; break;
	case Qt::Key_Escape: ret = "Escape"; break;
	case Qt::Key_Space: ret = "space"; break;
	case Qt::Key_PageUp: ret = "Prior"; break;
	case Qt::Key_PageDown: ret = "Next"; break;
	case Qt::Key_End: ret = "End"; break;
	case Qt::Key_Home: ret = "Home"; break;
	case Qt::Key_Left: ret = "Left"; break;
	case Qt::Key_Up: ret = "Up"; break;
	case Qt::Key_Right: ret = "Right"; break;
	case Qt::Key_Down: ret = "Down"; break;
	case Qt::Key_Select: ret = "Select"; break;
	case Qt::Key_Execute: ret = "Execute"; break;
	case Qt::Key_SysReq: ret = "Snapshot"; break;
	case Qt::Key_Insert: ret = "Insert"; break;
	case Qt::Key_Delete: ret = "Delete"; break;
	case Qt::Key_Help: ret = "Help"; break;
	case Qt::Key_0: ret = (modifiers & Qt::KeypadModifier) ? ("KP_0") : ("0"); break;
	case Qt::Key_1: ret = (modifiers & Qt::KeypadModifier) ? ("KP_1") : ("1"); break;
	case Qt::Key_2: ret = (modifiers & Qt::KeypadModifier) ? ("KP_2") : ("2"); break;
	case Qt::Key_3: ret = (modifiers & Qt::KeypadModifier) ? ("KP_3") : ("3"); break;
	case Qt::Key_4: ret = (modifiers & Qt::KeypadModifier) ? ("KP_4") : ("4"); break;
	case Qt::Key_5: ret = (modifiers & Qt::KeypadModifier) ? ("KP_5") : ("5"); break;
	case Qt::Key_6: ret = (modifiers & Qt::KeypadModifier) ? ("KP_6") : ("6"); break;
	case Qt::Key_7: ret = (modifiers & Qt::KeypadModifier) ? ("KP_7") : ("7"); break;
	case Qt::Key_8: ret = (modifiers & Qt::KeypadModifier) ? ("KP_8") : ("8"); break;
	case Qt::Key_9: ret = (modifiers & Qt::KeypadModifier) ? ("KP_9") : ("9"); break;
	case Qt::Key_A: ret = "a"; break;
	case Qt::Key_B: ret = "b"; break;
	case Qt::Key_C: ret = "c"; break;
	case Qt::Key_D: ret = "d"; break;
	case Qt::Key_E: ret = "e"; break;
	case Qt::Key_F: ret = "f"; break;
	case Qt::Key_G: ret = "g"; break;
	case Qt::Key_H: ret = "h"; break;
	case Qt::Key_I: ret = "i"; break;
	case Qt::Key_J: ret = "h"; break;
	case Qt::Key_K: ret = "k"; break;
	case Qt::Key_L: ret = "l"; break;
	case Qt::Key_M: ret = "m"; break;
	case Qt::Key_N: ret = "n"; break;
	case Qt::Key_O: ret = "o"; break;
	case Qt::Key_P: ret = "p"; break;
	case Qt::Key_Q: ret = "q"; break;
	case Qt::Key_R: ret = "r"; break;
	case Qt::Key_S: ret = "s"; break;
	case Qt::Key_T: ret = "t"; break;
	case Qt::Key_U: ret = "u"; break;
	case Qt::Key_V: ret = "v"; break;
	case Qt::Key_W: ret = "w"; break;
	case Qt::Key_X: ret = "x"; break;
	case Qt::Key_Y: ret = "y"; break;
	case Qt::Key_Z: ret = "z"; break;
	case Qt::Key_Asterisk: ret = "asterisk"; break;
	case Qt::Key_Plus: ret = "plus"; break;
	case Qt::Key_Bar: ret = "bar"; break;
	case Qt::Key_Minus: ret = "minus"; break;
	case Qt::Key_Period: ret = "period"; break;
	case Qt::Key_Slash: ret = "slash"; break;
	case Qt::Key_F1: ret = "F1"; break;
	case Qt::Key_F2: ret = "F2"; break;
	case Qt::Key_F3: ret = "F3"; break;
	case Qt::Key_F4: ret = "F4"; break;
	case Qt::Key_F5: ret = "F5"; break;
	case Qt::Key_F6: ret = "F6"; break;
	case Qt::Key_F7: ret = "F7"; break;
	case Qt::Key_F8: ret = "F8"; break;
	case Qt::Key_F9: ret = "F9"; break;
	case Qt::Key_F10: ret = "F10"; break;
	case Qt::Key_F11: ret = "F11"; break;
	case Qt::Key_F12: ret = "F12"; break;
	case Qt::Key_F13: ret = "F13"; break;
	case Qt::Key_F14: ret = "F14"; break;
	case Qt::Key_F15: ret = "F15"; break;
	case Qt::Key_F16: ret = "F16"; break;
	case Qt::Key_F17: ret = "F17"; break;
	case Qt::Key_F18: ret = "F18"; break;
	case Qt::Key_F19: ret = "F19"; break;
	case Qt::Key_F20: ret = "F20"; break;
	case Qt::Key_F21: ret = "F21"; break;
	case Qt::Key_F22: ret = "F22"; break;
	case Qt::Key_F23: ret = "F23"; break;
	case Qt::Key_F24: ret = "F24"; break;
	case Qt::Key_NumLock: ret = "Num_Lock"; break;
	case Qt::Key_ScrollLock: ret = "Scroll_Lock"; break;

	default:
		break;
	}
	return ret;
}
namespace MOON {
	static float viewW;
	static float viewH;
	Eigen::Matrix4f LookAt(const  Eigen::Vector3<float>& _from, const  Eigen::Vector3<float>& _to, const  Eigen::Vector3<float>& _up = Eigen::Vector3<float>(0.0f, 1.0f, 0.0f));
	static GridRenderer* grid_render;
	static Guizmo* guizmoRender;
	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	ViewerWindow* viewer_instance = nullptr;
	ViewerWindow::ViewerWindow(QWidget* parent) :
		QOpenGLWidget(parent)
	{

		if (viewer_instance == nullptr) {
			viewer_instance = this;
			//设置可以捕获鼠标移动消息
			this->setMouseTracking(true);
			this->grabKeyboard();
			//反锯齿
			QSurfaceFormat format;
			format.setSamples(4);
			this->setFormat(format);
			if (windowInteractor == nullptr) {
				windowInteractor = vtkRenderWindowInteractor::New();
				windowInteractor->Enable();
				boxWidget = vtkBoxWidget2::New();
				boxWidget->SetInteractor(windowInteractor);
				boxWidget->SetEnabled(1);
			}
		}
	}

	ViewerWindow::~ViewerWindow()
	{
		windowInteractor->Terminate();
	}

	void ViewerWindow::initializeGL()
	{
		// opengl funcs
		bool flag = initializeOpenGLFunctions();
		OpenGLProcAddressHelper::ctx = context();
		//CUSTOM_GL_API::CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		TEST::TestInstance::Instance().getCommandStream()->test(8);
		TEST::TestInstance::Instance().getCommandStream()->queueCommand([]() {
			std::cout << "say hello" << std::endl;
			});

		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.init();
		//viewer.post_resize(800, 900);
		grid_render = new GridRenderer();
		guizmoRender = new Guizmo();
		//viewer.core().background_color = Eigen::Vector4f(1.0, 0.0, 0.0, 1.0);

		//开启计时器
		this->startTimer(16);


		PathTrace::GetSceneFiles();
		PathTrace::GetEnvMaps();
		PathTrace::LoadScene(PathTrace::sceneFiles[PathTrace::sampleSceneIdx]);
		if (!PathTrace::InitRenderer()) {
			std::cout << "error" << std::endl;
		}
		initFlag = true;
	}

	void ViewerWindow::timerEvent(QTimerEvent* e)
	{
		this->update();
	}



	void ViewerWindow::paintGL()
	{
		TEST::TestInstance::Instance().flush();
		TEST::TestInstance::Instance().execute();
		PathTrace::GetRenderer()->Update(0.016);
		PathTrace::GetRenderer()->Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		PathTrace::GetRenderer()->Present();
		//PathTrace::GetRenderer()->SaveFrame();
	}

	bool ViewerWindow::event(QEvent* evt)
	{
		processEventByWindowInteractor(evt);
		return QOpenGLWidget::event(evt);
	}

	void ViewerWindow::leaveEvent(QEvent* event)
	{
	}

	void ViewerWindow::resizeEvent(QResizeEvent* event)
	{

		QOpenGLWidget::resizeEvent(event);
		viewer.post_resize(event->size().width() * 1.5, event->size().height() * 1.5);

		viewW = event->size().width();
		viewH = event->size().height();
		if (initFlag) {
			PathTrace::Resize(viewW, viewH);
		}

	}

	void ViewerWindow::mousePressEvent(QMouseEvent* e)
	{
		if (blockMouseMessage) {
			return;
		}
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		auto x = e2->x();
		auto y = e2->y();
#else
		auto x = e2->position().x();
		auto y = e2->position().y();
#endif
		Qt::MouseButton mb = e->button();
		if (mb == Qt::MouseButton::LeftButton) {
			viewer.mouse_down(Viewer::MouseButton::Left);

			PathTrace::CameraController::Instance().mouseLeftPress(x, y);
		}

		else if (mb == Qt::MouseButton::MiddleButton) {
			viewer.mouse_down(Viewer::MouseButton::Middle);
			PathTrace::CameraController::Instance().mouseMiddlePress();
		}

		else if (mb == Qt::MouseButton::RightButton)
		{
			PathTrace::CameraController::Instance().mouseRightPress();
			viewer.mouse_down(Viewer::MouseButton::Right);
		}

	}

	void ViewerWindow::mouseMoveEvent(QMouseEvent* event)
	{

		auto pos = event->localPos();
		//cursorX = pos.x() * 1.5;
		//cursorY = pos.y() * 1.5;

		PathTrace::CameraController::Instance().mouseMove(pos.x(), pos.y());
		viewer.mouse_move(pos.x() * 1.5, pos.y() * 1.5);
	}

	void ViewerWindow::mouseReleaseEvent(QMouseEvent* event)
	{
		if (blockMouseMessage) {
			return;
		}
		Qt::MouseButton mb = event->button();
		if (mb == Qt::MouseButton::LeftButton)
			viewer.mouse_up(Viewer::MouseButton::Left);
		else if (mb == Qt::MouseButton::MiddleButton) {
			viewer.mouse_up(Viewer::MouseButton::Middle);
			PathTrace::CameraController::Instance().mouseMiddleRelease();
		}

		else if (mb == Qt::MouseButton::RightButton)
		{
			viewer.mouse_up(Viewer::MouseButton::Right);
			PathTrace::CameraController::Instance().mouseRightRelease();
		}

	}

	void ViewerWindow::wheelEvent(QWheelEvent* event)
	{
		viewer.mouse_scroll(event->angleDelta().y());
		PathTrace::CameraController::Instance().wheelMouseWheel(event->angleDelta().y());
	}
	void ViewerWindow::keyPressEvent(QKeyEvent* event)
	{
		blockMouseMessage = true;
		if (event->key() == Qt::Key::Key_Space) {

		}
	}
	void ViewerWindow::keyReleaseEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key::Key_Space) {
			blockMouseMessage = false;
		}
	}
	bool ViewerWindow::processEventByWindowInteractor(QEvent* e)
	{
		if (windowInteractor == nullptr) {
			windowInteractor = vtkRenderWindowInteractor::New();
			windowInteractor->Enable();
			boxWidget = vtkBoxWidget2::New();
			boxWidget->SetInteractor(windowInteractor);
			boxWidget->SetEnabled(1);
		}
		vtkRenderWindowInteractor* iren = windowInteractor;
		if (iren == nullptr || e == nullptr)
			return false;

		const QEvent::Type t = e->type();

		if (!iren->GetEnabled())
			return false;

		if (t == QEvent::MouseButtonPress || t == QEvent::MouseButtonRelease ||
			t == QEvent::MouseButtonDblClick || t == QEvent::MouseMove || t == QEvent::HoverMove)
		{
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			auto x = e2->x();
			auto y = e2->y();
#else
			auto x = e2->position().x();
			auto y = e2->position().y();
#endif
			if (t == QEvent::MouseMove || t == QEvent::HoverMove)
			{
				iren->InvokeEvent(ExecuteCommand::MouseMoveEvent, e2);
			}

			else if (t == QEvent::MouseButtonPress)
			{
				switch (e2->button())
				{
				case Qt::LeftButton:
					iren->InvokeEvent(ExecuteCommand::LeftButtonPressEvent, e2);
					break;

				case Qt::MiddleButton:
					iren->InvokeEvent(ExecuteCommand::MiddleButtonPressEvent, e2);
					break;

				case Qt::RightButton:
					iren->InvokeEvent(ExecuteCommand::RightButtonPressEvent, e2);
					break;

				default:
					break;
				}
			}
			else if (t == QEvent::MouseButtonDblClick)
			{
				switch (e2->button())
				{
				case Qt::LeftButton:
					//iren->InvokeEvent(ExecuteCommand::LeftButtonDoubleClickEvent, e2);
					break;

				case Qt::MiddleButton:
					//iren->InvokeEvent(ExecuteCommand::MiddleButtonDoubleClickEvent, e2);
					break;

				case Qt::RightButton:
					//iren->InvokeEvent(ExecuteCommand::RightButtonDoubleClickEvent, e2);
					break;

				default:
					break;
				}
			}
			else if (t == QEvent::MouseButtonRelease)
			{
				switch (e2->button())
				{
				case Qt::LeftButton:
					iren->InvokeEvent(ExecuteCommand::LeftButtonReleaseEvent, e2);
					break;

				case Qt::MiddleButton:
					iren->InvokeEvent(ExecuteCommand::MiddleButtonReleaseEvent, e2);
					break;

				case Qt::RightButton:
					iren->InvokeEvent(ExecuteCommand::RightButtonReleaseEvent, e2);
					break;

				default:
					break;
				}
			}
			return true;
		}


		if (t == QEvent::Enter)
		{
			iren->InvokeEvent(ExecuteCommand::EnterEvent, e);
			return true;
		}

		if (t == QEvent::Leave)
		{
			iren->InvokeEvent(ExecuteCommand::LeaveEvent, e);
			return true;
		}

		if (t == QEvent::KeyPress || t == QEvent::KeyRelease)
		{
			QKeyEvent* e2 = static_cast<QKeyEvent*>(e);

			// get key and keysym information
			int ascii_key = e2->text().length() ? e2->text().unicode()->toLatin1() : 0;
			const char* keysym = ascii_to_key_sym(ascii_key);
			if (!keysym || e2->modifiers() == Qt::KeypadModifier)
			{
				// get virtual keys
				keysym = qt_key_to_key_sym(static_cast<Qt::Key>(e2->key()), e2->modifiers());
			}

			if (!keysym)
			{
				keysym = "None";
			}

			// give interactor event information
			iren->SetKeyEventInformation((e2->modifiers() & Qt::ControlModifier),
				(e2->modifiers() & Qt::ShiftModifier), ascii_key, e2->count(), keysym);
			iren->SetAltKey((e2->modifiers() & Qt::AltModifier) > 0 ? 1 : 0);

			if (t == QEvent::KeyPress)
			{

				iren->InvokeEvent(ExecuteCommand::KeyPressEvent, e2);
				if (ascii_key)
				{
					iren->InvokeEvent(ExecuteCommand::CharEvent, e2);
				}
			}
			else
			{
				iren->InvokeEvent(ExecuteCommand::KeyReleaseEvent, e2);
			}
			return true;
		}
		return false;
	}
	void ViewerWindow::viewnode(const std::shared_ptr<NodeData>& node) {
		auto mesh_data = std::dynamic_pointer_cast<SurfaceMeshData>(node);

		bool is_triangle = mesh_data->mesh()->is_triangle_mesh();
		SurfaceMesh* present_mesh = mesh_data->mesh().get();
		if (!is_triangle) {
			present_mesh = new SurfaceMesh(*mesh_data->mesh());
			triangulate(*present_mesh);
		}
		Eigen::MatrixXd SV;
		auto& pos = present_mesh->positions();
		SV.resize(pos.size(), 3);
		for (int i = 0; i < pos.size(); i++) {
			SV.row(i) << pos[i][0], pos[i][1], pos[i][2];
		}
		Eigen::MatrixXi SF;
		SF.resize(present_mesh->faces_size(), 3);
		const auto& faces = present_mesh->faces();
		int j = 0, x, y, z;
		for (auto f : faces) {
			auto it = present_mesh->vertices(f).begin();
			x = (*it++).idx();
			y = (*it++).idx();
			z = (*it++).idx();
			SF.row(j++) << x, y, z;
		}
		viewer.data(0).clear();
		viewer.data(0).set_mesh(SV, SF);
	}

}