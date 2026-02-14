#include "MessageWidget.h"
#include <QGraphicsDropShadowEffect>
#include <QSvgRenderer>
#include <QPainter>
#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

QMap<QWidget*, QList<MessageWidget*>> MessageWidget::m_manager;

MessageWidget::MessageWidget(const MessageType type, const QString &text, QWidget *parent)
    : QWidget(parent), m_type(type), m_text(text) {
    setWindowFlags(windowFlags() | Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_DeleteOnClose);

    setupUi();
}

MessageWidget::~MessageWidget() {
    qDebug()<<"~MessageWidget";
}

void MessageWidget::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(0);

    auto *contentWidget = new QWidget(this);
    contentWidget->setAttribute(Qt::WA_StyledBackground, true);
    contentWidget->setStyleSheet("background-color: #FFF; border: none; border-radius: 5px;");
    rootLayout->addWidget(contentWidget);

    auto *layout = new QHBoxLayout(contentWidget);
    auto *iconLabel = new QLabel(contentWidget);
    auto *textLabel = new QLabel(m_text, contentWidget);
    constexpr QSize iconSize(16, 16);

    iconLabel->setFixedSize(iconSize);
    iconLabel->setScaledContents(false);

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);

    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(10);

    QString iconPath;
    switch (m_type) {
        case MessageType::Success: iconPath = ":/Resource/Icons/Success.svg"; break;
        case MessageType::Warning: iconPath = ":/Resource/Icons/Warning.svg"; break;
        case MessageType::Error:   iconPath = ":/Resource/Icons/Error.svg";   break;
        case MessageType::Loading: iconPath = ":/Resource/Icons/Loading.gif"; break;
    }

    if (!iconPath.isEmpty()) {
        if (m_type == MessageType::Loading) {
            auto *movie = new QMovie(iconPath, QByteArray(), iconLabel);
            iconLabel->setAlignment(Qt::AlignCenter);
            QObject::connect(movie, &QMovie::frameChanged, iconLabel, [this, movie, iconLabel, iconSize]() {
                const qreal dpr = devicePixelRatioF();
                const QImage frame = movie->currentImage();
                if (frame.isNull()) return;

                const QImage scaled = frame.scaled(iconSize * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QPixmap pixmap = QPixmap::fromImage(scaled);
                pixmap.setDevicePixelRatio(dpr);
                iconLabel->setPixmap(pixmap);
            });
            movie->start();
        } else {
            QSvgRenderer renderer(iconPath);
            const qreal dpr = devicePixelRatioF();
            QPixmap pixmap(iconSize * dpr);
            pixmap.setDevicePixelRatio(dpr);
            pixmap.fill(Qt::transparent);

            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(iconSize)));

            iconLabel->setPixmap(pixmap);
        }
    }

    auto* shadow = new QGraphicsDropShadowEffect(contentWidget);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0,0,0,50));
    shadow->setBlurRadius(24);
    contentWidget->setGraphicsEffect(shadow);
}

MessageWidget* MessageWidget::showMessage(QWidget *parent, const MessageType type, const QString &text) {
    if (!parent) return nullptr;

    auto *msg = new MessageWidget(type, text, parent);
    msg->adjustSize();

    m_manager[parent].append(msg);

    rearrangeMessages(parent);

    msg->show();

    auto *posAni = new QPropertyAnimation(msg, "pos");
    posAni->setDuration(300);
    posAni->setStartValue(msg->pos());
    posAni->setEndValue(QPoint(msg->x(), msg->y() + 20));
    posAni->setEasingCurve(QEasingCurve::OutBack);
    posAni->start(QAbstractAnimation::DeleteWhenStopped);

    auto *fade = new QPropertyAnimation(msg, "windowOpacity");
    fade->setDuration(300);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->start(QAbstractAnimation::DeleteWhenStopped);

    if (type != MessageType::Loading) {
        QTimer::singleShot(3000, msg, [msg]() {
            removeMessage(msg);
        });
    }

    return msg;
}

MessageWidget* MessageWidget::showLoading(QWidget *parent, const QString &text) {
    return showMessage(parent, MessageType::Loading, text);
}

MessageWidget * MessageWidget::showSuccess(QWidget *parent, const QString &text) {
    return showMessage(parent, MessageType::Success, text);
}

MessageWidget * MessageWidget::showWarning(QWidget *parent, const QString &text) {
    return showMessage(parent, MessageType::Warning, text);
}

MessageWidget * MessageWidget::showError(QWidget *parent, const QString &text) {
    return showMessage(parent, MessageType::Error, text);
}

void MessageWidget::removeMessage(MessageWidget* message) {
    if (!message) return;

    if (QWidget *parent = message->parentWidget(); parent && m_manager.contains(parent)) {
        m_manager[parent].removeOne(message);
        if (m_manager[parent].isEmpty()) {
            m_manager.remove(parent);
        } else {
            rearrangeMessages(parent);
        }
    }

    message->close();
}

void MessageWidget::rearrangeMessages(QWidget* parent) {
    if (!m_manager.contains(parent)) return;

    const QList<MessageWidget*> &list = m_manager[parent];
    const QPoint parentGlobalPos = parent->mapToGlobal(QPoint(0, 0));
    const int startY = parentGlobalPos.y() + 40;

    for (int i = 0; i < list.size(); ++i) {
        MessageWidget *msg = list[i];

        const int targetX = parentGlobalPos.x() + (parent->width() - msg->width()) / 2;
        int targetY = startY;
        for (int j = 0; j < i; ++j) {
            const int spacing = 10;
            targetY += list[j]->height() + spacing;
        }

        const QPoint targetPos(targetX, targetY);

        if (!msg->isVisible()) {
            msg->move(targetX, targetY - 20);
        }

        auto *posAni = new QPropertyAnimation(msg, "pos");
        posAni->setDuration(300);
        posAni->setStartValue(msg->pos());
        posAni->setEndValue(targetPos);
        posAni->setEasingCurve(QEasingCurve::OutCubic);
        posAni->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
