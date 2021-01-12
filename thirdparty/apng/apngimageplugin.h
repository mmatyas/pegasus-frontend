#ifndef APNGIMAGEPLUGIN_H
#define APNGIMAGEPLUGIN_H

#include <QImageIOPlugin>

class ApngImagePlugin : public QImageIOPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QImageIOHandlerFactoryInterface_iid FILE "qapng.json")

public:
	ApngImagePlugin(QObject *parent = nullptr);

	// QImageIOPlugin interface
	Capabilities capabilities(QIODevice *device, const QByteArray &format) const final;
	QImageIOHandler *create(QIODevice *device, const QByteArray &format) const final;
};

#endif // APNGIMAGEPLUGIN_H
