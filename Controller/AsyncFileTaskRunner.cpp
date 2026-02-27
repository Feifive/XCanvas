#include "AsyncFileTaskRunner.h"

#include "MessageWidget.h"

#include <QThread>

AsyncFileTaskRunner::AsyncFileTaskRunner(
    QWidget* const hostView,
    FileActionsEnabledChanged onFileActionsEnabledChanged)
    : m_hostView(hostView),
      m_onFileActionsEnabledChanged(std::move(onFileActionsEnabledChanged)),
      m_fileIoThread(new QThread(hostView)),
      m_fileIoContext(new QObject()),
      m_taskRunning(false),
      m_taskMessage(nullptr)
{
    m_fileIoContext->moveToThread(m_fileIoThread);
    QObject::connect(m_fileIoThread, &QThread::finished, m_fileIoContext, &QObject::deleteLater);
    m_fileIoThread->start();
}

AsyncFileTaskRunner::~AsyncFileTaskRunner()
{
    closeLoading();
    if (m_fileIoThread)
    {
        m_fileIoThread->quit();
        m_fileIoThread->wait();
    }
}

bool AsyncFileTaskRunner::isTaskRunning() const
{
    return m_taskRunning;
}

QObject* AsyncFileTaskRunner::workerContext() const
{
    return m_fileIoContext;
}

bool AsyncFileTaskRunner::beginTask(const QString& loadingText)
{
    if (!m_fileIoContext || !m_fileIoThread || m_taskRunning)
    {
        return false;
    }

    m_taskRunning = true;
    if (m_onFileActionsEnabledChanged)
    {
        m_onFileActionsEnabledChanged(false);
    }
    showLoading(loadingText);
    return true;
}

void AsyncFileTaskRunner::endTask()
{
    closeLoading();
    if (m_onFileActionsEnabledChanged)
    {
        m_onFileActionsEnabledChanged(true);
    }
    m_taskRunning = false;
}

void AsyncFileTaskRunner::showLoading(const QString& text)
{
    closeLoading();
    m_taskMessage = MessageWidget::showLoading(m_hostView ? m_hostView->window() : nullptr, text);
}

void AsyncFileTaskRunner::closeLoading()
{
    if (!m_taskMessage)
    {
        return;
    }
    MessageWidget::removeMessage(m_taskMessage);
    m_taskMessage = nullptr;
}
