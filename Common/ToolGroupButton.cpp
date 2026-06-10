#include "ToolGroupButton.h"
#include <QActionGroup>
#include <QApplication>
#include <QEnterEvent>
#include <QMouseEvent>

static constexpr int kShowDelayMs = 100;
static constexpr int kHideDelayMs = 200;
static constexpr int kButtonSize  = 36;
static constexpr int kFlyoutGap   = 4;

ToolGroupButton::ToolGroupButton(const QVector<ToolEntry>& tools, QWidget* parent)
    : QWidget(parent), m_tools(tools)
{
    setFixedSize(kButtonSize, kButtonSize);

    m_mainButton = new qfw::TransparentToggleToolButton(this);
    m_mainButton->setFixedSize(kButtonSize, kButtonSize);
    m_mainButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_showTimer.setSingleShot(true);
    m_hideTimer.setSingleShot(true);

    connect(&m_showTimer, &QTimer::timeout, this, &ToolGroupButton::showFlyout);
    connect(&m_hideTimer, &QTimer::timeout, this, [this]() {
        if (!isMouseOverAny())
            hideFlyout();
    });

    connect(m_mainButton, &QToolButton::clicked, this, [this]() {
        m_showTimer.stop();
        hideFlyout();
        if (m_lastUsedTool != DrawingToolType::None)
            emit toolSelected(m_lastUsedTool);
    });

    if (!m_tools.isEmpty())
    {
        m_lastUsedTool = DrawingToolType::Rect;
        updateMainButton();
    }

    m_mainButton->installEventFilter(this);
}

ToolGroupButton::~ToolGroupButton()
{
    removeFlyoutEventFilter();
    delete m_flyout;
}

void ToolGroupButton::setCurrentTool(const DrawingToolType type)
{
    m_currentTool = type;

    bool inGroup = false;
    for (const auto& tool : m_tools)
    {
        if (tool.type == type)
        {
            inGroup        = true;
            m_lastUsedTool = type;
            break;
        }
    }

    m_mainButton->setChecked(inGroup);
    updateMainButton();

    const DrawingToolType highlighted = inGroup ? type : m_lastUsedTool;
    for (int i = 0; i < m_tools.size() && i < m_flyoutActions.size(); ++i)
        m_flyoutActions[i]->setChecked(m_tools[i].type == highlighted);
}

void ToolGroupButton::updateMainButton()
{
    for (const auto& tool : m_tools)
    {
        if (tool.type == m_lastUsedTool)
        {
            m_mainButton->setIcon(XCanvasIcon(tool.iconType));
            m_mainButton->setToolTip(tool.tooltip);
            break;
        }
    }
}

void ToolGroupButton::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    m_hideTimer.stop();
    if (!m_flyoutVisible && !shouldSuppressHoverFlyout())
        m_showTimer.start(kShowDelayMs);
}

void ToolGroupButton::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    resetHoverSuppression();
    m_showTimer.stop();
    if (m_flyoutVisible)
        m_hideTimer.start(kHideDelayMs);
}

bool ToolGroupButton::eventFilter(QObject* obj, QEvent* event)
{
    if (handleMainButtonClickWhileFlyoutVisible(event))
        return true;

    if (obj == m_mainButton)
    {
        if (event->type() == QEvent::Enter)
        {
            m_hideTimer.stop();
            if (!m_flyoutVisible && !shouldSuppressHoverFlyout())
                m_showTimer.start(kShowDelayMs);
        }
        else if (event->type() == QEvent::Leave)
        {
            resetHoverSuppression();
            m_showTimer.stop();
            if (m_flyoutVisible)
                m_hideTimer.start(kHideDelayMs);
        }
        return false;
    }

    if (obj == m_flyout)
    {
        if (event->type() == QEvent::Enter)
        {
            m_hideTimer.stop();
        }
        else if (event->type() == QEvent::Leave)
        {
            if (!rect().contains(mapFromGlobal(QCursor::pos())))
                m_hideTimer.start(kHideDelayMs);
        }
        return false;
    }

    return QWidget::eventFilter(obj, event);
}

bool ToolGroupButton::isMouseOverAny() const
{
    const QPoint gp = QCursor::pos();

    if (m_flyout && m_flyout->isVisible())
    {
        if (m_flyout->geometry().contains(gp))
            return true;
    }

    return QRect(mapToGlobal(QPoint(0, 0)), size()).contains(gp);
}

void ToolGroupButton::createFlyout()
{
    if (m_flyout)
        return;

    m_flyout = new qfw::CheckableMenu(QString(), nullptr,
                                      qfw::MenuIndicatorType::Radio);
    m_flyout->installEventFilter(this);

    connect(m_flyout, &QMenu::aboutToHide, this, [this]() {
        m_flyoutVisible = false;
        removeFlyoutEventFilter();
    });

    m_flyoutActionGroup = new QActionGroup(m_flyout);
    m_flyoutActionGroup->setExclusive(true);

    bool inGroup = false;
    for (const auto& t : m_tools) {
        if (t.type == m_currentTool) { inGroup = true; break; }
    }
    const DrawingToolType highlighted = inGroup ? m_currentTool : m_lastUsedTool;

    for (const auto& tool : m_tools)
    {
        auto* action = new qfw::Action(tool.tooltip, m_flyout);
        action->setIcon(XCanvasIcon(tool.iconType));
        action->setCheckable(true);
        action->setChecked(tool.type == highlighted);

        m_flyout->addAction(action);
        m_flyoutActionGroup->addAction(action);
        m_flyoutActions.append(action);

        connect(action, &QAction::triggered, this, [this, type = tool.type]() {
            m_showTimer.stop();
            hideFlyout();
            emit toolSelected(type);
        });
    }
}

void ToolGroupButton::showFlyout()
{
    createFlyout();
    if (!m_flyout)
        return;

    m_flyoutVisible = true;
    installFlyoutEventFilter();

    const QPoint globalPos =
        mapToGlobal(QPoint(width() + kFlyoutGap, 0));
    m_flyout->execAt(globalPos, true, qfw::MenuAnimationType::FadeInDropDown);
}

void ToolGroupButton::hideFlyout()
{
    if (m_flyout)
        m_flyout->hide();
    m_flyoutVisible = false;
    removeFlyoutEventFilter();
}

void ToolGroupButton::installFlyoutEventFilter()
{
    if (m_appEventFilterInstalled)
        return;

    qApp->installEventFilter(this);
    m_appEventFilterInstalled = true;
}

void ToolGroupButton::removeFlyoutEventFilter()
{
    if (!m_appEventFilterInstalled)
        return;

    qApp->removeEventFilter(this);
    m_appEventFilterInstalled = false;
}

bool ToolGroupButton::handleMainButtonClickWhileFlyoutVisible(QEvent* event)
{
    if (!m_flyoutVisible || event->type() != QEvent::MouseButtonPress)
        return false;

    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() != Qt::LeftButton)
        return false;

    const QRect mainButtonRect(m_mainButton->mapToGlobal(QPoint(0, 0)), m_mainButton->size());
    if (!mainButtonRect.contains(mouseEvent->globalPosition().toPoint()))
        return false;

    m_showTimer.stop();
    m_hoverFlyoutSuppressed = true;
    hideFlyout();
    if (m_lastUsedTool != DrawingToolType::None)
        emit toolSelected(m_lastUsedTool);

    return true;
}

bool ToolGroupButton::shouldSuppressHoverFlyout() const
{
    return m_hoverFlyoutSuppressed;
}

void ToolGroupButton::resetHoverSuppression()
{
    m_hoverFlyoutSuppressed = false;
}
