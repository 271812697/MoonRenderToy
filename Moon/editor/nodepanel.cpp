#include <QtNodes/ConnectionStyle>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>
#include "QMenuBar"
#include "nodepanel.h"
#include "nodes/ReadMeshModel.h"
using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::DataFlowGraphModel;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
{
	auto ret = std::make_shared<NodeDelegateModelRegistry>();
	ret->registerModel<ReadMeshModel>("ReadMesh");
	return ret;
}
static void setNodeStyle()
{
	QtNodes::ConnectionStyle::setConnectionStyle(
		R"(
  {
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": "black",
      "SelectedColor": "gray",
      "SelectedHaloColor": "deepskyblue",
      "HoveredColor": "deepskyblue",

      "LineWidth": 3.0,
      "ConstructionLineWidth": 2.0,
      "PointDiameter": 10.0,

      "UseDataDefinedColors": true
    }
  }
  )");
}
namespace MOON {

	Nodepanel::Nodepanel(QWidget* parent) : QWidget(parent)
	{
		auto menuBar = new QMenuBar(this);
		QMenu* menu = menuBar->addMenu("File");
		auto saveAction = menu->addAction("Save Scene");
		saveAction->setShortcut(QKeySequence::Save);
		auto loadAction = menu->addAction("Load Scene");
		loadAction->setShortcut(QKeySequence::Open);
		QVBoxLayout* l = new QVBoxLayout(this);


		setNodeStyle();

		std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
		dataFlowGraphModel = std::make_unique<DataFlowGraphModel>(registry);
		auto scene = new DataFlowGraphicsScene(*dataFlowGraphModel.get(), this);
		auto view = new GraphicsView(scene);
		l->addWidget(menuBar);
		l->addWidget(view);
		l->setContentsMargins(0, 0, 0, 0);
		l->setSpacing(0);
	}

}