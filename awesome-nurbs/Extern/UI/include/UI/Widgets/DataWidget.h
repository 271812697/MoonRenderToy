#pragma once
#include "UI/Widgets/AWidget.h"
namespace UI::Widgets
{

	template <typename T>
	class DataWidget : public AWidget
	{
	public:

		DataWidget(T& p_dataHolder) : m_data(p_dataHolder) {};

	
		virtual void Draw() override;

		void NotifyChange();

	private:
		T& m_data;
	};

	template<typename T>
	inline void DataWidget<T>::Draw()
	{
		if (enabled)
		{
			TRY_GATHER(T, m_data);
			AWidget::Draw();
			TRY_PROVIDE(T, m_data);
		}
	}

	template<typename T>
	inline void DataWidget<T>::NotifyChange()
	{
		TRY_NOTIFY_CHANGE(T);
	}
}
