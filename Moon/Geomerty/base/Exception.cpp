/***************************************************************************
 *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License (LGPL)   *
 *   as published by the Free Software Foundation; either version 2 of     *
 *   the License, or (at your option) any later version.                   *
 *   for detail see the LICENCE text file.                                 *
 *                                                                         *
 *   FreeCAD is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with FreeCAD; if not, write to the Free Software        *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 ***************************************************************************/


#include "Exception.h"

#include <typeinfo>
using namespace Base;


TYPESYSTEM_SOURCE(Base::Exception, Base::BaseClass)


Exception::Exception()
    : _sErrMsg("FreeCAD Exception")
    , _line(0)
    , _isTranslatable(false)
    , _isReported(false)
{}

Exception::Exception(const Exception& inst) = default;

Exception::Exception(Exception&& inst) noexcept = default;

Exception::Exception(const char* sMessage)
    : _sErrMsg(sMessage)
    , _line(0)
    , _isTranslatable(false)
    , _isReported(false)
{}

Exception::Exception(std::string sMessage)
    : _sErrMsg(std::move(sMessage))
    , _line(0)
    , _isTranslatable(false)
    , _isReported(false)
{}

Exception& Exception::operator=(const Exception& inst)
{
    _sErrMsg = inst._sErrMsg;
    _file = inst._file;
    _line = inst._line;
    _function = inst._function;
    _isTranslatable = inst._isTranslatable;
    return *this;
}

Exception& Exception::operator=(Exception&& inst) noexcept
{
    _sErrMsg = std::move(inst._sErrMsg);
    _file = std::move(inst._file);
    _line = inst._line;
    _function = std::move(inst._function);
    _isTranslatable = inst._isTranslatable;
    return *this;
}

const char* Exception::what() const noexcept
{
    return _sErrMsg.c_str();
}

void Exception::ReportException() const
{
    if (!_isReported) {
        const char* msg {};
        if (_sErrMsg.empty()) {
            msg = typeid(*this).name();
        }
        else {
            msg = _sErrMsg.c_str();
        }

        _isReported = true;
    }
}



void Exception::setPyException() const
{

}

// ---------------------------------------------------------

TYPESYSTEM_SOURCE(Base::AbortException, Base::Exception)

AbortException::AbortException(const char* sMessage)
    : Exception(sMessage)
{}

AbortException::AbortException()
{
    _sErrMsg = "Aborted operation";
}

const char* AbortException::what() const noexcept
{
    return Exception::what();
}


// ---------------------------------------------------------


BadFormatError::BadFormatError() = default;

BadFormatError::BadFormatError(const char* sMessage)
    : Exception(sMessage)
{}

BadFormatError::BadFormatError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------


MemoryException::MemoryException()
{
    _sErrMsg = "Not enough memory available";
}

MemoryException::MemoryException(const MemoryException& inst)
#if defined(__GNUC__)
    : std::bad_alloc()
    , Exception(inst)
#else
    : Exception(inst)
#endif
{}

MemoryException::MemoryException(MemoryException&& inst) noexcept
#if defined(__GNUC__)
    : std::bad_alloc()
    , Exception(inst)
#else
    : Exception(inst)
#endif
{}

MemoryException& MemoryException::operator=(const MemoryException& inst)
{
    Exception::operator=(inst);
    return *this;
}

MemoryException& MemoryException::operator=(MemoryException&& inst) noexcept
{
    Exception::operator=(inst);
    return *this;
}

#if defined(__GNUC__)
const char* MemoryException::what() const noexcept
{
    // call what() of Exception, not of std::bad_alloc
    return Exception::what();
}
#endif


// ---------------------------------------------------------

AccessViolation::AccessViolation()
{
    _sErrMsg = "Access violation";
}

AccessViolation::AccessViolation(const char* sMessage)
    : Exception(sMessage)
{}

AccessViolation::AccessViolation(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

AbnormalProgramTermination::AbnormalProgramTermination()
{
    _sErrMsg = "Abnormal program termination";
}

AbnormalProgramTermination::AbnormalProgramTermination(const char* sMessage)
    : Exception(sMessage)
{}

AbnormalProgramTermination::AbnormalProgramTermination(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

UnknownProgramOption::UnknownProgramOption() = default;

UnknownProgramOption::UnknownProgramOption(const char* sMessage)
    : Exception(sMessage)
{}

UnknownProgramOption::UnknownProgramOption(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

ProgramInformation::ProgramInformation() = default;

ProgramInformation::ProgramInformation(const char* sMessage)
    : Exception(sMessage)
{}

ProgramInformation::ProgramInformation(const std::string& sMessage)
    : Exception(sMessage)
{}

// ---------------------------------------------------------

TypeError::TypeError() = default;

TypeError::TypeError(const char* sMessage)
    : Exception(sMessage)
{}

TypeError::TypeError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

ValueError::ValueError() = default;

ValueError::ValueError(const char* sMessage)
    : Exception(sMessage)
{}

ValueError::ValueError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

IndexError::IndexError() = default;

IndexError::IndexError(const char* sMessage)
    : Exception(sMessage)
{}

IndexError::IndexError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

NameError::NameError() = default;

NameError::NameError(const char* sMessage)
    : Exception(sMessage)
{}

NameError::NameError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

ImportError::ImportError() = default;

ImportError::ImportError(const char* sMessage)
    : Exception(sMessage)
{}

ImportError::ImportError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

AttributeError::AttributeError() = default;

AttributeError::AttributeError(const char* sMessage)
    : Exception(sMessage)
{}

AttributeError::AttributeError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

PropertyError::PropertyError() = default;

PropertyError::PropertyError(const char* sMessage)
    : AttributeError(sMessage)
{}

PropertyError::PropertyError(const std::string& sMessage)
    : AttributeError(sMessage)
{}


// ---------------------------------------------------------

RuntimeError::RuntimeError() = default;

RuntimeError::RuntimeError(const char* sMessage)
    : Exception(sMessage)
{}

RuntimeError::RuntimeError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

BadGraphError::BadGraphError()
    : RuntimeError("The graph must be a DAG.")
{}

BadGraphError::BadGraphError(const char* sMessage)
    : RuntimeError(sMessage)
{}

BadGraphError::BadGraphError(const std::string& sMessage)
    : RuntimeError(sMessage)
{}



// ---------------------------------------------------------

NotImplementedError::NotImplementedError() = default;

NotImplementedError::NotImplementedError(const char* sMessage)
    : Exception(sMessage)
{}

NotImplementedError::NotImplementedError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

ZeroDivisionError::ZeroDivisionError() = default;

ZeroDivisionError::ZeroDivisionError(const char* sMessage)
    : Exception(sMessage)
{}

ZeroDivisionError::ZeroDivisionError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

ReferenceError::ReferenceError() = default;

ReferenceError::ReferenceError(const char* sMessage)
    : Exception(sMessage)
{}

ReferenceError::ReferenceError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

ExpressionError::ExpressionError() = default;

ExpressionError::ExpressionError(const char* sMessage)
    : Exception(sMessage)
{}

ExpressionError::ExpressionError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

ParserError::ParserError() = default;

ParserError::ParserError(const char* sMessage)
    : Exception(sMessage)
{}

ParserError::ParserError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

UnicodeError::UnicodeError() = default;

UnicodeError::UnicodeError(const char* sMessage)
    : Exception(sMessage)
{}

UnicodeError::UnicodeError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

OverflowError::OverflowError() = default;

OverflowError::OverflowError(const char* sMessage)
    : Exception(sMessage)
{}

OverflowError::OverflowError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

UnderflowError::UnderflowError() = default;

UnderflowError::UnderflowError(const char* sMessage)
    : Exception(sMessage)
{}

UnderflowError::UnderflowError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

UnitsMismatchError::UnitsMismatchError() = default;

UnitsMismatchError::UnitsMismatchError(const char* sMessage)
    : Exception(sMessage)
{}

UnitsMismatchError::UnitsMismatchError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

CADKernelError::CADKernelError() = default;

CADKernelError::CADKernelError(const char* sMessage)
    : Exception(sMessage)
{}

CADKernelError::CADKernelError(const std::string& sMessage)
    : Exception(sMessage)
{}



// ---------------------------------------------------------

RestoreError::RestoreError() = default;

RestoreError::RestoreError(const char* sMessage)
    : Exception(sMessage)
{}

RestoreError::RestoreError(const std::string& sMessage)
    : Exception(sMessage)
{}


// ---------------------------------------------------------

#if defined(__GNUC__) && defined(FC_OS_LINUX)
#include <stdexcept>
#include <iostream>

SignalException::SignalException()
{
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = throw_signal;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    ok = (sigaction(SIGSEGV, &new_action, &old_action) < 0);
#ifdef _DEBUG
    std::cout << "Set new signal handler" << std::endl;
#endif
}

SignalException::~SignalException()
{
    sigaction(SIGSEGV, &old_action, nullptr);
#ifdef _DEBUG
    std::cout << "Restore old signal handler" << std::endl;
#endif
}

void SignalException::throw_signal(int signum)
{
    std::cerr << "SIGSEGV signal raised: " << signum << std::endl;
    throw std::runtime_error("throw_signal");
}
#endif
