#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
#include "Widgets/PropertyComponent.h"

namespace MOON {
    class ParamComponent :public PropertyComponent
    {
    public:
        ParamComponent(ParamTaskDialog* paramWidget,const QString& groupName):owner(paramWidget), paramGroupName(groupName) {
           
        }
        virtual ~ParamComponent() {
        }
       
        virtual QVariant getPropertyValue(const QString& propertyName)override {
            return owner->getParamValue(paramGroupName+":"+propertyName);
        }
        virtual void setPropertyValue(const QString& propertyName, const QVariant& value)override {
            owner->setParamValue(paramGroupName + ":" + propertyName,value);
        }
        virtual QString getComponentName()override {
            return paramGroupName;
        }
    private:
        ParamTaskDialog* owner = nullptr;
        QString paramGroupName = "Group";
    };
    ParamTaskDialog::ParamTaskDialog(QWidget* parent)
        : BaseTaskDialog(parent)
    {
        mainLayout()->setContentsMargins(0, 0, 0, 0);
    }

    ParamTaskDialog::~ParamTaskDialog()
    {
        for (int i = 0;i < m_comps.size();i++) {
            delete m_comps[i].second;
        }
    }

    void ParamTaskDialog::addParam(WidgetProperty* param)
    {
        QString groupName =param->getOwnerName();
        auto it=groupToIndex.find(groupName);
        if (it != groupToIndex.end()) {
            int index = it->second;
            m_comps[index].second->addProperty(param);
        }
        else
        {
            groupToIndex[groupName] = m_comps.size();
            auto collpase = new CollapsibleGroupBoxWidget(groupName,this);
            auto p = new ParamComponent(this,groupName);
            m_comps.push_back({ collpase ,p });
        }
    }

    PropertyComponent* ParamTaskDialog::addGroupParam(const QString& groupName)
    {
        auto it = groupToIndex.find(groupName);
        if (it == groupToIndex.end()) {
            groupToIndex[groupName] = m_comps.size();
            auto collpase = new CollapsibleGroupBoxWidget(groupName, this);
            auto p = new ParamComponent(this, groupName);
            m_comps.push_back({ collpase ,p });
            return p;
        }
        return nullptr;
    }

    PropertyComponent* ParamTaskDialog::getGroupComponent(const QString& groupName)
    {
        auto it = groupToIndex.find(groupName);
        if (it != groupToIndex.end()) {
            return m_comps[it->second].second;
        }
        return nullptr;
    }

    void ParamTaskDialog::buildUi()
    {
        for (int i = 0;i < m_comps.size();i++) {
            auto p = m_comps[i].second;
            auto collpase = m_comps[i].first;
            mainLayout()->addWidget(collpase);
            for (auto u : p->getProperties()) {
                collpase->addProperty(u);
            }
        }
        mainLayout()->addStretch();
    }
    void ParamTaskDialog::clickOk()
    {
    }
    void ParamTaskDialog::clickApply()
    {
    }
    void ParamTaskDialog::clickCancel()
    {
    }
    QVariant ParamTaskDialog::getParamValue(const QString& propertyName)
    {
        return QVariant();
    }
    void ParamTaskDialog::setParamValue(const QString& propertyName, const QVariant& value)
    {
    }
}