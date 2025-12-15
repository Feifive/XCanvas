#ifndef DELETESHAPECOMMAND_H
#define DELETESHAPECOMMAND_H

#include  <QUndoCommand>
#include  "ShapeManager.h"

namespace xcanvas {

    class RemoveShapeCommand : public QUndoCommand
    {
    public:
        RemoveShapeCommand(xcanvas::ShapeManager* shapes, xcanvas::Shape* shape, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapes(shapes), m_shape(shape)
        {
            setText("Remove Shape");
        }

        ~RemoveShapeCommand()
        {
            if (m_removed && m_shape) 
            {
                delete m_shape;
            }
        }

        void redo() override
        {
            m_shapes->removeShape(m_shape);
            m_removed = true;
        }

        void undo() override
        {
            m_shapes->addShape(m_shape);
            m_removed = false;
        }

    private:
        xcanvas::ShapeManager* m_shapes;
        xcanvas::Shape*  m_shape;
        bool             m_removed = false;
    };
}


#endif //DELETESHAPECOMMAND_H
