#pragma once

#include "mailio/message.hpp"
#include "mailio/smtp.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include "fmt/core.h"
#include "fmt/xchar.h"

#include <unknwn.h> // Needed for notifications
#include "DesktopNotificationManagerCompat.h"

#include "Windows.h"
#include <conio.h>
#include <functional>
#include <iostream>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>

using namespace winrt;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::UI::Notifications;

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::UI::Notifications;
using namespace Windows::Foundation::Collections;

namespace utils{
std::string sendEmail(nlohmann::json const& j);
std::string sendTextToast(nlohmann::json const& j);
}

