// SPDX-License-Identifier: LGPL-2.1-or-later

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

#include <utility>




#include "Exception.h"


using namespace Base;


TYPESYSTEM_SOURCE(Base::Exception, Base::BaseClass)
Exception::Exception(std::string message)
    : errorMessage {std::move(message)}
{}

Exception::Exception(const Exception& inst) = default;

Exception::Exception(Exception&& inst) noexcept = default;

Exception& Exception::operator=(const Exception& inst)
{
    errorMessage = inst.errorMessage;
    fileName = inst.fileName;
    lineNum = inst.lineNum;
    functionName = inst.functionName;
    isTranslatable = inst.isTranslatable;
    return *this;
}

Exception& Exception::operator=(Exception&& inst) noexcept
{
    errorMessage = std::move(inst.errorMessage);
    fileName = std::move(inst.fileName);
    lineNum = inst.lineNum;
    functionName = std::move(inst.functionName);
    isTranslatable = inst.isTranslatable;
    return *this;
}

const char* Exception::what() const noexcept
{
    return errorMessage.c_str();
}

void Exception::reportException() const
{
    if (hasBeenReported) {
        return;
    }

    std::string msg = errorMessage.empty() ? typeid(*this).name() : errorMessage;

#ifdef FC_DEBUG
    if (!functionName.empty()) {
        msg = functionName + " -- " + msg;
    }
#endif

    hasBeenReported = true;
}




// ---------------------------------------------------------

TYPESYSTEM_SOURCE(Base::AbortException, Base::Exception)

AbortException::AbortException(const std::string& message)
    : Exception(message)
{}

const char* AbortException::what() const noexcept
{
    return Exception::what();
}



// ---------------------------------------------------------

XMLBaseException::XMLBaseException(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

XMLParseException::XMLParseException(const std::string& message)
    : XMLBaseException(message)
{}

const char* XMLParseException::what() const noexcept
{
    return XMLBaseException::what();
}



// ---------------------------------------------------------

XMLAttributeError::XMLAttributeError(const std::string& message)
    : XMLBaseException(message)
{}

const char* XMLAttributeError::what() const noexcept
{
    return XMLBaseException::what();
}



// ---------------------------------------------------------

FileException::FileException(const std::string& message, const std::string& fileName)
    : Exception(message)
    , file(fileName)
{
    setFileName(fileName);
}

FileException::FileException(const std::string& message, const FileInfo& File)
    : Exception(message)
    , file(File)
{
    setFileName(File.filePath());
}

void FileException::setFileName(const std::string& fileName)
{
    file.setFile(fileName);
    _sErrMsgAndFileName = getMessage();
    if (!getFile().empty()) {
        _sErrMsgAndFileName += ": ";
        _sErrMsgAndFileName += fileName;
    }
}

std::string FileException::getFileName() const
{
    return file.fileName();
}

const char* FileException::what() const noexcept
{
    return _sErrMsgAndFileName.c_str();
}

void FileException::reportException() const
{
    if (getReported()) {
        return;
    }
    std::string msg = _sErrMsgAndFileName.empty() ? typeid(*this).name() : _sErrMsgAndFileName;

#ifdef FC_DEBUG
    if (!getFunction().empty()) {
        msg = getFunction() + " -- " + msg;
    }
#endif

    //_FC_ERR(getFile().c_str(), getLine(), msg);
    setReported(true);
}





// ---------------------------------------------------------

FileSystemError::FileSystemError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

BadFormatError::BadFormatError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

MemoryException::MemoryException(const std::string& message)
    : Exception(message)  // NOLINT(*-throw-keyword-missing)
{}

#if defined(__GNUC__)
const char* MemoryException::what() const noexcept
{
    return Exception::what();  // from Exception, not std::bad_alloc
}
#endif


// ---------------------------------------------------------

AccessViolation::AccessViolation(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

AbnormalProgramTermination::AbnormalProgramTermination(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

UnknownProgramOption::UnknownProgramOption(const std::string& message)
    : Exception(message)
{}

// ---------------------------------------------------------

ProgramInformation::ProgramInformation(const std::string& message)
    : Exception(message)
{}

// ---------------------------------------------------------

TypeError::TypeError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

ValueError::ValueError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

IndexError::IndexError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

NameError::NameError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

ImportError::ImportError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

AttributeError::AttributeError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

PropertyError::PropertyError(const std::string& message)
    : AttributeError(message)
{}


// ---------------------------------------------------------

RuntimeError::RuntimeError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

BadGraphError::BadGraphError(const std::string& message)
    : RuntimeError(message)
{}



// ---------------------------------------------------------

NotImplementedError::NotImplementedError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

ZeroDivisionError::ZeroDivisionError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

ReferenceError::ReferenceError(const std::string& message)
    : Exception(message)
{}

// ---------------------------------------------------------

ExpressionError::ExpressionError(const std::string& message)
    : Exception(message)
{}

// ---------------------------------------------------------

ParserError::ParserError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

UnicodeError::UnicodeError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

OverflowError::OverflowError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

UnderflowError::UnderflowError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

UnitsMismatchError::UnitsMismatchError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

CADKernelError::CADKernelError(const std::string& message)
    : Exception(message)
{}


// ---------------------------------------------------------

RestoreError::RestoreError(const std::string& message)
    : Exception(message)
{}



// ---------------------------------------------------------

#if defined(__GNUC__) && defined(FC_OS_LINUX)
# include <stdexcept>
# include <iostream>
# include <csignal>

SignalException::SignalException()
{
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = throw_signal;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    ok = (sigaction(SIGSEGV, &new_action, &old_action) < 0);
# ifdef _DEBUG
    std::cout << "Set new signal handler" << std::endl;
# endif
}

SignalException::~SignalException()
{
    sigaction(SIGSEGV, &old_action, nullptr);
# ifdef _DEBUG
    std::cout << "Restore old signal handler" << std::endl;
# endif
}

void SignalException::throw_signal(const int signum)
{
    std::cerr << "SIGSEGV signal raised: " << signum << std::endl;
    throw std::runtime_error("throw_signal");
}
#endif
