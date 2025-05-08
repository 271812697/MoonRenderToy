#include "ReadMeshModel.h"
#include "nodedatas/StringData.h"
#include "nodedatas/SurfaceMeshData.h"
#include "core/read_mesh.h"
#include "editor/viewerwindow.h"
#include <QtCore/QJsonValue>
#include <QtWidgets/QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
namespace MOON {
	extern ViewerWindow* viewer_instance;
}
ReadMeshModel::ReadMeshModel()
	: _param{ nullptr }, _lineEdit{ nullptr }, _show{ nullptr }
	, _path(std::make_shared<StringData>()), _srf(std::shared_ptr<SurfaceMeshData>(nullptr))
{}

QJsonObject  ReadMeshModel::save() const
{
	QJsonObject modelJson = NodeDelegateModel::save();

	modelJson["path"] = _path->str();

	return modelJson;
}

void  ReadMeshModel::load(QJsonObject const& p)
{
	QJsonValue v = p["path"];

	if (!v.isUndefined()) {
		_path = std::make_shared<StringData>(v.toString());
		if (_lineEdit)
			_lineEdit->setText(_path->str());
	}
}

unsigned int  ReadMeshModel::nPorts(PortType portType) const
{
	unsigned int result = 1;

	switch (portType) {
	case PortType::In:
		result = 0;
		break;

	case PortType::Out:
		result = 1;

	default:
		break;
	}

	return result;
}

void  ReadMeshModel::onPathEdited(QString const& str)
{

	_path = std::make_shared<StringData>(str);
	Q_EMIT dataUpdated(0);

}

NodeDataType  ReadMeshModel::dataType(PortType, PortIndex) const
{
	return SurfaceMeshData().type();
}

std::shared_ptr<NodeData>  ReadMeshModel::outData(PortIndex)
{
	//"D:/Project/C++/NURBSDEMO/Res/camelhead.off"
	//"D:/Project/C++/NURBSDEMO/Res/Texture/cat-5.obj"
	using namespace MOON;
	if (!_srf.get()) {
		std::shared_ptr<SurfaceMesh> mesh = std::make_shared<SurfaceMesh>();

		const std::filesystem::path fp(_path->str().toStdString().c_str());
		if (std::filesystem::exists(fp)) {
			auto ext = fp.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

			if (ext == ".obj")
				read_obj(*mesh, fp);
			else if (ext == ".off")
				read_off(*mesh, fp);
			_srf = std::make_shared<SurfaceMeshData>(mesh);
		}
	}
	return _srf;
}

QWidget* ReadMeshModel::embeddedWidget()
{
	if (!_param) {
		_param = new QWidget();
		QVBoxLayout* layout = new QVBoxLayout(_param);

		if (!_lineEdit) {
			_lineEdit = new QLineEdit();

			connect(_lineEdit, &QLineEdit::textChanged, this, &ReadMeshModel::onPathEdited);
			_lineEdit->setText(_path->str());
			layout->addWidget(_lineEdit);
		}
		if (!_show) {
			_show = new QPushButton();
			//connect(_show, &QPushButton::pressed, this, &ReadMeshModel::onShow);
			//connect(this, &ReadMeshModel::viewnode, MOON::viewer_instance, &MOON::ViewerWindow::viewnode);
			layout->addWidget(_show);

		}
	}

	return _param;
}

void ReadMeshModel::setPath(QString const& path)
{
	_path = std::make_shared<StringData>(path);
}

void ReadMeshModel::onShow()
{
	//_srf.reset();
	Q_EMIT viewnode(outData(0));
}

