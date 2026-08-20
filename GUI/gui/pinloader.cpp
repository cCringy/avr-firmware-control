#include "pinloader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "config.h"

QVariantList PinLoader::loadPins(const QString &path,qreal imageWidth,qreal imageHeight){
    QVariantList result;

    QFile file(path);

    if(!file.open(QIODevice::ReadOnly)){
        qWarning()<< "Couldnt open JSON:"<<path;
        return result;
    }

    auto doc = QJsonDocument::fromJson(file.readAll());

    for(const auto &val : doc.array()){
        QJsonObject jObj = val.toObject();
        QVariantMap pin;
        pin["name"] = jObj.value("name").toString();
        pin["type"] = jObj.value("type").toString();
        pin["relX"] = (jObj.value("x").toDouble()+14.5)/imageWidth;
        pin["relY"] = (jObj.value("y").toDouble()+14.5)/imageHeight;
        result.append(pin);
    }
    return result;
}


QVariantList PinLoader::loadDefaultPins(qreal imageWidth, qreal imageHeight) {
    QString path = QString(PROJECT_ROOT) + "/pindata/pins.json";
    return loadPins(path, imageWidth, imageHeight);
}
