#ifndef XCANVAS_MESSAGEBOX_H
#define XCANVAS_MESSAGEBOX_H

#include <QDialog>

class QLabel;
class QPushButton;
class QToolButton;
class QWidget;

class MessageBox : public QDialog
{
    Q_OBJECT

public:
    enum class ClickedButton
    {
        Primary,
        Secondary,
        Closed
    };

    explicit MessageBox(QWidget* parent = nullptr);

    void setTitleText(const QString& text) const;
    void setContentText(const QString& text) const;
    void setPrimaryButtonText(const QString& text) const;
    void setSecondaryButtonText(const QString& text) const;

    ClickedButton clickedButton() const;

    static ClickedButton ask(
        QWidget* parent,
        const QString& title,
        const QString& content,
        const QString& primaryText = QObject::tr("确定"),
        const QString& secondaryText = QObject::tr("取消"));

    static ClickedButton Confirm(
        QWidget* parent,
        const QString& title,
        const QString& content);

    static ClickedButton Info(
        QWidget* parent,
        const QString& title,
        const QString& content);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void onPrimaryClicked();
    void onSecondaryClicked();
    void onCloseClicked();
    void setCloseButtonVisible(bool visible) const;
    void setSecondaryButtonVisible(bool visible) const;
    void updatePanelWidth() const;

private:
    QWidget* m_panel;
    QLabel* m_titleLabel;
    QLabel* m_contentLabel;
    QToolButton* m_closeButton;
    QPushButton* m_secondaryButton;
    QPushButton* m_primaryButton;
    ClickedButton m_clickedButton;
};

#endif //XCANVAS_MESSAGEBOX_H
