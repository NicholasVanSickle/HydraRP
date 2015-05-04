#pragma once

#include <lib/libexport.h>

#include <QVariant>
#include <QObject>

class HYDRAEXPORT PropertyContainer
{
public:
    virtual QStringList properties() = 0;
    virtual QVariant property(QString propertyName) = 0;
    virtual void setProperty(QString propertyName, QVariant value) = 0;
};

class HYDRAEXPORT PropertyTable : public QObject
{
    Q_OBJECT

public:
    PropertyTable(QObject* parent = nullptr);
};
