#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include "tsfile.h"

class DocumentManager: public QObject {
  Q_OBJECT

private:
  TSModel *m_newModel;
  TSModel *m_transModel;


public:
  DocumentManager();

  static DocumentManager &sharedManager();

  Q_INVOKABLE void addFiles(const QList<QUrl> paths, const bool translated);
  Q_INVOKABLE void mergeFiles(const QString outputPath);

  TSModel *newModel() const;
  TSModel *transModel() const;
};

#endif // DOCUMENTMANAGER_H
