#include "pinloader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

QVariantList PinLoader::loadPins(const QString &path,qreal imageWidth,qreal imageHeight){
    QVariantList result;

    QString localPath = path;
    if (localPath.startsWith("qrc:/")) {
        localPath = ":" + localPath.mid(4);   // "qrc:/foo" -> ":/foo"
    }

    QFile file(localPath);

    if(!file.open(QIODevice::ReadOnly)){
        qWarning()<< "Couldnt open JSON:"<<localPath;
        return result;
    }

    auto doc = QJsonDocument::fromJson(file.readAll());

    for(const auto &val : doc.array()){
        QJsonObject jObj = val.toObject();
        QVariantMap pin;
        pin["name"] = jObj.value("name").toString();
        pin["type"] = jObj.value("name").toString();
        pin["relX"] = jObj.value("x").toDouble()/imageWidth;
        pin["relY"] = jObj.value("x").toDouble()/imageHeight;
        result.append(pin);
    }
    return result;
}
