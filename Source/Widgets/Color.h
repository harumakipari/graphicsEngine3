#pragma once
#include <DirectXMath.h>
using namespace DirectX;
//�J���[�iXMFLOAT4�ƌ݊�������j
struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color(const Color&) = default;
	Color& operator=(const Color&) = default;

	Color(Color&&) = default;
	Color& operator=(Color&&) = default;

	constexpr Color(float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f) noexcept : r(r), g(g), b(b), a(a) {}
	explicit constexpr Color(const XMFLOAT4& color) : r(color.x), g(color.y), b(color.z), a(color.w) {}

	explicit Color(_In_reads_(4) const float* pArray) noexcept : r(pArray[0]), g(pArray[1]), b(pArray[2]), a(pArray[3]) {}

	operator XMFLOAT4() const { return XMFLOAT4(r, g, b, a); }

	//Color operator=(const XMFLOAT4& color) { return { color.x,color.y,color.z,color.w }; }
	Color& operator+=(const Color& c) { r += c.r, g += c.g, b += c.b, a += c.a; return *this; }
	Color& operator-=(const Color& c) { r -= c.r, g -= c.g, b -= c.b, a -= c.a; return *this; }
	Color& operator*=(const Color& c) { r *= c.r, g *= c.g, b *= c.b, a *= c.a; return *this; }
	Color& operator/=(const Color& c) { r /= c.r, g /= c.g, b /= c.b, a /= c.a; return *this; }
	Color& operator*=(float scale) { r *= scale, g *= scale, b *= scale, a *= scale; return *this; }
	Color& operator/=(float scale) { r /= scale, g /= scale, b /= scale, a /= scale; return *this; }

	Color operator+(const Color& c) { return { this->r + c.r, this->g + c.g, this->b + c.b, this->a + c.a }; }
	Color operator-(const Color& c) { return { this->r - c.r, this->g - c.g, this->b - c.b, this->a - c.a }; }
	Color operator*(const Color& c) { return { this->r * c.r, this->g * c.g, this->b * c.b, this->a * c.a }; }
	Color operator/(const Color& c) { return { this->r / c.r, this->g / c.g, this->b / c.b, this->a / c.a }; }
	Color operator*(float scale) { return { this->r * scale, this->g * scale, this->b * scale, this->a * scale }; }
	Color operator/(float scale) { return { this->r / scale, this->g / scale, this->b / scale, this->a / scale }; }
	bool operator==(Color& c) { return (r == c.r && g == c.g && b == c.b && a == c.a); }
	bool operator!=(Color& c) { return (r != c.r || g != c.g || b != c.b || a != c.a); }
	bool operator==(const Color& c) const { return (r == c.r && g == c.g && b == c.b && a == c.a); }
	bool operator!=(const Color& c) const { return (r != c.r || g != c.g || b != c.b || a != c.a); }

	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Orange;
	static const Color Purple;
	static const Color White;
	static const Color Black;

	static void ConvertToPastelColors(Color& color);
	/**
	 * @brief 16�i���J���[�R�[�h�i0xRRGGBB�j��RGB�i0�`1�j�ɕϊ�����
	 * @param hexColor 0xRRGGBB�`���̃J���[�R�[�h
	 * @param color �ϊ����Color�\���́ir, g, b, a�j
	 * @param alpha �A���t�@�l�i�ȗ�����1.0�j
	 */
	static void HexToRGB(uint32_t hexColor, Color& color, float alpha = 1.f);
	/**
	* @brief �F�������Â�����
	* @param color �Ώۂ̐F�i0�`1�͈̔́j
	* @param factor �Â����銄���i0.0�`1.0�A�f�t�H���g��0.8��20%�Â�����j
	*/
	static void DarkenColor(Color& color, float factor = 0.8f);
};
