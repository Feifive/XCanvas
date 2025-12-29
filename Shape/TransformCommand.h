#ifndef TRANSFORMCOMMAND_H
#define TRANSFORMCOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"

namespace xcanvas {
class TransformCommand final : public QUndoCommand {
public:
    TransformCommand(ShapeManager* manager, std::map<Shape*, QTransform> beforeTransform, const QString& cmdText, QUndoCommand* parent = nullptr) :
    QUndoCommand(parent), m_manager(manager), m_beforeTransform(std::move(beforeTransform))
    {
        setText(cmdText);
        for (auto const& [shape, transform] : m_beforeTransform) {
            m_afterTransform[shape] = shape->transform();
        }
    }

    void undo() override {
        for (auto const& [shape, transform] : m_beforeTransform) {
            shape->setTransform(transform);
        }
        m_manager->invalidateSelectedRect();
    }

    void redo() override {
        for (auto const& [shape, transform] : m_afterTransform) {
            shape->setTransform(transform);
        }
        m_manager->invalidateSelectedRect();
    }

private:
    ShapeManager* m_manager;
    std::map<Shape*, QTransform> m_beforeTransform;
    std::map<Shape*, QTransform> m_afterTransform;
};
}

#endif //TRANSFORMCOMMAND_H
