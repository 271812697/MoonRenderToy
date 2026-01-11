#include "editor/UI/PropertyPanel/Collapsiblegroupboxwidget.h"
#include <QToolButton>
#include <QGridLayout>   // for QGridLayout
#include <QHBoxLayout>   // for QHBoxLayout
#include <QLabel>
namespace MOON {
	class CollapsibleGroupBoxWidget::CollapsibleGroupBoxWidgetInternal {
	public:
		std::unique_ptr<QWidget> createPropertyLayoutWidget(
			QLabel* defaultLabel) {
			auto widget = std::make_unique<QWidget>();
			widget->setObjectName("CompositeContents");

			auto propertyLayout = std::make_unique<QGridLayout>();
			propertyLayout->setObjectName("PropertyWidgetLayout");
			propertyLayout->setAlignment(Qt::AlignTop);
			
			propertyLayout->setHorizontalSpacing(0);
			

			auto layout = propertyLayout.release();
			widget->setLayout(layout);

			// add default label to widget layout, this will also set the correct stretching and spacing
			layout->addWidget(defaultLabel, 0, 0);
			layout->addItem(new QSpacerItem(0, 1, QSizePolicy::Fixed), 0, 1);
			layout->setColumnStretch(0, 1);
			layout->setColumnStretch(1, 0);

			return widget;
		}
		void updateFocusPolicy() {
			mSelf->setFocusPolicy(btnCollapse_->focusPolicy());
			mSelf->setFocusProxy(btnCollapse_);
		}
		CollapsibleGroupBoxWidgetInternal(CollapsibleGroupBoxWidget* self) :mSelf(self),
			defaultLabel_{ new QLabel("No properties available") },
			propertyWidgetGroup_ { createPropertyLayoutWidget(defaultLabel_).release() }
		, propertyWidgetGroupLayout_{ static_cast<QGridLayout*>(propertyWidgetGroup_->layout()) } {
			btnCollapse_=new QToolButton(mSelf);
			btnCollapse_->setCheckable(true);
			btnCollapse_->setChecked(false);
			btnCollapse_->setObjectName("collapseButton");
			btnCollapse_->setFocusPolicy(Qt::StrongFocus);
			
			label_ = new QLabel("set", mSelf);
			updateFocusPolicy();
			QHBoxLayout* heading = new QHBoxLayout();
			heading->setContentsMargins(0, 0, 0, 0);
			heading->addWidget(btnCollapse_);
			heading->addWidget(label_);
			QVBoxLayout* layout = new QVBoxLayout();
			
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(0);
			layout->addLayout(heading);
			layout->addWidget(propertyWidgetGroup_);
			mSelf->setLayout(layout);
		}
		~CollapsibleGroupBoxWidgetInternal() {

		}
		void setCollapsed(bool collapse) {
			mSelf->setUpdatesEnabled(false);
			propertyWidgetGroup_->setVisible(!collapse);
			btnCollapse_->setChecked(collapse);
			mSelf->setUpdatesEnabled(true);
		}
	private:
		QLabel* defaultLabel_;
		friend class CollapsibleGroupBoxWidget;

		CollapsibleGroupBoxWidget* mSelf = nullptr;
		QToolButton* btnCollapse_;
		QLabel* label_;
		QWidget* propertyWidgetGroup_;
		QGridLayout* propertyWidgetGroupLayout_;
		std::vector<Property*> properties_;
		std::vector<QWidget*> propertyWidgets_;
	};
	CollapsibleGroupBoxWidget::CollapsibleGroupBoxWidget(const QString& name,QWidget* parent):QWidget(parent),mInternal(new CollapsibleGroupBoxWidgetInternal(this))
	{
		mInternal->label_->setText(name);
		connect(mInternal->btnCollapse_, &QToolButton::toggled, this, &CollapsibleGroupBoxWidget::setCollapsed);
	}
	CollapsibleGroupBoxWidget::~CollapsibleGroupBoxWidget()
	{
		delete mInternal;
	}
	void CollapsibleGroupBoxWidget::addProperty(Property* tmpProperty)
	{
		insertProperty(tmpProperty, mInternal->properties_.size());
	}
	void CollapsibleGroupBoxWidget::insertProperty(Property* prop, size_t index)
	{
		setUpdatesEnabled(false);
		mInternal->propertyWidgetGroupLayout_->setEnabled(false);


		const size_t insertIndex = std::min(index, mInternal->properties_.size());
		const bool insertAtEnd = (insertIndex == mInternal->properties_.size());

		auto insertPoint = mInternal->properties_.begin() + insertIndex;
		auto widgetInsertPoint = mInternal->propertyWidgets_.begin() + insertIndex;

		mInternal->properties_.insert(insertPoint, prop);
		

		//auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
		if (auto propertyWidget =prop->createEditorWidget(this)) {
			mInternal->propertyWidgets_.insert(widgetInsertPoint, propertyWidget);

			insertPropertyWidget(propertyWidget, insertAtEnd);
			
			//RenderContext::getPtr()->activateDefaultRenderContext();

			// need to re-set tab order for all following widgets to ensure tab order is correct
			// (see http://doc.qt.io/qt-5/qwidget.html#setTabOrder)
			for (auto wit = mInternal->propertyWidgets_.begin() + 1; wit != mInternal->propertyWidgets_.end(); ++wit) {
				setTabOrder(*(wit - 1), *wit);
			}
		}
		else {
			//log::warn("Could not find a widget for property: {}", prop->getClassIdentifier());
			// insert empty element to keep property widget vector in sync with property vector
			mInternal->propertyWidgets_.insert(widgetInsertPoint, nullptr);
		}
		mInternal->propertyWidgetGroupLayout_->setEnabled(true);
		setUpdatesEnabled(true);
	}
	void CollapsibleGroupBoxWidget::insertPropertyWidget(QWidget* propertyWidget, bool insertAtEnd)
	{
		auto addPropertyWidget = [&](QGridLayout* layout, int row, QWidget* widget) {
			//if (auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(widget)) {
			//	collapsibleWidget->setNestedDepth(this->getNestedDepth() + 1);
				// make the collapsible widget go all the way to the right border
				//layout->addWidget(widget, row, 0, 1, 2);
			//}
			//else {  // not a collapsible widget
				//widget->setNestedDepth(this->getNestedDepth());
				// property widget should only be added to the left column of the layout
				layout->addWidget(widget, row, 0);
			//}

			//if (isChildRemovable()) {
				//addButtonLayout(layout, row, widget->getProperty());
			//}

			//widget->setParentPropertyWidget(this);
			//widget->initState();
			};
		if (insertAtEnd) {
			// append property widget
			addPropertyWidget(mInternal->propertyWidgetGroupLayout_, mInternal->propertyWidgetGroupLayout_->rowCount(),
				propertyWidget);
		}
		else {
	
		}
	}
	QSize CollapsibleGroupBoxWidget::sizeHint() const
	{
		QSize size = layout()->sizeHint();
		const auto em = fontMetrics().boundingRect('M').width();

		size.setWidth(std::max(static_cast<int>(20 * em), size.width()));
		return size;
	}
	QSize CollapsibleGroupBoxWidget::minimumSizeHint() const
	{
		QSize size = layout()->sizeHint();
		QSize minSize = layout()->minimumSize();
		const auto em = fontMetrics().boundingRect('M').width();
		size.setWidth(
			std::max(static_cast<int>(20 * em), minSize.width()));
		return size;
	}
	void CollapsibleGroupBoxWidget::setCollapsed(bool v) {
		mInternal->setCollapsed(v);
	}
}