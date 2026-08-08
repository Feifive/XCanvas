#include "Controller/TextEditCommit.h"
#include "Shape/EditTextCommand.h"
#include "Shape/ShapeManager.h"
#include "Shape/ShapeText.h"

#include <QUndoStack>
#include <QtTest>

class tst_TextEditCommit final : public QObject
{
    Q_OBJECT

  private slots:
    void unchangedTextDoesNotEnterUndoStack();
    void changedTextUndoRestoresOriginal();
};

void tst_TextEditCommit::unchangedTextDoesNotEnterUndoStack()
{
    const TextEditCommit edit{QStringLiteral("original"), QStringLiteral("original")};
    QUndoStack stack;

    if (edit.shouldPush(true))
    {
        stack.push(new QUndoCommand());
    }

    QCOMPARE(stack.count(), 0);
}

void tst_TextEditCommit::changedTextUndoRestoresOriginal()
{
    xcanvas::ShapeManager manager;
    xcanvas::ShapeText shape;
    shape.setText(QStringLiteral("original"));
    const TextEditCommit edit{shape.text(), QStringLiteral("edited")};
    QUndoStack stack;

    QVERIFY(edit.shouldPush(true));
    stack.push(new xcanvas::EditTextCommand(
        &manager, &shape, edit.originalText, edit.editedText));
    QCOMPARE(shape.text(), QStringLiteral("edited"));

    stack.undo();
    QCOMPARE(shape.text(), QStringLiteral("original"));
}

QTEST_APPLESS_MAIN(tst_TextEditCommit)
#include "tst_TextEditCommit.moc"
