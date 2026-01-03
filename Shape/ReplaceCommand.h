#ifndef REPLACECOMMAND_H
#define REPLACECOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"

namespace xcanvas {
    class ReplaceShapesCommand final : public QUndoCommand
    {
    public:
        ReplaceShapesCommand(ShapeManager* shapesManager, ShapeList beforeShapeList, ShapeList afterShapeList, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapesManager(shapesManager), m_beforeShapeList(std::move(beforeShapeList)), m_afterShapeList(std::move(afterShapeList))
        {
            setText("Replace ShapeList");
        }

        ~ReplaceShapesCommand() override
        {
            if (m_isReplaced) {
                qDeleteAll(m_beforeShapeList);
            }
            else {
                qDeleteAll(m_afterShapeList);
            }
        }

        void redo() override
        {
            m_shapesManager->removeShapes(m_beforeShapeList);
            m_shapesManager->append(m_afterShapeList);
            m_isReplaced = true;
        }

        void undo() override
        {
            m_shapesManager->removeShapes(m_afterShapeList);
            m_shapesManager->append(m_beforeShapeList);
            m_isReplaced = false;
        }

    private:
        ShapeManager* m_shapesManager;
        ShapeList     m_beforeShapeList;
        ShapeList     m_afterShapeList;
        bool          m_isReplaced = false;
    };
}
#endif //REPLACECOMMAND_H
