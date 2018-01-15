#include "tsfile.h"

QString TSFile::fileName() const {
  return m_fileName;
}

QString TSFile::language() const {
  return m_language;
}

QUrl TSFile::urlPath() const {
  return m_urlPath;
}

TSFile::Status TSFile::status() const {
  return m_status;
}

void TSFile::setStatus(const Status &status) {
  m_status = status;
}

xml_document *TSFile::doc() {
  return &m_doc;
}

xml_node *TSFile::rootNode() {
  return &m_tsNode;
}

bool TSFile::saveFile(QString path) {
  return m_doc.save_file(qUtf8Printable(path), "  ", format_indent | format_no_empty_element_tags, encoding_utf8);
}

TSFile::TSFile() {

}

TSFile::TSFile(QUrl urlpath, QObject *parent): QObject(parent) {
  m_urlPath = urlpath;
  m_fileName = urlpath.fileName();
  m_language = "";
  m_status = PAIR_MISSING;
}

TSFile::TSFile(const TSFile &ts) {
  m_urlPath = ts.urlPath();
  m_fileName = ts.fileName();
  m_language = ts.language();
  m_status = ts.status();
}

bool TSFile::loadXml() {
  xml_parse_result result = m_doc.load_file(m_urlPath.toLocalFile().toStdString().c_str(), parse_default | parse_declaration | parse_doctype);

  if(result) {
    m_tsNode = m_doc.child("TS");
    if(!m_tsNode.empty()) {

//      for(xml_node context : tsNode.children("context")) {
//        xml_node name = context.child("name");
//        qDebug() << "NAME: " <<  name.child_value();

////        for(xml_node messag : context.child("message")) {
////          xml_attribute
////        }
//      }

      xml_attribute langAttr = m_tsNode.attribute("language");

      if(!langAttr.empty()) {
        m_language = langAttr.value();
        return true;
      }
      else
        qDebug() << "Could not find language attribute";

    }
    else
    qDebug() << "Could not find ts node";
  }
  else {
//    qDebug() << "Error description: " << result.description();
//    qDebug() << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]";
  }

  return false;
}


TSModel::TSModel(QObject *parent): QAbstractListModel(parent) {

}

int TSModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent)
  return m_files.count();
}

QVariant TSModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= m_files.count())
      return QVariant();

  const TSFile *tsFile = m_files[index.row()];
  if (role == NameRole)
      return tsFile->fileName();
  else if (role == PathRole)
      return tsFile->urlPath();
  else if (role == LanguageRole)
    return tsFile->language();
  else if (role == StatusRole)
    return tsFile->status();

  return QVariant();
}

void TSModel::addTSFile(TSFile *file) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_files << file;
  endInsertRows();
}

void TSModel::reset() {
  beginResetModel();
  m_files.clear();
  endResetModel();
}

int TSModel::findLanguage(QString lang) {
  int i=0;
  for(const TSFile *ts : m_files) {
    if(ts->language() == lang)
      return i;
    i++;
  }

  return -1;
}

void TSModel::changeStatus(int row, TSFile::Status status) {
  if (row >= 0 && row < m_files.count()) {
    QModelIndex index = this->index(row);
    setData(index, status, StatusRole);
    m_files[row]->setStatus(status);
    emit dataChanged(index, index);
  }
}

QHash<int, QByteArray> TSModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole]           = "fileName";
  roles[PathRole]           = "path";
  roles[LanguageRole]       = "language";
  roles[StatusRole]         = "status";

  return roles;
}

QList<TSFile *> TSModel::files() const {
  return m_files;
}

TSFile *TSModel::getFile(int index) const {
  if (index >= 0 && index < m_files.count()) {
    return m_files[index];
  }
}

