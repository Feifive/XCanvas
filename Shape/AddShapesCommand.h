#ifndef ADDSHAPESCOMMAND_H
#define ADDSHAPESCOMMAND_H

#include <QUndoCommand>
#include <QVector>
#include "ShapeManager.h"

namespace xcanvas {

    class AddShapesCommand : public QUndoCommand
    {
    public:
        AddShapesCommand(ShapeManager* shapesManager, ShapeList shapesToAdd, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_manager(shapesManager), m_shapesToAdd(shapesToAdd)
        {
            setText("Add ShapeList");
        }

        ~AddShapesCommand()
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
            m_manager->append(m_shapesToAdd);
            m_isRedone = true;
        }

        void undo() override
        {
            for (Shape* shape : m_shapesToAdd)
            {
                m_manager->removeShape(shape);
            }
            m_isRedone = false;
        }

    private:
    private:
        ShapeManager* m_manager;
        ShapeList     m_shapesToAdd;
        bool m_isRedone = true; 
    };

}

#endif //ADDSHAPESCOMMAND_H
