#pragma once
#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

class PinLoader : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    using QObject::QObject;

    Q_INVOKABLE QVariantList loadPins(const QString &path, qreal imageWidth, qreal imageHeight);
    Q_INVOKABLE QVariantList loadDefaultPins(qreal imageWidth, qreal imageHeight);
};