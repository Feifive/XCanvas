#ifndef REPLACECOMMAND_H
#define REPLACECOMMAND_H

#include <QUndoCommand>
#include "ShapeManager.h"
#include "../Layer/LayerManager.h"

namespace xcanvas {
    class ReplaceShapesCommand final : public QUndoCommand
    {
    public:
        ReplaceShapesCommand(ShapeManager* shapesManager, LayerManager* layerManager, ShapeList beforeShapeList, ShapeList afterShapeList, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapesManager(shapesManager), m_layerManager(layerManager), m_beforeShapeList(std::move(beforeShapeList)), m_afterShapeList(std::move(afterShapeList))
        {
            setText("Replace ShapeList");
        }

        ~ReplaceShapesCommand() override
        {
            if (m_isReplaced) {
                qDeleteAll(m_beforeShapeList);
            }
            else {
                qDeleteAll(m_afterShapeList);
            }
        }

        void redo() override
        {
            m_shapesManager->removeShapes(m_beforeShapeList);
            m_layerManager->removeShapesFromLayer(&m_beforeShapeList);
            m_shapesManager->append(m_afterShapeList);
            m_layerManager->addShapesToLayer(&m_afterShapeList);
            m_isReplaced = true;
        }

        void undo() override
        {
            m_shapesManager->removeShapes(m_afterShapeList);
            m_layerManager->removeShapesFromLayer(&m_afterShapeList);
            m_shapesManager->append(m_beforeShapeList);
            m_layerManager->addShapesToLayer(&m_beforeShapeList);
            m_isReplaced = false;
        }

    private:
        ShapeManager* m_shapesManager;
        LayerManager* m_layerManager;
        ShapeList     m_beforeShapeList;
        ShapeList     m_afterShapeList;
        bool          m_isReplaced = false;
    };
}
#endif //REPLACECOMMAND_H
