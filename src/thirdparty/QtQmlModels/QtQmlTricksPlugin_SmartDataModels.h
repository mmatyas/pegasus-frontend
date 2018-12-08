#ifndef QTQMLTRICKSPLUGIN_SMARTDATAMODELS_H
#define QTQMLTRICKSPLUGIN_SMARTDATAMODELS_H

#include <QQmlEngine>
#include <QtQml>

#include "QQmlObjectListModel.h"
#include "QQmlVariantListModel.h"

static void registerQtQmlTricksSmartDataModel (QQmlEngine * engine) {
    Q_UNUSED (engine)

    const char * uri = "QtQmlTricks.SmartDataModels"; // @uri QtQmlTricks.SmartDataModels
    const int    maj = 2;
    const int    min = 0;
    const char * msg = "!!!";

    qmlRegisterUncreatableType<QQmlObjectListModelBase> (uri, maj, min, "ObjectListModel",  msg);
    qmlRegisterUncreatableType<QQmlVariantListModel>    (uri, maj, min, "VariantListModel", msg);
}

#endif // QTQMLTRICKSPLUGIN_SMARTDATAMODELS_H
