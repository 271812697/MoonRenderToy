#pragma once
#include "Widgets/PropertyQtWidgets.h"
#include <QDragEnterEvent>
#include <QImage>
namespace MOON {

    class TextureDrop : public PropertyQtWidget
    {
        Q_OBJECT
    public:
        explicit TextureDrop(QWidget* parent, WidgetProperty* prop);
        virtual QVariant widgetValue()override;
        virtual void setWidgetValue(const QVariant& value) override;
        // 获取当前贴图路径
        QString currentTexturePath() const { return m_texturePath; }
        // 手动设置贴图
        void setTexture(const QString& path);
        // 清空贴图
        void clearTexture();
    protected:
        // 拖拽事件重写
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void paintEvent(QPaintEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;

    private:
        // 检查是否为图片文件
        bool isImageFile(const QString& path);

    signals:
        // 贴图改变信号（外部绑定这个，就能拿到 Shader 要用的贴图路径）
        void textureChanged(const QString& texturePath);
    private slots:
        void onTextureChanged(const QString& selectText);
    private:
        QImage loadHDR(const QString& path);  
        QImage loadEXR(const QString& path);  
        QImage tonemap(const QImage& hdr);    
    private:
        QString m_texturePath;   // 当前贴图路径
        QPixmap m_displayPixmap; // 当前显示的图片
        bool    m_isEmpty;       // 是否为空
    };


}