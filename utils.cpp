#include "utils.h"

namespace utils {
std::string sendEmail(nlohmann::json const& j)
{
    using namespace mailio;
    auto __ = [&](std::string_view name) {
        return j.at(name);
    };
    try {
        mailio::message msg;
        msg.header_codec(message::header_codec_t::BASE64);
        msg.from(mail_address(__("from_name"), __("from_email")));
        msg.add_recipient(mail_address(__("to_name"), __("to_email")));
        msg.subject(__("subject"));
        msg.add_references(__("ref_id"));
        msg.content_transfer_encoding(mime::content_transfer_encoding_t::QUOTED_PRINTABLE);
        msg.content_type(message::media_type_t::TEXT, "html", "utf-8");
        msg.content(__("content"));
        smtps conn(__("smtp_host_name"), 587);
        conn.authenticate(__("from_email"), __("Smtp-Password"), smtps::auth_method_t::START_TLS);
        conn.submit(msg);
    }
    catch (std::exception& e) {
        SPDLOG_ERROR("{}", e.what());
        return e.what();
    }
    return "Email sent!";
}

std::string sendTextToast(nlohmann::json const& j)
{
    // Construct the toast template
    // https://learn.microsoft.com/en-us/uwp/schemas/tiles/toastschema/element-image
    XmlDocument doc;
    doc.LoadXml(
        L"<toast>\
    <visual>\
        <binding template=\"ToastGeneric\">\
            <text></text>\
            <text></text>\
            <image placement=\"appLogoOverride\" />\
        </binding>\
    </visual>\
</toast>"
    );

    try {
        // Populate with text and values
        doc.DocumentElement().SetAttribute(L"launch", winrt::to_hstring(to_string(j["action"])));
        doc.SelectSingleNode(L"//text[1]").InnerText(winrt::to_hstring(j["subject"].get<std::string>()));
        doc.SelectSingleNode(L"//text[2]").InnerText(winrt::to_hstring(j["content"].get<std::string>()));
        doc.SelectSingleNode(L"//image[1]").as<XmlElement>().SetAttribute(L"src", winrt::to_hstring(j["icon"].get<std::string>()));

        // Construct the notification
        ToastNotification notif{doc};

        notif.Tag(winrt::to_hstring(j["id"].get<std::string>()));

        // And send it!
        DesktopNotificationManagerCompat::CreateToastNotifier().Show(notif);
    }
    catch (hresult_error& e) {
        auto error = fmt::format(L"code: {}, message: {}.", (int) e.code(), (std::wstring_view) e.message());
        SPDLOG_ERROR(error);
        return winrt::to_string(error);
    }
    catch (std::exception& e) {
        auto error = fmt::format("message: {}.", e.what());
        SPDLOG_ERROR(error);
        return error;
    }
    return "Show Toast!";
}

}