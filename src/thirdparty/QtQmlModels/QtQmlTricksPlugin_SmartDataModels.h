#ifndef QTQMLTRICKSPLUGIN_SMARTDATAMODELS_H
#define QTQMLTRICKSPLUGIN_SMARTDATAMODELS_H

#include <QQmlEngine>
#include <QtQml>

#include "QQmlObjectListModel.h"
#include "QQmlVariantListModel.h"

static void registerQtQmlTricksSmartDataModel ()
{
    constexpr auto uri = "QtQmlTricks.SmartDataModels"; // @uri QtQmlTricks.SmartDataModels
    constexpr int maj = 2;
    constexpr int min = 0;
    constexpr auto msg = "!!!";

    qmlRegisterUncreatableType<QQmlObjectListModelBase> (uri, maj, min, "ObjectListModel",  msg);
    qmlRegisterUncreatableType<QQmlVariantListModel>    (uri, maj, min, "VariantListModel", msg);
}

#endif // QTQMLTRICKSPLUGIN_SMARTDATAMODELS_H
