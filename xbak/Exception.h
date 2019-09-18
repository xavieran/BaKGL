/*
 * This file is part of xBaK.
 *
 * xBaK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xBaK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xBaK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Guido de Jong <guidoj@users.sf.net>
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class Exception: public std::exception
{
private:
    std::string filename;
    unsigned int linenr;
    std::string message;
public:
    Exception ( const std::string &file, const unsigned int line, const std::string &msg );
    Exception ( const std::string &file, const unsigned int line, const std::string &msg, const unsigned int val );
    virtual ~Exception() throw();
    void Print ( const std::string &handler ) const throw();
    std::string What() const throw();
};

class BufferEmpty: public Exception
{
public:
    BufferEmpty ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    virtual ~BufferEmpty() throw ();
};

class BufferFull: public Exception
{
public:
    BufferFull ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    virtual ~BufferFull() throw ();
};

class CompressionError: public Exception
{
public:
    CompressionError ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    CompressionError ( const std::string &file, const unsigned int line, const std::string &msg, const unsigned int value );
    virtual ~CompressionError() throw ();
};

class DataCorruption: public Exception
{
public:
    DataCorruption ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    DataCorruption ( const std::string &file, const unsigned int line, const std::string &msg, const unsigned int value );
    virtual ~DataCorruption() throw ();
};

class FileNotFound: public Exception
{
public:
    FileNotFound ( const std::string &file, const unsigned int line, const std::string &msg );
    virtual ~FileNotFound() throw ();
};

class IndexOutOfRange: public Exception
{
public:
    IndexOutOfRange ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    IndexOutOfRange ( const std::string &file, const unsigned int line, const std::string &msg, const unsigned int value );
    virtual ~IndexOutOfRange() throw ();
};

class IOError: public Exception
{
public:
    IOError ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    virtual ~IOError() throw ();
};

class NullPointer: public Exception
{
public:
    NullPointer ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    virtual ~NullPointer() throw ();
};

class OpenError: public Exception
{
public:
    OpenError ( const std::string &file, const unsigned int line, const std::string &msg = "" );
    virtual ~OpenError() throw ();
};

class SDL_Exception: public Exception
{
public:
    SDL_Exception ( const std::string &file, const unsigned int line, const std::string &msg );
    virtual ~SDL_Exception() throw ();
};

class UnexpectedValue: public Exception
{
public:
    UnexpectedValue ( const std::string &file, const unsigned int line, const std::string &value );
    UnexpectedValue ( const std::string &file, const unsigned int line, const unsigned int value );
    virtual ~UnexpectedValue() throw ();
};

#endif
