#ifndef ROTATESHAPESCOMMAND_H
#define ROTATESHAPESCOMMAND_H

#include <QUndoCommand>
#include <utility>
#include "ShapeManager.h"

namespace xcanvas {
    class RotateShapesCommand final : public QUndoCommand
    {
    public:
        RotateShapesCommand(ShapeList shapeList, const double angle, QPointF rotationCenter, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapeList(std::move(shapeList)), m_angle(angle), m_rotationCenter(std::move(rotationCenter))
        {
            setText("Rotate Shapes");
        }

        void undo() override {
            for (Shape* shape : m_shapeList) {
                shape->rotate(-m_angle, m_rotationCenter);
            }
        }

        void redo() override {
            for (Shape* shape : m_shapeList) {
                shape->rotate(m_angle, m_rotationCenter);
            }
        }

    private:
        ShapeList m_shapeList;
        double m_angle;
        QPointF m_rotationCenter;
    };
}

#endif //ROTATESHAPESCOMMAND_H
