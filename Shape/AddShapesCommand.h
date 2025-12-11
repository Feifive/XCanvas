#ifndef ADDSHAPESCOMMAND_H
#define ADDSHAPESCOMMAND_H

#include <QUndoCommand>
#include "Shapes.h"

namespace xcanvas {

    class AddShapesCommand : public QUndoCommand
    {
    public:
        AddShapesCommand(xcanvas::Shapes* shapesManager, xcanvas::Shapes* newShapes, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_manager(shapesManager), m_newShapes(newShapes)
        {
            setText("Add Shapes");
        }

        ~AddShapesCommand()
        {
            if (!m_isRedone) 
            {
                m_newShapes->clear();
				delete m_newShapes;
            }
        }

        void redo() override
        {
            m_manager->append(m_newShapes->shapes());
            m_isRedone = true;
        }

        void undo() override
        {
            for (Shape* shape : m_newShapes->shapes()) 
            {
                m_manager->removeShape(shape);
            }
            m_isRedone = false;
        }

    private:
    private:
        Shapes* m_manager;
        Shapes* m_newShapes;
        bool m_isRedone = true; 
    };

}

#endif //ADDSHAPESCOMMAND_H
