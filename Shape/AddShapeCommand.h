#ifndef ADDSHAPECOMMAND_H
#define ADDSHAPECOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"

namespace xcanvas {

    class AddShapeCommand : public QUndoCommand
    {
    public:
        AddShapeCommand(xcanvas::ShapeManager* shapes, xcanvas::Shape* shape, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapes(shapes), m_shape(shape)
        {
            setText("Add Shape");
        }

        ~AddShapeCommand()
        {
            if (!m_added && m_shape) 
            {
                delete m_shape;
            }
        }

        void redo() override
        {
            m_shapes->addShape(m_shape);
            m_added = true;
        }

        void undo() override
        {
            m_shapes->removeShape(m_shape);
            m_added = false;
        }

    private:
        xcanvas::ShapeManager* m_shapes;
        xcanvas::Shape*  m_shape;
        bool             m_added = false;
    };

}

#endif //ADDSHAPECOMMAND_H
