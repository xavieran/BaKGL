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

#include "Exception.h"
#include "GameFile.h"
#include "Directories.h"

GameFile::GameFile()
{}

GameFile::~GameFile()
{}

std::string
GameFile::GetDefaultPath() const
{
    return Directories::GetInstance()->GetGamesPath();
}

std::string
GameFile::GetAlternatePath() const
{
    return Directories::GetInstance()->GetUserPath();
}

std::string
GameFile::GetLastResortPath() const
{
    return Directories::GetInstance()->GetResourcePath();
}

std::string
GameFile::GetStoragePath() const
{
    return Directories::GetInstance()->GetGamesPath();
}
