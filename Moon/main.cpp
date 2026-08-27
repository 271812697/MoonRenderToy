#include "resource/DarkStyle.h"
#include "core/log.h"
#include "GeometryInit.h"
#include "core/JobSystem.h"
#include <QApplication>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <editor/editor.h>
#include <editor/View/sceneview/viewerwidget.h>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setStyle(new DarkStyle);
	const int font_id = QFontDatabase::addApplicationFont(
		":/darkstyle/Ubuntu-R.ttf");
	QFont font(QFontDatabase::applicationFontFamilies(font_id).at(0));
	font.setPointSize(10);
	MOON::Log::Init();
	CORE_INFO("start ");
	CORE_INFO("Init Geometry Types");
	Part::GeometryTypeInit();
	//test jobsystem
	MOON::System::JobSystem::OnInit(2);
	
	QApplication::setFont(font);

	const bool imguiEditor = QApplication::arguments().contains("--imgui");
	int res = 0;
	if (true) {
		// ImGui editor mode: host the renderer + ImGui editor UI in a plain
		// window, without any Qt widget UI (menus, docks, toolbars).
		MOON::ViewerWidget::SetImGuiEditorMode(true);
		QWidget window;
		window.setWindowTitle("MOON - ImGui Editor");
		auto* layout = new QVBoxLayout(&window);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		layout->addWidget(new MOON::ViewerWidget(&window));
		window.resize(1920, 1080);
		window.show();
		res = QApplication::exec();
	}
	else {
		MOON::Editor editor;
		editor.setWindowTitle("MOON");
		editor.resize(1920, 1080);
		editor.show();
		res = QApplication::exec();
	}

	MOON::System::JobSystem::Release();
	MOON::Log::Shutdown();
	return 0;
}
