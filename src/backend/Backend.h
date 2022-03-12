// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "CliArgs.h"

namespace model { class ApiObject; }
namespace model { class Internal; }
class FrontendLayer;
class ProcessLauncher;


namespace backend {

class Backend {
public:
    explicit Backend();
    explicit Backend(const CliArgs&);
    ~Backend();

    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;

    void start();

private:
    // frontend <-> api <-> launcher
    // NOTE: unique_ptr had forward declaration issues
    model::ApiObject* m_api_public;
    model::Internal* m_api_private;
    FrontendLayer* m_frontend;
    ProcessLauncher* m_launcher;
};

} // namespace backend
