#pragma once
#include "Maths/FVector3.h"
#include "Maths/FVector2.h"
namespace Rendering::Geometry
{
	/**
	* Data structure that defines the geometry of a vertex
	*/
	struct Vertex
	{
		float position[3];
		float texCoords[2];
		float normals[3];
		float tangent[3];
		float bitangent[3];
	};
	struct VertexBVH
	{
		Maths::FVector3 position;
		Maths::FVector2 texCoords;
		Maths::FVector3 normals;
	};
}