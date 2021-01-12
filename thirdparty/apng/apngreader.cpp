#include "apngreader_p.h"
#include <QDebug>
#include <QImage>
#include <QRect>
#include <QThread>
#include <QtEndian>

ApngReader::ApngReader(QObject *parent) :
	QObject{parent}
{}

ApngReader::~ApngReader()
{
	if(_png)
		png_destroy_read_struct(&_png, &_info, nullptr);

	if (_frame.rows)
		delete[] _frame.rows;
	if (_frame.p)
		delete[] _frame.p;
}

bool ApngReader::checkPngSig(QIODevice *device)
{
	if(device) {
		auto sig = device->peek(8);
		if(png_sig_cmp(reinterpret_cast<png_const_bytep>(sig.constData()), 0, static_cast<png_size_t>(sig.size())) == 0)
			return true;
	}
	return false;
}

bool ApngReader::init(QIODevice *device)
{
	if (_device == device) {
		if (_device->pos() < _infoOffset) {
			_allFrames.clear();
			_device->seek(_infoOffset);
		}
		return _infoOffset > 0;
	}

	//verify png
	if(checkPngSig(device))
		_device = device;
	else
		return false;

	// clear cached frames
	_allFrames.clear();

	//init png structs
	_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(!_png) {
		qCritical() << "failed to create png struct";
		return false;
	}

	_info = png_create_info_struct(_png);
	if(!_info) {
		qCritical() << "failed to create info struct";
		return false;
	}

	png_set_progressive_read_fn(_png, this, &ApngReader::info_fn, &ApngReader::row_fn, &ApngReader::end_fn);

	//set png jump position
	if (setjmp(png_jmpbuf(_png))) {
		_infoOffset = 0;
		return false;
	}

	//read image sig + header
	auto valid = readChunk(8);
	do {
		valid = readChunk();
	} while(valid && _infoOffset == 0);

	return _infoOffset > 0;
}

ApngReader::ApngFrame ApngReader::readFrame(quint32 index)
{
	return readFrame(static_cast<int>(index));
}

ApngReader::ApngFrame ApngReader::readFrame(int index)
{
	if(index < _allFrames.size())
		return _allFrames[index];

	if (setjmp(png_jmpbuf(_png)))
		return {};

	auto valid = false;
	do {
		valid = readChunk();
	} while(valid && index >= _allFrames.size());

	if(index < _allFrames.size())
		return _allFrames[index];
	else
		return {};
}

bool ApngReader::isAnimated() const
{
	return _animated;
}

QSize ApngReader::size() const
{
	return _imageSize;
}

quint32 ApngReader::frames() const
{
	return _frameCount;
}

quint32 ApngReader::plays() const
{
	return _plays;
}

void ApngReader::info_fn(png_structp png_ptr, png_infop info_ptr)
{
	auto reader = reinterpret_cast<ApngReader*>(png_get_io_ptr(png_ptr));
	Frame &frame = reader->_frame;

	//init png reading
	png_set_expand(png_ptr);
	png_set_strip_16(png_ptr);
	png_set_gray_to_rgb(png_ptr);
	png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
	png_set_bgr(png_ptr);
	(void)png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	//init read frame struct
	frame.x = 0;
	frame.y = 0;
	frame.width = png_get_image_width(png_ptr, info_ptr);
	frame.height = png_get_image_height(png_ptr, info_ptr);
	frame.channels = png_get_channels(png_ptr, info_ptr);
	frame.delay_num = 0;
	frame.delay_den = 10;
	frame.dop = PNG_DISPOSE_OP_NONE;
	frame.bop = PNG_BLEND_OP_SOURCE;
	frame.rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	frame.p = new unsigned char[frame.height * frame.rowbytes];
	frame.rows = new png_bytep[frame.height * sizeof(png_bytep)];
	for (quint32 j = 0; j < frame.height; j++)
		frame.rows[j] = frame.p + j * frame.rowbytes;

	// init image
	reader->_lastImg = QImage(static_cast<int>(frame.width),
							  static_cast<int>(frame.height),
							  QImage::Format_ARGB32);
	reader->_lastImg.fill(Qt::transparent);

	//read apng information
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL)) {
		reader->_animated = true;
		png_get_acTL(png_ptr, info_ptr, &reader->_frameCount, &reader->_plays);
		reader->_skipFirst = png_get_first_frame_is_hidden(png_ptr, info_ptr);

		//add extended APNG read functions
		png_set_progressive_frame_fn(png_ptr, &ApngReader::frame_info_fn, &ApngReader::frame_end_fn);
		//read info for first frame (skipped otherwise)
		if(!reader->_skipFirst)
			frame_info_fn(png_ptr, 0);
	} else
		reader->_animated = false;

	reader->_infoOffset = reader->_device->pos();
}

void ApngReader::row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
	Q_UNUSED(pass)
	auto reader = reinterpret_cast<ApngReader*>(png_get_io_ptr(png_ptr));
	png_progressive_combine_row(png_ptr, reader->_frame.rows[row_num], new_row);
}

void ApngReader::end_fn(png_structp png_ptr, png_infop info_ptr)
{
	Q_UNUSED(info_ptr);
	auto reader = reinterpret_cast<ApngReader*>(png_get_io_ptr(png_ptr));
	Frame &frame = reader->_frame;

	if(!reader->_animated) {
		reader->copyOver();
		reader->_allFrames.append(reader->_lastImg);
	}

	if (frame.rows) {
		delete[] frame.rows;
		frame.rows = nullptr;
	}
	if (frame.p) {
		delete[] frame.p;
		frame.p = nullptr;
	}
}

void ApngReader::frame_info_fn(png_structp png_ptr, png_uint_32 frame_num)
{
	Q_UNUSED(frame_num);
	auto reader = reinterpret_cast<ApngReader*>(png_get_io_ptr(png_ptr));
	auto info_ptr = reader->_info;
	Frame &frame = reader->_frame;

	frame.x = png_get_next_frame_x_offset(png_ptr, info_ptr);
	frame.y = png_get_next_frame_y_offset(png_ptr, info_ptr);
	frame.width = png_get_next_frame_width(png_ptr, info_ptr);
	frame.height = png_get_next_frame_height(png_ptr, info_ptr);
	frame.delay_num = png_get_next_frame_delay_num(png_ptr, info_ptr);
	frame.delay_den = png_get_next_frame_delay_den(png_ptr, info_ptr);
	frame.dop = png_get_next_frame_dispose_op(png_ptr, info_ptr);
	frame.bop = png_get_next_frame_blend_op(png_ptr, info_ptr);
}

void ApngReader::frame_end_fn(png_structp png_ptr, png_uint_32 frame_num)
{
	auto reader = reinterpret_cast<ApngReader*>(png_get_io_ptr(png_ptr));
	Frame &frame = reader->_frame;
	auto &image = reader->_lastImg;

	if(frame_num == 0 && reader->_skipFirst)//TODO not always, only when reading animated
		return;
	if((frame_num == 0 && !reader->_skipFirst) ||
	   (frame_num == 1 && reader->_skipFirst)) {
		frame.bop = PNG_BLEND_OP_SOURCE;
		if (frame.dop == PNG_DISPOSE_OP_PREVIOUS)
			frame.dop = PNG_DISPOSE_OP_BACKGROUND;
	}

	QImage temp;

	if(frame.dop == PNG_DISPOSE_OP_PREVIOUS)
		temp = image;

	if (frame.bop == PNG_BLEND_OP_OVER)
		reader->blendOver();
	else
		reader->copyOver();

	reader->_allFrames.append({image, frame.delay_num, frame.delay_den});

	if (frame.dop == PNG_DISPOSE_OP_PREVIOUS)
		image = temp;
	else if (frame.dop == PNG_DISPOSE_OP_BACKGROUND) {
		for(auto y = 0u; y < frame.height; y++) {
			for(auto x = 0u; x < frame.width; x++)
				image.setPixelColor(static_cast<int>(x + frame.x),
									static_cast<int>(y + frame.y),
									Qt::transparent);
		}
	}
}

bool ApngReader::readChunk(quint32 len)
{
	QByteArray data;
	if(len == 0) {// read exactly 1 chunk
		//read 4 bytes -> size
		data = _device->read(4);
		if(data.size() == 4) {
			len = qFromBigEndian<quint32>(data.constData()) + 8u;//type (4b) + crc (4b)
			data += _device->read(len);
		}
		//is save for invalid data, as at least 4 byte are always read
	} else
		data = _device->read(len);
	if(!data.isEmpty())
		png_process_data(_png, _info, reinterpret_cast<png_bytep>(data.data()), static_cast<png_size_t>(data.size()));
	return !_device->atEnd();
}

void ApngReader::copyOver()
{
	for(quint32 y = 0; y < _frame.height; y++) {
		for(quint32 x = 0; x < _frame.width; x++) {
			auto px = x*4;

			QColor c;
			c.setBlue(_frame.rows[y][px]);
			c.setGreen(_frame.rows[y][px+1]);
			c.setRed(_frame.rows[y][px+2]);
			c.setAlpha(_frame.rows[y][px+3]);

			_lastImg.setPixelColor(static_cast<int>(x + _frame.x),
								   static_cast<int>(y + _frame.y),
								   c);
		}
	}
}

void ApngReader::blendOver()
{
	for(quint32 y = 0; y < _frame.height; y++) {
		for(quint32 x = 0; x < _frame.width; x++) {
			auto px = x*4;

			QColor src;
			src.setBlue(_frame.rows[y][px]);
			src.setGreen(_frame.rows[y][px+1]);
			src.setRed(_frame.rows[y][px+2]);
			src.setAlpha(_frame.rows[y][px+3]);

			if(src.alpha() == 0xFF) {
				_lastImg.setPixelColor(static_cast<int>(x + _frame.x),
									   static_cast<int>(y + _frame.y),
									   src);
			} else if(src.alpha() != 0x00) {
				auto dst = _lastImg.pixelColor(static_cast<int>(x + _frame.x),
											   static_cast<int>(y + _frame.y));

				//do porter-duff blending
				if(dst.alpha() != 0x00) {
					auto u = src.alpha() * 0xFF;
					auto v = (0xFF - src.alpha()) * dst.alpha();
					auto al = u + v;
					src.setBlue((src.blue() * u + dst.blue() * v) / al);
					src.setGreen((src.green() * u + dst.green() * v) / al);
					src.setRed((src.red() * u + dst.red() * v) / al);
					src.setAlpha(al/255);
				}

				_lastImg.setPixelColor(static_cast<int>(x + _frame.x),
									   static_cast<int>(y + _frame.y),
									   src);
			}
		}
	}
}



ApngReader::ApngFrame::ApngFrame(const QImage &image, quint16 delay_num, quint16 delay_den) :
	QImage{image},
	_delay{static_cast<double>(delay_num) / static_cast<double>(delay_den)}
{}

double ApngReader::ApngFrame::delay() const
{
	return _delay;
}

int ApngReader::ApngFrame::delayMsec() const
{
	return qRound(_delay * 1000);
}
