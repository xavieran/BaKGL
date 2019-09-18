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

#ifndef GENERIC_FILE_H
#define GENERIC_FILE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FileBuffer.h"

class BasicFile
{
private:
    std::ifstream ifs;
    std::ofstream ofs;
public:
    BasicFile();
    virtual ~BasicFile();
    virtual std::string GetDefaultPath() const;
    virtual std::string GetAlternatePath() const;
    virtual std::string GetLastResortPath() const;
    virtual std::string GetStoragePath() const;
    void Open ( const std::string &name, const bool writable );
    void Close();
    void Seek ( const std::streamoff offset );
    void SeekEnd ( const std::streamoff offset );
    std::streamsize Size();
    void Load ( FileBuffer &buffer );
    void Save ( FileBuffer &buffer );
    void Save ( FileBuffer &buffer, const unsigned int n );
};

#endif
