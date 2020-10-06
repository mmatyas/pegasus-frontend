// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "BlurhashProvider.h"

#include <QHash>

#include <array>
#include <cmath>


namespace {
constexpr std::array<QChar, 83> BASE83 {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '#', '$', '%', '*', '+', ',',
    '-', '.', ':', ';', '=', '?', '@', '[', ']', '^', '_', '{', '|', '}', '~',
};
constexpr int BLURHASH_MIN_LEN = 6;


struct FpColor {
    float r;
    float g;
    float b;
};


unsigned decode_base83(QStringView str)
{
    static const QHash<QChar, unsigned int> BASE83_MAP = [](){
        QHash<QChar, unsigned int> out;
        out.reserve(BASE83.size());
        for (size_t i = 0; i < BASE83.size(); i++)
            out.insert(BASE83[i], i);
        return out;
    }();

    unsigned int result = 0;
    for (const QChar ch : str) {
        const auto it = BASE83_MAP.find(ch);
        if (it != BASE83_MAP.cend()) {
            result *= BASE83.size();
            result += *it;
        }
    }
    return result;
}


float srgb_to_linear(uint8_t srgb_val)
{
    // NOTE: See "sRGB reverse transformation"
    const float u = srgb_val / 255.f;
    return u <= 0.4045f
        ? u / 12.92f
        : std::pow((u + 0.055f) / 1.055f, 2.4f);
}


uint8_t linear_to_srgb(float linear_val)
{
    // NOTE: See "sRGB forward transformation"
    const float u = std::max(0.f, std::min(linear_val, 1.f));
    const float g = u <= 0.0031308f
        ? u * 12.92
        : 1.055 * std::pow(u, 1.f / 2.4f) - 0.055f;
    return std::round(g * 255.f + 0.5f);
}


float unquant_ac_component(float quant, float max_ac)
{
    const float base = quant - 9.f;
    return std::copysign(1.f, base) * std::pow(base / 9.f, 2.f) * max_ac;
}


FpColor decode_dc(unsigned raw_val)
{
    const float b = srgb_to_linear(raw_val & 0xFF);
    raw_val >>= 8;
    const float g = srgb_to_linear(raw_val & 0xFF);
    raw_val >>= 8;
    const float r = srgb_to_linear(raw_val & 0xFF);
    return { r, g, b };
}


FpColor decode_ac(unsigned raw_val, float max_ac)
{
    const unsigned quant_r = raw_val / (19 * 19);
    const unsigned quant_g = (raw_val / 19) % 19;
    const unsigned quant_b = raw_val % 19;

    const float r = unquant_ac_component(quant_r, max_ac);
    const float g = unquant_ac_component(quant_g, max_ac);
    const float b = unquant_ac_component(quant_b, max_ac);
    return { r, g, b };
}


std::vector<float> create_cos_table(unsigned components, unsigned image_dim)
{
    std::vector<float> out(components * image_dim);
    for (size_t i = 0; i < out.size(); i++)
        out[i] = std::cos(M_PI * i / image_dim);
    return out;
}
} // namespace


BlurhashProvider::BlurhashProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{}


QImage BlurhashProvider::requestImage(const QString& hash_url, QSize* out_size, const QSize& requested_size)
{
    const QString hash = QUrl::fromPercentEncoding(hash_url.toLatin1());
    if (hash.length() < BLURHASH_MIN_LEN)
        return {};

    const QSize img_size = requested_size.isEmpty()
        ? QSize(24, 24)
        : requested_size;

    const unsigned components_raw = decode_base83(hash.leftRef(1));
    const unsigned components_x = (components_raw % 9) + 1;
    const unsigned components_y = (components_raw / 9) + 1;
    const size_t color_cnt = components_x * components_y;
    if (static_cast<unsigned>(hash.length()) != 4 + 2 * color_cnt) // 2 head + 4 DC + 2 * (nx * ny - 1) AC
        return {};

    const unsigned max_ac_raw = decode_base83(hash.midRef(1, 1));
    const float max_ac = (max_ac_raw + 1) / 166.f;

    const std::vector<FpColor> colors = [color_cnt, max_ac, &hash](){
        std::vector<FpColor> out;
        out.reserve(color_cnt);

        const unsigned avg_color_raw = decode_base83(hash.midRef(2, 4));
        out.emplace_back(decode_dc(avg_color_raw));

        for (size_t i = 1; i < color_cnt; i++) {
            const int str_start = 4 + i * 2;
            const unsigned color_raw = decode_base83(hash.midRef(str_start, 2));
            out.emplace_back(decode_ac(color_raw, max_ac));
        }

        return out;
    }();

    const std::vector<float> cos_x_table = create_cos_table(components_x, img_size.width());
    const std::vector<float> cos_y_table = create_cos_table(components_y, img_size.height());

    QImage out_img(img_size, QImage::Format_RGB888);
    uchar* const out_img_data = out_img.bits();

    for (int img_y = 0; img_y < img_size.height(); img_y++) {
        for (int img_x = 0; img_x < img_size.width(); img_x++) {
            FpColor out_color { 0, 0, 0 };

            for (unsigned cy = 0; cy < components_y; cy++) {
                for (unsigned cx = 0; cx < components_x; cx++) {
                    const float basis = cos_x_table[img_x * cx] * cos_y_table[img_y * cy];
                    const size_t color_idx = cy * components_x + cx;
                    const FpColor color = colors[color_idx];

                    out_color.r += color.r * basis;
                    out_color.g += color.g * basis;
                    out_color.b += color.b * basis;
                }
            }

            const size_t pixeldata_idx = (img_y * img_size.width() + img_x) * 3;
            out_img_data[pixeldata_idx + 0] = linear_to_srgb(out_color.r);
            out_img_data[pixeldata_idx + 1] = linear_to_srgb(out_color.g);
            out_img_data[pixeldata_idx + 2] = linear_to_srgb(out_color.b);
        }
    }

    if (out_size)
        *out_size = img_size;
    return out_img;
}
