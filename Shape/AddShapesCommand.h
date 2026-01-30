#ifndef ADDSHAPESCOMMAND_H
#define ADDSHAPESCOMMAND_H

#include <QUndoCommand>
#include "Shape.h"
#include "ShapeManager.h"
#include "../Layer/LayerManager.h"

namespace xcanvas {

    class AddShapesCommand final : public QUndoCommand
    {
    public:
        AddShapesCommand(ShapeManager* shapesManager, LayerManager* layerManager, ShapeList shapesToAdd, QUndoCommand* parent = nullptr)
            : QUndoCommand(parent), m_shapesManager(shapesManager), m_layerManager(layerManager), m_shapesToAdd(std::move(shapesToAdd))
        {
            setText("Add ShapeList");
        }

        ~AddShapesCommand() override
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
            m_shapesManager->append(m_shapesToAdd);
			m_layerManager->addShapesToLayer(&m_shapesToAdd);
            m_isRedone = true;
        }

        void undo() override
        {
            m_shapesManager->removeShapes(m_shapesToAdd);
			m_layerManager->removeShapesFromLayer(&m_shapesToAdd);
            m_isRedone = false;
        }

    private:
    private:
        ShapeManager* m_shapesManager;
		LayerManager* m_layerManager;
        ShapeList     m_shapesToAdd;
        bool m_isRedone = true; 
    };

}

#endif //ADDSHAPESCOMMAND_H
