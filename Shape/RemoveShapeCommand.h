#ifndef DELETESHAPECOMMAND_H
#define DELETESHAPECOMMAND_H

#include  "UndoCommand.h"
#include  "Shapes.h"

namespace xcanvas {

    class RemoveShapeCommand : public UndoCommand
    {
    public:
        RemoveShapeCommand(Shapes* shapes, Shape* shape) : m_shapes(shapes), m_shape(shape) {}

        void undo() override
        {
            m_shapes->addShape(m_shape);
        }

        void redo() override
        {
            m_shapes->removeShape(m_shape);
        }

    private:
        Shapes* m_shapes;
        Shape*  m_shape;
    };

}


#endif //DELETESHAPECOMMAND_H
