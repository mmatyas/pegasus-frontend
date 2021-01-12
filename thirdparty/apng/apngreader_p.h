#ifndef APNGREADER_P_H
#define APNGREADER_P_H

#include <QObject>
#include <QIODevice>
#include <png.h>
#include <QImage>

#ifndef PNG_APNG_SUPPORTED
#error libpng with APNG patch is required
#endif
#ifndef PNG_READ_APNG_SUPPORTED
#error libpng with APNG patch and APNG read support is required
#endif

class ApngReader : public QObject
{
	Q_OBJECT

public:
	class ApngFrame : public QImage {
	public:
		ApngFrame(const QImage &image = {}, quint16 delay_num = 0, quint16 delay_den = 10);

		double delay() const;
		int delayMsec() const;

	private:
		double _delay;
	};

	explicit ApngReader(QObject *parent = nullptr);
	~ApngReader() override;

	static bool checkPngSig(QIODevice *device);

	bool init(QIODevice *device);
	ApngFrame readFrame(quint32 index);
	ApngFrame readFrame(int index);

	bool isAnimated() const;
	QSize size() const;
	quint32 frames() const;
	quint32 plays() const;

private:
	QIODevice *_device = nullptr;

	png_structp _png = nullptr;
	png_infop _info = nullptr;

	//image info
	qint64 _infoOffset = 0;
	bool _animated = false;
	bool _skipFirst = false;
	QSize _imageSize;
	quint32 _frameCount = 1;
	quint32 _plays = 0;

	struct Frame {
		quint32 x = 0;
		quint32 y = 0;
		quint32 width = 0;
		quint32 height = 0;
		quint32 channels = 0;

		quint16 delay_num = 0;
		quint16 delay_den = 1;
		quint8 dop = PNG_DISPOSE_OP_NONE;
		quint8 bop = PNG_BLEND_OP_SOURCE;

		quint64 rowbytes = 0;
		unsigned char * p = nullptr;
		png_bytepp rows = nullptr;
	} _frame;

	QImage _lastImg;

	QList<ApngFrame> _allFrames;

	static void info_fn(png_structp png_ptr, png_infop info_ptr);
	static void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass);
	static void end_fn(png_structp png_ptr, png_infop info_ptr);

	static void frame_info_fn(png_structp png_ptr, png_uint_32 frame_num);
	static void frame_end_fn(png_structp png_ptr, png_uint_32 frame_num);

	bool readChunk(quint32 len = 0);
	void copyOver();
	void blendOver();
};

#endif // APNGREADER_P_H
