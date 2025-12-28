#ifndef TRANSFORMCOMMAND_H
#define TRANSFORMCOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"

namespace xcanvas {
    class TransformCommand : public QUndoCommand {
        public:
        TransformCommand(ShapeManager* manager, std::map<Shape*, std::unique_ptr<ShapeState>> beforeStates, const QString& cmdText, QUndoCommand* parent = nullptr) :
        m_manager(manager), m_beforeStates(std::move(beforeStates)){
            setText(cmdText);
            for (auto const& [shape, state] : m_beforeStates) {
                if (shape) {
                    m_afterStates[shape] = shape->createSnapshot();
                }
            }
        }

        void undo() override {
            for (auto const& [shape, state] : m_beforeStates) {
                if (shape && state) {
                    shape->restoreSnapshot(state.get());
                }
            }
            m_manager->invalidateSelectedRect();
        }

        void redo() override {
            for (auto const& [shape, state] : m_afterStates) {
                if (shape && state) {
                    shape->restoreSnapshot(state.get());
                }
            }
            m_manager->invalidateSelectedRect();
        }

        private:
        ShapeManager* m_manager;
        std::map<Shape*, std::unique_ptr<ShapeState>> m_beforeStates;
        std::map<Shape*, std::unique_ptr<ShapeState>> m_afterStates;
    };
}

#endif //TRANSFORMCOMMAND_H
