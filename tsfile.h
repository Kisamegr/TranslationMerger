#ifndef TSFILE_H
#define TSFILE_H

#include <QDebug>
#include <QAbstractListModel>
#include <QUrl>

#include "pugiconfig.hpp"
#include "pugixml.hpp"

using namespace pugi;

class TSFile: public QObject {
  Q_OBJECT

public:
  TSFile();
  TSFile(QUrl urlpath, QObject *parent=0);
  TSFile(const TSFile &ts);
  enum Status{
    PAIR_MISSING,
    DUPLICATE,
    NORMAL
  };

  Q_ENUMS(Status)

  bool loadXml();

  QString path() const;
  QString fileName() const;
  QString language() const;
  QUrl urlPath() const;
  Status status() const;
  void setStatus(const Status &status);
  xml_document *doc();
  xml_node *rootNode();

  bool saveFile(QString path);

private:
  QUrl m_urlPath;
  QString m_fileName;
  QString m_language;
  Status m_status;

  xml_document m_doc;
  xml_node m_tsNode;
};
Q_DECLARE_METATYPE(TSFile *)

class TSModel : public QAbstractListModel {
  Q_OBJECT

  // QAbstractItemModel interface
public:
  enum TSRoles {
    NameRole = Qt::DisplayRole,
    PathRole = Qt::UserRole + 1,
    LanguageRole = Qt::UserRole + 2,
    StatusRole = Qt::UserRole + 3
  };

  TSModel(QObject *parent=0);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  void addTSFile(TSFile *file);
  void reset();


  int findLanguage(QString lang);
  void changeStatus(int row, TSFile::Status status);
  // QAbstractItemModel interface
  QList<TSFile *> files() const;
  TSFile *getFile(int index) const;

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<TSFile*> m_files;
};

#endif // TSFILE_H
