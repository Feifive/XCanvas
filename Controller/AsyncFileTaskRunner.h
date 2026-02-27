#ifndef ASYNCFILETASKRUNNER_H
#define ASYNCFILETASKRUNNER_H

#include <functional>

#include <QString>

class MessageWidget;
class QObject;
class QThread;
class QWidget;

class AsyncFileTaskRunner final
{
  public:
    using FileActionsEnabledChanged = std::function<void(bool)>;

    AsyncFileTaskRunner(QWidget* hostView, FileActionsEnabledChanged onFileActionsEnabledChanged);
    ~AsyncFileTaskRunner();

    bool isTaskRunning() const;
    QObject* workerContext() const;

    bool beginTask(const QString& loadingText);
    void endTask();

  private:
    void showLoading(const QString& text);
    void closeLoading();

  private:
    QWidget*                  m_hostView;
    FileActionsEnabledChanged m_onFileActionsEnabledChanged;
    QThread*                  m_fileIoThread;
    QObject*                  m_fileIoContext;
    bool                      m_taskRunning;
    MessageWidget*            m_taskMessage;
};

#endif// ASYNCFILETASKRUNNER_H
