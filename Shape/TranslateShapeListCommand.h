#ifndef TRANSLATESHAPELISTCOMMAND_H
#define TRANSLATESHAPELISTCOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"

namespace xcanvas {

    class TranslateShapesCommand final : public QUndoCommand
    {
    public:
        TranslateShapesCommand(ShapeList shapeList, const QPointF& offset, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapeList(std::move(shapeList)), m_offset(offset)
        {
            setText("Translate ShapeList");
        }

        void redo() override
        {
            for (Shape* shape : m_shapeList) {
                shape->translate(m_offset);
            }
        }

        void undo() override
        {
            for (Shape* shape : m_shapeList) {
                shape->translate(-m_offset);
            }
        }

    private:
        ShapeList m_shapeList;
        QPointF   m_offset;
    };

} // namespace xcanvas

#endif //TRANSLATESHAPELISTCOMMAND_H
