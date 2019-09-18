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

#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#include <string>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class Directories
{
private:
    std::string resourcePath;
    std::string sharedPath;
    std::string userPath;
    std::string gamesPath;
    std::string capturePath;
    std::string dataPath;
    static Directories *instance;
    void CreatePath ( const std::string& path );
    std::string SearchResources() const;
protected:
    Directories();
public:
    ~Directories();
    static Directories* GetInstance();
    static void CleanUp();
    std::string GetResourcePath() const;
    std::string GetSharedPath() const;
    std::string GetUserPath() const;
    std::string GetGamesPath() const;
    std::string GetCapturePath() const;
    std::string GetDataPath() const;
    void SetResourcePath ( const std::string &path );
};

#endif
