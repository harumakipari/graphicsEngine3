#pragma once


#include <memory>
class Widget :public std::enable_shared_from_this <Widget>
{


protected:
    // ��ʏ�̈ʒu�ƃT�C�Y�i�X�N���[�����W�n�̋�`�j
    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 100.0f;
    float height_ = 30.0f;
};