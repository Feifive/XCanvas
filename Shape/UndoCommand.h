#ifndef UNDOCOMMAND_H
#define UNDOCOMMAND_H

namespace xcanvas {

    class UndoCommand
    {
    public:
        virtual ~UndoCommand() {}
        virtual void undo() = 0;
        virtual void redo() = 0;
        virtual bool mergeWith(const UndoCommand* other) { return false; }
    };

}

#endif //UNDOCOMMAND_H
