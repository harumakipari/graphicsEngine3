#pragma once


#include <memory>
#include <vector>
#include <functional>

#include "Graphics/Core/Graphics.h"
#include "Graphics/Sprite/Sprite.h"

class UIWidget :public std::enable_shared_from_this <UIWidget>
{
public:
    // コンストラクタ
    UIWidget(const std::string& name) :name_(name), active_(true) {};

    virtual ~UIWidget() {}
    //{
    //    for (auto child : children_)
    //    {
    //        delete child;
    //    }
    //}

    // ファイル設定
    void SetSprite(const std::wstring& filename)
    {
        ID3D11Device* device = Graphics::GetDevice();
        sprite_ = std::make_shared<Sprite>(device, filename.c_str());
    }

    // 位置の設定
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    // サイズの設定
    void SetSize(float w, float h) { width_ = w; height_ = h; }

    // 子Widgetを追加
    void AddChild(std::unique_ptr<UIWidget> child)
    {
        child->parent_ = this;
        children_.push_back(std::move(child));
    }
    //// 子Widgetを追加
    //void AddChild(UIWidget* child)
    //{
    //    child->parent_ = this;
    //    children_.push_back(child);
    //}

    // 親子構造のGetters
    UIWidget* GetParent() const { return parent_; }
    const std::vector<std::unique_ptr<UIWidget>>& GetChildren() const { return children_; }
    //const std::vector<UIWidget*>& GetChildren() const { return children_; }

    // 可視性設定
    void SetVisible(bool visible) { isVisible_ = visible; }
    bool IsVisible() const { return isVisible_; }

    // 座標取得（親の座標を加味した絶対座標）
    float GetAbsoluteX() const
    {

        return parent_ ? parent_->GetAbsoluteX() + x_ : x_;
    }
    float GetAbsoluteY() const
    {
        return parent_ ? parent_->GetAbsoluteY() + y_ : y_;
    }

    virtual void Draw(ID3D11DeviceContext* immediateContext)
    {
        if (!isVisible_)
        {
            return;
        }

        if (sprite_)
        {
            sprite_->Render(immediateContext, x_, y_, width_, height_);
        }

        for (auto& child : children_)
        {
            child->Draw(immediateContext);
        }
    }

    // マウスのヒット判定
    bool HitTest(float mouseX, float mouseY) const
    {
        if (!isVisible_) return false;
        float absX = GetAbsoluteX();
        float absY = GetAbsoluteY();
        return
            (mouseX >= absX && mouseX <= absX + width_ &&
                mouseY >= absY && mouseY <= absY + height_);
    }

    // 入力処理
    virtual bool OnMouseDown(float mouseX, float mouseY)
    {
        if (!isVisible_) return false;

        for (auto it = children_.rbegin(); it != children_.rend(); ++it)
        {
            if ((*it)->HitTest(mouseX, mouseY))
            {
                if ((*it)->OnMouseDown(mouseX, mouseY))
                {
                    //HandleClick();
                    return true;
                }
            }
        }
        if (HitTest(mouseX, mouseY))
        {
            HandleClick();
            return true;
        }
        return false;
    }

    virtual bool OnMouseMove(float mouseX, float mouseY)
    {
        return false;
    }


protected:
    // 画面上の位置とサイズ（スクリーン座標系の矩形）
    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 100.0f;
    float height_ = 30.0f;

    // 子ウィジェットのリスト
    //std::vector<UIWidget*> children_;
    std::vector<std::unique_ptr<UIWidget>> children_;
    // 親Widget（nullptrならルート）
    UIWidget* parent_ = nullptr;

    // 表示状態
    bool isVisible_ = true;

    bool active_ = true;

    std::shared_ptr<Sprite> sprite_;

    std::string name_;

protected:
    // 派生クラスでオーバーライドする
    virtual void HandleClick() {}
};

class UIButton :public UIWidget
{
public:
    enum class State
    {
        Normal, Hovered, Pressed
    };

    UIButton(const std::string& name) :UIWidget(name) {}

    std::function<void()> onClick;

    void Draw(ID3D11DeviceContext* immediateContext) override
    {
        if (!IsVisible()) return;

        DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
        if (state_ == State::Hovered)
        {
            color.w = 0.5f;
        }
        if (sprite_)
        {
            sprite_->Render(immediateContext, GetAbsoluteX(), GetAbsoluteY(), width_, height_, color.x, color.y, color.z, color.w, 0.0f);
        }

        for (auto& child : GetChildren())
            child->Draw(immediateContext);
    }


protected:
    void HandleClick()override
    {
        if (onClick)
        {
            onClick();
        }
    }

    State state_ = State::Normal;
};

class UIRoot
{
public:
    std::unique_ptr<UIWidget> root;

    void Draw(ID3D11DeviceContext* immediateContext)
    {
        if (root)
        {
            root->Draw(immediateContext);
        }
    }

    void OnClick(float mouseX, float mouseY)
    {
        if (root)
        {
            root->OnMouseDown(mouseX, mouseY);
        }
    }
};