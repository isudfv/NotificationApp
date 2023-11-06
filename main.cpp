#include <QCoreApplication>
#include <QFile>
#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSslKey>

#include "mailio/message.hpp"
#include "mailio/smtp.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"

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

QString sendEmail(QVariantMap const& map)
{
    using namespace mailio;
    auto __ = [&](QString const& name) {
        return map[name].toString().toStdString();
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

QString sendTextToast(QVariantMap const& map)
{
//    QStringList action_args;
//    for (auto&& [key, value] : map["action"].toMap().asKeyValueRange()) {
//        action_args.append(key + "=" + value.toString());
//    }

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
        doc.DocumentElement().SetAttribute(L"launch", L"123");
        doc.SelectSingleNode(L"//text[1]").InnerText(map["subject"].toString().toStdWString());
        doc.SelectSingleNode(L"//text[2]").InnerText(map["content"].toString().toStdWString());
//        doc.SelectSingleNode(L"//image[1]").as<XmlElement>().SetAttribute(L"src", map["icon"].toString().toStdWString());

        // Construct the notification
        ToastNotification notif{doc};

        notif.Tag(map["id"].toString().toStdWString());

        // And send it!
        ToastNotificationManager::CreateToastNotifier(L"PC.NotificationApp").Show(notif);
    }
    catch (hresult_error& e) {
//        SPDLOG_ERROR(L"code: {}, message: {}.", (int)e.code(), (std::wstring_view)e.message());
//        return QString("code: %1, message: %2.").arg(e.code()).arg(QString::fromStdWString(e.message().data()));
    }
    return "Show Toast!";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


int main(int argc, char* argv[])
{
//    DesktopNotificationManagerCompat::Register(L"PC.NotificationApp", L"通知中心", L"C:\\Users\\isudfv\\Pictures\\fireworks.png");

    try {
        auto logger = std::make_shared<spdlog::logger>(
            "combined_logger",
            spdlog::sinks_init_list{
                std::make_shared<spdlog::sinks::stdout_sink_mt>(),
                std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/logs.txt"),
            }
        );
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%C-%m-%d %H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] [source %s] [function %!] [line %#] %v");
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log init failed: " << ex.what() << std::endl;
    }

//    DesktopNotificationManagerCompat::OnActivated([](DesktopNotificationActivatedEventArgsCompat e) {
//        SPDLOG_INFO(L"On Toast Activated: {}", e.Argument());
//        QString args = QString::fromStdWString(e.Argument());
//        if (args.contains("action=execute")) {
//            QUrlQuery query{args};
//            SPDLOG_INFO("Execute: {}", QProcess::startDetached(query.queryItemValue("exe_path"), query.allQueryItemValues("exe_args")));
//        }
//    });

    QCoreApplication app(argc, argv);

    QHttpServer http_server;
    http_server.route("/email", QHttpServerRequest::Method::Post, [](QHttpServerRequest const& request) {
        SPDLOG_INFO("[Notification] request body: {}", request.body().toStdString());
        QJsonParseError error;
        auto document = QJsonDocument::fromJson(request.body(), &error);
        if (error.error != QJsonParseError::NoError) {
            SPDLOG_ERROR("[Notification] request body: {}", error.errorString().toStdString());
            return error.errorString();
        }
        auto map = document.object().toVariantMap();
        for (auto&& [header, value] : request.headers()) {
            map.insert(header, value);
        }
        auto result = sendEmail(map);
        return result;
    });
    http_server.route("/hello", QHttpServerRequest::Method::Post, []() {
        return "Hello";
    });
    http_server.route("/toast", QHttpServerRequest::Method::Post, [](QHttpServerRequest const& request) {
        SPDLOG_INFO("[Notification] request body: {}", request.body().toStdString());
        auto document = QJsonDocument::fromJson(request.body());
        auto map = document.object().toVariantMap();
        return sendTextToast(map);
    });

    http_server.listen(QHostAddress::Any, 8888);


    return QCoreApplication::exec();
}
