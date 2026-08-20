#ifndef PINLOADER_H
#define PINLOADER_H
#pragma once
#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

class PinLoader : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    using QObject::QObject;

    Q_INVOKABLE QVariantList loadPins(const QString &path, qreal ImageWidth,qreal ImageHeight);
};

#endif // PINLOADER_H
