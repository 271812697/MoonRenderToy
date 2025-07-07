

#pragma once

#include <string>
#include <unordered_map>


#include "UI/Modules/Canvas.h"
#include "UI/Styling/EStyle.h"

namespace UI::Core
{

	class UIManager
	{
	public:

		UIManager(GLFWwindow* p_glfwWindow, Styling::EStyle p_style = Styling::EStyle::IM_DARK_STYLE, const std::string& p_glslVersion = "#version 150");

		~UIManager();

		void ApplyStyle(Styling::EStyle p_style);

		bool LoadFont(const std::string& p_id, const std::string& p_path, float p_fontSize);


		bool UnloadFont(const std::string& p_id);

		bool UseFont(const std::string& p_id);


		void UseDefaultFont();


		void EnableEditorLayoutSave(bool p_value);


		bool IsEditorLayoutSaveEnabled() const;


		void SetEditorLayoutSaveFilename(const std::string& p_filename);


		void SetEditorLayoutAutosaveFrequency(float p_frequency);
		

		float GetEditorLayoutAutosaveFrequency(float p_frequeny);


		void EnableDocking(bool p_value);


        void ResetLayout(const std::string & p_config) const;

		bool IsDockingEnabled() const;

	
		void SetCanvas(Modules::Canvas& p_canvas);


		void RemoveCanvas();


		void Render();

	private:
		void PushCurrentFont();
		void PopCurrentFont();

	private:
		bool m_dockingState;
		Modules::Canvas* m_currentCanvas = nullptr;
		std::unordered_map<std::string, ImFont*> m_fonts;
		std::string m_layoutSaveFilename = "imgui.ini";
	};
}