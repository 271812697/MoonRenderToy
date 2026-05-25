#pragma once
#include <stdexcept>

namespace MOON {

	class InvalidInputException : public std::invalid_argument
	{
		using std::invalid_argument::invalid_argument;
	};

	class SolverException : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	class AllocationException : public std::length_error
	{
		using std::length_error::length_error;
	};

	class TopologyException : public std::logic_error
	{
		using std::logic_error::logic_error;
	};

	class IOException : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	class GLException : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
}
