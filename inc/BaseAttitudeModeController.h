#pragma once

#include "OrbiterSDK.h"
#include "IDisplay.h"
#include <functional>
#include <memory>

typedef std::function<std::shared_ptr<IDisplay>(oapi::Sketchpad* sketchpad)> CreateDisplayFunction;
