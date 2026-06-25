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
	MOON::System::JobSystem::OnInit();
	MOON::System::JobSystem::Context ctx;
	std::vector<int>testTable(100);
	//execute
	//for (int i = 0;i < 100;i++) {
	//	//MOON::System::JobSystem;
	//	auto lamda=[i,&testTable](JobDispatchArgs arg) {
	//		testTable[i] = i;
	//		};
	//	MOON::System::JobSystem::Execute(ctx,lamda);
	//}

	//dispatch
	//auto lamda = [ &testTable](JobDispatchArgs arg) {
	//	testTable[arg.jobIndex] = arg.jobIndex;
	//	};
	//MOON::System::JobSystem::Dispatch(ctx,100,10,lamda);
	//MOON::System::JobSystem::Wait(ctx);
	
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
