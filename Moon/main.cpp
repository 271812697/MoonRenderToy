#include "resource/DarkStyle.h"
#include "core/log.h"
#include "GeometryInit.h"
#include "core/JobSystem.h"
#include <QApplication>
#include <QFontDatabase>
#include <editor/editor.h>

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
	MOON::Editor editor;
	editor.setWindowTitle("MOON");
	editor.resize(1920, 1080);
	editor.show();
	int res = QApplication::exec();
	MOON::System::JobSystem::Release();
	MOON::Log::Shutdown();
	return 0;
}
