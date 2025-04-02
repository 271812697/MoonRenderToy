#pragma once
namespace asset {
	class Shader;
}
namespace PathTrace
{

	class Quad
	{
	public:
		Quad();
		void Draw(asset::Shader* shader);

	private:
		unsigned vao;
		unsigned vbo;
	};
}