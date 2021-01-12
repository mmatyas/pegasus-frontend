#include "apngimagehandler_p.h"
#include "apngreader_p.h"

ApngImageHandler::ApngImageHandler() :
	QImageIOHandler{},
	_reader{new ApngReader{}}
{}

ApngImageHandler::~ApngImageHandler() = default;

QByteArray ApngImageHandler::name() const
{
	return "apng";
}

bool ApngImageHandler::canRead() const
{
	auto valid = _reader->init(device());
	if(valid)
		setFormat("apng");
	return valid && _index < _reader->frames();
}

bool ApngImageHandler::read(QImage *image)
{
	if(!_reader->init(device()))
		return false;
	*image = _reader->readFrame(_index++);
	return !image->isNull();
}

QVariant ApngImageHandler::option(QImageIOHandler::ImageOption option) const
{
	switch(option) {
	case QImageIOHandler::Size:
		if(_reader->init(device()))
			return _reader->size();
		else
			return QSize();
	case QImageIOHandler::Animation:
		if(_reader->init(device()))
			return _reader->isAnimated();
		else
			return true;
	default:
		return QVariant();
	}
}

bool ApngImageHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
	switch(option) {
	case QImageIOHandler::Size:
	case QImageIOHandler::Animation:
		return true;
	default:
		return false;
	}
}

bool ApngImageHandler::jumpToNextImage()
{
	if(!_reader->init(device()))
		return false;
	else
		return (++_index < _reader->frames());
}

bool ApngImageHandler::jumpToImage(int imageNumber)
{
	if(!_reader->init(device()) || imageNumber < 0)
		return false;
	_index = static_cast<quint32>(imageNumber);
	return _index < _reader->frames();
}

int ApngImageHandler::loopCount() const
{
	if(_reader->init(device()) && _reader->isAnimated()) {
		auto plays = _reader->plays();
		if(plays == 0)
			return -1;
		else
			return static_cast<int>(plays);
	} else
		return 0;
}

int ApngImageHandler::imageCount() const
{
	if(_reader->init(device()))
		return static_cast<int>(_reader->frames());
	else
		return 0;
}

int ApngImageHandler::nextImageDelay() const
{
	if(!_reader->init(device()))
		return false;
	if(_index == 0 || _index > _reader->frames())
		return 0;
	else
		return _reader->readFrame(_index - 1).delayMsec();
}

int ApngImageHandler::currentImageNumber() const
{
	return _index < _reader->frames() ? static_cast<int>(_index) : -1;
}
