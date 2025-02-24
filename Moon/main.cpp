#include <QApplication>
#include <QFontDatabase>
#include <editor/editor.h>
#include "resource/DarkStyle.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setStyle(new DarkStyle);
	const int font_id = QFontDatabase::addApplicationFont(
		":/darkstyle/Ubuntu-R.ttf");
	QFont font(QFontDatabase::applicationFontFamilies(font_id).at(0));
	font.setPointSize(9);
	QApplication::setFont(font);
	MOON::Editor editor;
	editor.setWindowTitle("Editor");
	editor.resize(1440, 768);
	editor.show();
	
	return QApplication::exec();
}
