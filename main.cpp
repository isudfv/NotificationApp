#include <QCoreApplication>
#include <QHttpServer>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>

#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>

#include <mailio/message.hpp>
#include <mailio/smtp.hpp>

#include "DesktopNotificationManagerCompat.h"

#include <iostream>
#include "Windows.h"
#include "DesktopNotificationManagerCompat.h"
#include <functional>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>
#include <conio.h>

using namespace winrt;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::UI::Notifications;

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::UI::Notifications;
using namespace Windows::Foundation::Collections;

void sendToast();

QString sendEmail(QVariantMap const& map) {
    using namespace mailio;
    auto __ = [&] (QString const& name) {
        return map[name].toString().toStdString();
    };
    try{
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
    catch (std::exception& e)
    {
        qDebug() << e.what();
        return e.what();
    }
    return "Email sent!";
}

QString sendTextToast(QVariantMap const& map)
{
    QStringList action_args;
    for (auto&& [key, value] : map["action"].toMap().asKeyValueRange()) {
        action_args.append(key + "=" + value.toString());
    }

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
</toast>");

    try{
        // Populate with text and values
        doc.DocumentElement().SetAttribute(L"launch", action_args.join('&').toStdWString());
        doc.SelectSingleNode(L"//text[1]").InnerText(map["subject"].toString().toStdWString());
        doc.SelectSingleNode(L"//text[2]").InnerText(map["content"].toString().toStdWString());
        doc.SelectSingleNode(L"//image[1]").as<XmlElement>().SetAttribute(L"src", map["icon"].toString().toStdWString());

        // Construct the notification
        ToastNotification notif{doc};

        notif.Tag(map["id"].toString().toStdWString());

        // And send it!
        DesktopNotificationManagerCompat::CreateToastNotifier().Show(notif);
    }
    catch (hresult_error& e) {
        return QString("code: %1, message: %2.").arg(e.code()).arg(QString::fromStdWString(e.message().data()));
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


int main(int argc, char *argv[]) {
    DesktopNotificationManagerCompat::Register(L"PC.NotificationApp", L"通知中心",
                                               L"C:\\Users\\isudfv\\Pictures\\fireworks.png");

    DesktopNotificationManagerCompat::OnActivated([](DesktopNotificationActivatedEventArgsCompat e) {
        qDebug() << e.Argument();
        QString args = QString::fromStdWString(e.Argument());
        if (args.contains("action=execute")) {
            QUrlQuery query{args};
            qDebug() << "execute : " << QProcess::startDetached(query.queryItemValue("exe_path"), query.allQueryItemValues("exe_args"));
        }
//        auto notification_id = QUrlQuery{args}.allQueryItemValues("notificationId").join(',');
//        DesktopNotificationManagerCompat::History().Remove(notification_id.toStdWString());
/*
        if (e.Argument()._Starts_with(L"action=like")) {
            sendBasicToast(L"Sent like!");

            if (!_hasStarted) {
                exit(0);
            }
        } else if (e.Argument()._Starts_with(L"action=reply")) {
            std::wstring msg = e.UserInput().Lookup(L"tbReply").c_str();

            sendBasicToast(L"Sent reply! Reply: " + msg);

            if (!_hasStarted) {
                exit(0);
            }
        } else {
            if (!_hasStarted) {
                if (e.Argument()._Starts_with(L"action=viewConversation")) {
                    std::cout << "Launched from toast, opening the conversation!\n\n";
                }

//                start();
            } else {
                showWindow();

                if (e.Argument()._Starts_with(L"action=viewConversation")) {
                    std::cout << "\n\nOpening the conversation!\n\nEnter a number to continue: ";
                } else {
                    std::wcout << L"\n\nToast activated!\n - Argument: " + e.Argument() +
                                  L"\n\nEnter a number to continue: ";
                }
            }
        }
*/
    });

    QCoreApplication app(argc, argv);

    QHttpServer http_server;
    http_server.route("/email", QHttpServerRequest::Method::Post, [](QHttpServerRequest const &request) {
        qDebug() << request;
        std::optional<QString> email_content;
        QJsonParseError error;
        auto document = QJsonDocument::fromJson(request.body(), &error);
        auto body =request.body();
        qDebug() << request.body();
        if (error.error != QJsonParseError::NoError) {
            qDebug() << error.errorString();
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
    http_server.route("/toast", QHttpServerRequest::Method::Post, [](QHttpServerRequest const &request) {
        std::optional<QString> toast_content;

        auto document = QJsonDocument::fromJson(request.body());
        auto map = document.object().toVariantMap();
        return sendTextToast(map);
    });

    http_server.listen(QHostAddress::Any, 8888);


    return QCoreApplication::exec();
}

void sendToast() {

    // Construct the toast template
    XmlDocument doc;
    doc.LoadXml(
            L"<toast>\
    <visual>\
        <binding template=\"ToastGeneric\">\
            <text></text>\
            <text></text>\
            <image placement=\"appLogoOverride\" hint-crop=\"circle\"/>\
            <image/>\
        </binding>\
    </visual>\
    <actions>\
        <input\
            id=\"tbReply\"\
            type=\"text\"\
            placeHolderContent=\"Type a reply\"/>\
        <action\
            content=\"Reply\"\
            activationType=\"background\"/>\
        <action\
            content=\"Like\"\
            activationType=\"background\"/>\
        <action\
            content=\"View\"\
            activationType=\"background\"/>\
    </actions>\
</toast>");

    // Populate with text and values
    doc.DocumentElement().SetAttribute(L"launch", L"action=viewConversation&conversationId=9813");
    doc.SelectSingleNode(L"//text[1]").InnerText(L"Andrew sent you a picture");
    doc.SelectSingleNode(L"//text[2]").InnerText(L"Check this out, Happy Canyon in Utah!");
    doc.SelectSingleNode(L"//image[1]").as<XmlElement>().SetAttribute(L"src", L"https://unsplash.it/64?image=1005");
    doc.SelectSingleNode(L"//image[2]").as<XmlElement>().SetAttribute(L"src",
                                                                      L"https://picsum.photos/364/202?image=883");
    doc.SelectSingleNode(L"//action[1]").as<XmlElement>().SetAttribute(L"arguments",
                                                                       L"action=reply&conversationId=9813");
    doc.SelectSingleNode(L"//action[2]").as<XmlElement>().SetAttribute(L"arguments",
                                                                       L"action=like&conversationId=9813");
    doc.SelectSingleNode(L"//action[3]").as<XmlElement>().SetAttribute(L"arguments",
                                                                       L"action=viewImage&imageUrl=https://picsum.photos/364/202?image=883");

    // Construct the notification
    ToastNotification notif{doc};

    // And send it!
    DesktopNotificationManagerCompat::CreateToastNotifier().Show(notif);

    std::cout << "Sent!\n";
}
