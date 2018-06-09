#pragma once

#include <QSharedPointer>

namespace modeldata { struct Collection; }
namespace modeldata { struct Game; }
namespace modeldata { struct GameAssets; }
namespace modeldata { using GamePtr = QSharedPointer<modeldata::Game>; }
