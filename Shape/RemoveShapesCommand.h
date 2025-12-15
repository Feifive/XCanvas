#ifndef REMOVESHAPESCOMMAND_H
#define REMOVESHAPESCOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"

namespace xcanvas {

class RemoveShapesCommand : public QUndoCommand
{
public:
    RemoveShapesCommand(ShapeManager* shapesManager, const ShapeList &shapesToRemove, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent), m_shapesManager(shapesManager), m_shapesToRemove(shapesToRemove)
    {
        setText("Remove ShapeList");
    }

    ~RemoveShapesCommand() override
    {
        if (m_isRemoved)
        {
            qDeleteAll(m_shapesToRemove);
        }
    }

    void redo() override
    {
        m_shapesManager->removeAllShapes(m_shapesToRemove);
        m_isRemoved = true;
    }

    void undo() override
    {
        m_shapesManager->append(m_shapesToRemove);
        m_isRemoved = false;
    }

private:
    ShapeManager* m_shapesManager;
    ShapeList     m_shapesToRemove;
    bool m_isRemoved = false;
};

}

#endif //REMOVESHAPESCOMMAND_H
