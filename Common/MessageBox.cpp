#include "MessageBox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QShowEvent>
#include <QSvgRenderer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

MessageBox::MessageBox(QWidget* parent)
    : QDialog(parent),
      m_panel(nullptr),
      m_titleLabel(nullptr),
      m_contentLabel(nullptr),
      m_closeButton(nullptr),
      m_secondaryButton(nullptr),
      m_primaryButton(nullptr),
      m_clickedButton(ClickedButton::Closed)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setModal(true);

    auto* overlay = new QWidget(this);
    overlay->setObjectName("MessageBoxOverlay");
    overlay->setAttribute(Qt::WA_StyledBackground, true);
    overlay->setStyleSheet(R"(
        #MessageBoxOverlay {
            background: rgba(0, 0, 0, 0.35);
        }
    )");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(overlay);

    auto* overlayLayout = new QVBoxLayout(overlay);
    overlayLayout->addStretch();

    m_panel = new QWidget(overlay);
    m_panel->setObjectName("MessageBoxPanel");
    m_panel->setAttribute(Qt::WA_StyledBackground, true);
    m_panel->setStyleSheet(
        "#MessageBoxPanel {"
        "  background-color: #ffffff;"
        "  border-radius: 14px;"
        "}");
    overlayLayout->addWidget(m_panel, 0, Qt::AlignHCenter);
    overlayLayout->addStretch();

    auto* panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(20, 14, 20, 20);
    panelLayout->setSpacing(8);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    auto* titleIconLabel = new QLabel(m_panel);
    constexpr QSize iconSize(18, 18);
    titleIconLabel->setFixedSize(iconSize);
    {
        QSvgRenderer renderer(QStringLiteral(":/Resource/Icons/Warning.svg"));
        const qreal dpr = devicePixelRatioF();
        QPixmap pixmap(iconSize * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(iconSize)));
        titleIconLabel->setPixmap(pixmap);
    }
    headerLayout->addWidget(titleIconLabel, 0, Qt::AlignVCenter);

    m_titleLabel = new QLabel(tr("提示"), m_panel);
    m_titleLabel->setStyleSheet("color: #202124; font-size: 17px; font-weight: 600;");
    headerLayout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    headerLayout->addStretch();

    m_closeButton = new QToolButton(m_panel);
    m_closeButton->setText(QStringLiteral("✕"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAutoRaise(true);
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setStyleSheet(
        "QToolButton {"
        "  border: none;"
        "  color: #252c36;"
        "  font-size: 16px;"
        "  font-weight: 500;"
        "}");
    connect(m_closeButton, &QToolButton::clicked, this, &MessageBox::onCloseClicked);
    headerLayout->addWidget(m_closeButton, 0, Qt::AlignTop);

    panelLayout->addLayout(headerLayout);

    m_contentLabel = new QLabel(m_panel);
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setMinimumHeight(26);
    m_contentLabel->setStyleSheet("color: #202124; font-size: 13px;");
    panelLayout->addWidget(m_contentLabel);

    auto* footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->addStretch();
    footerLayout->setSpacing(6);

    m_secondaryButton = new QPushButton(tr("取消"), m_panel);
    m_secondaryButton->setMinimumWidth(65);
    m_secondaryButton->setCursor(Qt::PointingHandCursor);
    m_secondaryButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #FFF;"
        "  color: #222222;"
        "  border: 1px solid #d9d9d9;"
        "  border-radius: 7px;"
        "  font-size: 13px;"
        "  font-weight: 500;"
        "  padding: 5px 10px 5px 10px;"
        "}"
        "QPushButton:hover { background-color: #f6f7f9; }"
        "QPushButton:pressed { background-color: #efeff3; }");
    connect(m_secondaryButton, &QPushButton::clicked, this, &MessageBox::onSecondaryClicked);
    footerLayout->addWidget(m_secondaryButton);

    m_primaryButton = new QPushButton(tr("确定"), m_panel);
    m_primaryButton->setMinimumWidth(65);
    m_primaryButton->setCursor(Qt::PointingHandCursor);
    m_primaryButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #060b10;"
        "  color: #ffffff;"
        "  border: 1px solid #060b10;"
        "  border-radius: 7px;"
        "  font-size: 13px;"
        "  font-weight: 500;"
        "  padding: 5px 10px 5px 10px;"
        "}"
        "QPushButton:hover { background-color: #252c36; }"
        "QPushButton:pressed { background-color: #1c2129; }");
    connect(m_primaryButton, &QPushButton::clicked, this, &MessageBox::onPrimaryClicked);
    footerLayout->addWidget(m_primaryButton);

    panelLayout->addLayout(footerLayout);
}

void MessageBox::setTitleText(const QString& text) const {
    m_titleLabel->setText(text);
}

void MessageBox::setContentText(const QString& text) const {
    m_contentLabel->setText(text);
}

void MessageBox::setPrimaryButtonText(const QString& text) const {
    m_primaryButton->setText(text);
}

void MessageBox::setSecondaryButtonText(const QString& text) const {
    m_secondaryButton->setText(text);
}

MessageBox::ClickedButton MessageBox::clickedButton() const
{
    return m_clickedButton;
}

MessageBox::ClickedButton MessageBox::ask(
    QWidget* parent,
    const QString& title,
    const QString& content,
    const QString& primaryText,
    const QString& secondaryText)
{
    MessageBox messageBox(parent);
    messageBox.setTitleText(title);
    messageBox.setContentText(content);
    messageBox.setPrimaryButtonText(primaryText);
    messageBox.setSecondaryButtonText(secondaryText);
    messageBox.exec();
    return messageBox.clickedButton();
}

MessageBox::ClickedButton MessageBox::Confirm(
    QWidget* parent,
    const QString& title,
    const QString& content)
{
    MessageBox messageBox(parent);
    messageBox.setTitleText(title);
    messageBox.setContentText(content);
    messageBox.setPrimaryButtonText(tr("确认"));
    messageBox.setSecondaryButtonText(tr("取消"));
    messageBox.setCloseButtonVisible(false);
    messageBox.setSecondaryButtonVisible(true);
    messageBox.exec();
    return messageBox.clickedButton();
}

MessageBox::ClickedButton MessageBox::Info(
    QWidget* parent,
    const QString& title,
    const QString& content)
{
    MessageBox messageBox(parent);
    messageBox.setTitleText(title);
    messageBox.setContentText(content);
    messageBox.setPrimaryButtonText(QObject::tr("知道了"));
    messageBox.setCloseButtonVisible(false);
    messageBox.setSecondaryButtonVisible(false);
    messageBox.exec();
    return messageBox.clickedButton();
}

void MessageBox::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    if (const QWidget* topLevel = parentWidget() ? parentWidget()->window() : nullptr)
    {
        setGeometry(topLevel->geometry());
    }
    else
    {
        resize(1200, 700);
    }

    updatePanelWidth();
}

void MessageBox::onPrimaryClicked()
{
    m_clickedButton = ClickedButton::Primary;
    done(QDialog::Accepted);
}

void MessageBox::onSecondaryClicked()
{
    m_clickedButton = ClickedButton::Secondary;
    done(QDialog::Rejected);
}

void MessageBox::onCloseClicked()
{
    m_clickedButton = ClickedButton::Closed;
    done(QDialog::Rejected);
}

void MessageBox::setCloseButtonVisible(const bool visible) const
{
    if (m_closeButton)
    {
        m_closeButton->setVisible(visible);
    }
}

void MessageBox::setSecondaryButtonVisible(const bool visible) const
{
    if (m_secondaryButton)
    {
        m_secondaryButton->setVisible(visible);
    }
}

void MessageBox::updatePanelWidth() const {
    if (!m_panel)
    {
        return;
    }

    constexpr int minWidth = 340;
    constexpr int maxWidth = 580;
    const int availableWidth = width() - 160;
    const int panelWidth = qBound(minWidth, availableWidth, maxWidth);
    m_panel->setFixedWidth(panelWidth);
}
