#include "Color.h"
// �萔�̒�`
const Color Color::Red = Color(1.0f, 0.0f, 0.0f);
const Color Color::Green = Color(0.0f, 1.0f, 0.0f);
const Color Color::Blue = Color(0.0f, 0.0f, 1.0f);
const Color Color::Yellow = Color(1.0f, 1.0f, 0.0f);
const Color Color::Orange = Color(1.0f, 0.5f, 0.0f);
const Color Color::Purple = Color(1.0f, 0.0f, 1.0f);
const Color Color::White = Color(1.0f, 1.0f, 1.0f);
const Color Color::Black = Color(0.0f, 0.0f, 0.0f);

void Color::ConvertToPastelColors(Color& color) {

	color.r = (color.r + 1) * 0.5f;
	color.g = (color.g + 1) * 0.5f;
	color.b = (color.b + 1) * 0.5f;
}
void Color::DarkenColor(Color& color, float factor)
{
	// �e�������w�肵�������ňÂ�����
	color.r *= factor;
	color.g *= factor;
	color.b *= factor;
	// �A���t�@�l�͕ύX���Ȃ�
}
void Color::HexToRGB(uint32_t hexColor, Color& color, float alpha)
{
	// �Ԑ����𒊏o���A0�`1�͈̔͂ɕϊ�
	color.r = static_cast<float>((hexColor >> 16) & 0xFF) / 255.0f;
	// �ΐ����𒊏o���A0�`1�͈̔͂ɕϊ�
	color.g = static_cast<float>((hexColor >> 8) & 0xFF) / 255.0f;
	// �����𒊏o���A0�`1�͈̔͂ɕϊ�
	color.b = static_cast<float>(hexColor & 0xFF) / 255.0f;
	// �A���t�@�l��ݒ�
	color.a = alpha;
}