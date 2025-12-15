#ifndef ADDSHAPESCOMMAND_H
#define ADDSHAPESCOMMAND_H

#include <QUndoCommand>
#include <QVector>
#include "ShapeManager.h"

namespace xcanvas {

    class AddShapesCommand final : public QUndoCommand
    {
    public:
        AddShapesCommand(ShapeManager* shapesManager, const ShapeList &shapesToAdd, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapesManager(shapesManager), m_shapesToAdd(shapesToAdd)
        {
            setText("Add ShapeList");
        }

        ~AddShapesCommand() override
        {
            if (!m_isRedone) 
            {
                for (Shape* shape : m_shapesToAdd)
                {
                    delete shape;
                }
            }
        }

        void redo() override
        {
            m_shapesManager->append(m_shapesToAdd);
            m_isRedone = true;
        }

        void undo() override
        {
            for (Shape* shape : m_shapesToAdd)
            {
                m_shapesManager->removeShape(shape);
            }
            m_isRedone = false;
        }

    private:
    private:
        ShapeManager* m_shapesManager;
        ShapeList     m_shapesToAdd;
        bool m_isRedone = true; 
    };

}

#endif //ADDSHAPESCOMMAND_H
