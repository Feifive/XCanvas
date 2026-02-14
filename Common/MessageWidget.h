#ifndef XCANVAS_MESSAGEWIDGET_H
#define XCANVAS_MESSAGEWIDGET_H

// Message 全局提示
// 可提供成功、警告、错误和异步加载中等反馈信息。
// 顶部居中显示并自动消失，是一种不打断用户操作的轻量级提示方式。

#include  <QWidget>

enum class MessageType {Success, Warning, Error, Loading };

class MessageWidget : public QWidget {
    Q_OBJECT
public:
    explicit MessageWidget(MessageType type, const QString &text, QWidget *parent = nullptr);
    ~MessageWidget() override;
    static MessageWidget* showMessage(QWidget *parent, MessageType type, const QString &text);
    static MessageWidget* showLoading(QWidget *parent, const QString &text);
    static MessageWidget* showSuccess(QWidget *parent, const QString &text);
    static MessageWidget* showWarning(QWidget *parent, const QString &text);
    static MessageWidget* showError(QWidget *parent, const QString &text);
    static void removeMessage(MessageWidget* message);

private:
    void setupUi();
    static void rearrangeMessages(QWidget* parent);

private:
    static QMap<QWidget*, QList<MessageWidget*>> m_manager; // 追踪每个父窗口的消息列表
    MessageType m_type;
    QString m_text;
};

#endif //XCANVAS_MESSAGEWIDGET_H
