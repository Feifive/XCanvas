#ifndef TEXT_H
#define TEXT_H

#include "Shape.h"
#include <QFont>

namespace xcanvas
{
    class Text : public Shape
    {
    public:
        Text();
        virtual ~Text() override;
        void translate(const QPointF& offset) override;
        void setFont(QFont font);
        QFont font() const;
        void setText(const QString& text);
        QString text() const;
        void setPosition(const QPointF& position);
        QPointF position() const;

    protected:
        void updatePainterPath() override;

    private:
        QFont m_font;
        QString m_text;
        QPointF m_position;
    };
}


#endif // TEXT_H
