#ifndef ADDSHAPECOMMAND_H
#define ADDSHAPECOMMAND_H

#include "UndoCommand.h"
#include "Shapes.h"

namespace xcanvas {

    class AddShapeCommand : public UndoCommand
    {
    public:
        AddShapeCommand(Shapes* shapes, Shape* shape) : m_shapes(shapes), m_shape(shape) {}

        void undo() override
        {
            m_shapes->removeShape(m_shape);
        }

        void redo() override
        {
            m_shapes->addShape(m_shape);
        }

    private:
        Shapes* m_shapes;
        Shape*  m_shape;
    };

}

#endif //ADDSHAPECOMMAND_H
