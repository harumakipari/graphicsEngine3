#pragma once


#include <memory>
class Widget :public std::enable_shared_from_this <Widget>
{


protected:
    // 画面上の位置とサイズ（スクリーン座標系の矩形）
    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 100.0f;
    float height_ = 30.0f;
};