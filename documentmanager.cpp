#include "documentmanager.h"
#include <QQmlEngine>
#include <QDebug>
#include <QDir>

QObject *documentManagerFactory(QQmlEngine *, QJSEngine *)
{
  QObject *manager = &DocumentManager::sharedManager();
  QQmlEngine::setObjectOwnership(manager, QQmlEngine::CppOwnership);
  return manager;
}

TSModel *DocumentManager::newModel() const {
  return m_newModel;
}

TSModel *DocumentManager::transModel() const {
  return m_transModel;
}

DocumentManager::DocumentManager() {
  m_newModel = new TSModel(this);
  m_transModel = new TSModel(this);

  qmlRegisterSingletonType<DocumentManager>("TranslationMerge", 1, 0, "DocumentManager", &documentManagerFactory);
  qmlRegisterUncreatableType<TSFile>("TranslationMerge", 1, 0, "TSFile", "Cant Create Mate");

}

DocumentManager &DocumentManager::sharedManager() {
  static DocumentManager theManager;
  return theManager;
}

void DocumentManager::addFiles(const QList<QUrl> paths, const bool translated) {

  TSModel *model = translated ? m_transModel : m_newModel;
  TSModel *otherModel = !translated ? m_transModel : m_newModel;
  model->reset();

  for(QUrl tsUrl : paths) {
    TSFile *tsFile = new TSFile(tsUrl, model);
    if(tsFile->loadXml()) {
      // Check if duplicate
      int index = model->findLanguage(tsFile->language());

      if(index == -1) {
        int otherIndex = otherModel->findLanguage(tsFile->language());

        if(otherIndex != -1) {
          tsFile->setStatus(TSFile::NORMAL);
          otherModel->changeStatus(otherIndex, TSFile::NORMAL);
        }

      }
      else {
        tsFile->setStatus(TSFile::DUPLICATE);
        model->changeStatus(index, TSFile::DUPLICATE);
      }

      model->addTSFile(tsFile);
    }
  }

}

void DocumentManager::mergeFiles(const QString outputPath) {

  qDebug() << "Starting merge...";

  for(TSFile *newFile : newModel()->files()) {
    TSFile *transFile = transModel()->getFile(transModel()->findLanguage(newFile->language()));

    xml_node transContext;

    qDebug() << "New file:" << newFile->fileName() << "Translated file:" << transFile->fileName();

    for(xml_node newContext : newFile->rootNode()->children("context")) {
      QString newName, transName;
      bool breakContext = false;
      bool skipContext = false;

      xml_node curTransContext;

      // Find the same context, skip contexes from the translated file that does not exist in the new file
      do{
        if(transContext.empty()) {
          transContext = transFile->rootNode()->child("context");
          curTransContext = transContext;
        }
        else if (curTransContext.empty()) {

          curTransContext = transContext.next_sibling();
//          transFile->rootNode()->remove_child(transContext.previous_sibling());
        } else
          curTransContext = curTransContext.next_sibling();


        if(curTransContext.empty()) {
          breakContext = true;
          break;
        }

        newName = newContext.child("name").child_value();
        transName = curTransContext.child("name").child_value();

        qDebug() << " - Context:" << newName << transName;

        if(newName < transName) {
          skipContext = true;
          qDebug() << " - Skipping Context:" << newName;
          break;
//          curTransContext = transFile->rootNode()->insert_child_before("context", curTransContext);
//          curTransContext.append_child("name").set_value(newName.toStdString().c_str());
//          transName = newName;
        }

      } while (newName != transName);

      if(breakContext)
        break;
      if(skipContext)
        continue;

      transContext = curTransContext;

      xml_node transMessage, curTransMessage;

      for(xml_node newMessage : newContext.children("message")) {
        QString newSource, transSource;

        bool skipMessage = false;
        bool breakMessage = false;

        do {
          if(transMessage.empty())
            transMessage = transContext.child("message");
          else {

            if(curTransMessage == transMessage) {
              xml_attribute check;
              do {
                transMessage = transMessage.next_sibling();
                check = transMessage.attribute("node_check");

              } while(!transMessage.empty() && !check.empty());
            }

//            transContext.remove_child(transMessage.previous_sibling());
          }

          if(transMessage.empty()) {
            breakMessage = true;
            break;
          }

          curTransMessage = transMessage;
          newSource = newMessage.child("source").child_value();
          transSource = transMessage.child("source").child_value();

          qDebug() << "  -- Message:" << newSource << transSource;

          // Brute force the search of the whole current transContext for the source
          if(newSource != transSource) {

            xml_node tempTransMessage = transContext.child("message");
            bool found = false;

            // Search the translated file for the source
            // If it exists, set the current translaton message to that and do the message merge
            // If it does not, just skip this new message as it does not exist in the old/translated file
            while(!tempTransMessage.empty()) {
              QString tempTransSource = tempTransMessage.child("source").child_value();
              if(tempTransSource == newSource) {
                tempTransMessage.append_attribute("node_check").set_value("ok");
                curTransMessage = tempTransMessage;
                transSource = newSource;
                found = true;
                break;
              }
              else
                tempTransMessage = tempTransMessage.next_sibling();
            }

            if(!found) {
              skipMessage = true;
              curTransMessage = xml_node();
              break;
            }

//            QString newLocation = newMessage.child("location").attribute("filename").value();
//            QString transLocation = transMessage.child("location").attribute("filename").value();

//            if(newLocation < transLocation) {
//              skipMessage = true;
//              qDebug() << "  -- Skipping Message:" << newSource;
//              break;
//            }
//            else if(newLocation == transLocation) {
//              xml_node tempTransMessage = transMessage.next_sibling();
//              bool found = false;

//              // Search the translated file for the source
//              // If it exists, set the current translaton message to that and do the message merge
//              // If it does not, just skip this new message as it does not exist in the old/translated file
//              while(tempTransMessage.child("location").attribute("filename").value() == newLocation) {
//                QString tempTransSource = tempTransMessage.child("source").child_value();
//                if(tempTransSource == newSource) {
//                  tempTransMessage.append_attribute("node_check").set_value("ok");
//                  curTransMessage = tempTransMessage;
//                  transSource = newSource;
//                  found = true;
//                  break;
//                }
//                else
//                  tempTransMessage = tempTransMessage.next_sibling();
//              }

//              if(!found) {
//                skipMessage = true;
//                curTransMessage = xml_node();
//                break;
//              }

//            }
          }

        } while(newSource != transSource);

        if(breakMessage)
          break;

        if(skipMessage)
          continue;

        qDebug() << "   --- Merging Message:" << newSource;
        // Set translation
        xml_node newTranslation = newMessage.child("translation");
        xml_node transTranslation = curTransMessage.child("translation");

        xml_attribute newTranslationType = newTranslation.attribute("type");
        xml_attribute transTranslationType = transTranslation.attribute("type");

        if(transTranslationType.empty() && !newTranslationType.empty())
          newTranslation.remove_attribute("type");
        else if(!transTranslationType.empty()) {
          newTranslationType.set_value(transTranslationType.value());
        }

        const char* transValue = transTranslation.child_value();
        xml_text newText = newTranslation.text();
        newText.set(transValue);

        // Set translator comment
        xml_node newComment = newMessage.child("translatorcomment");
        xml_node transComment = curTransMessage.child("translatorcomment");

        if(!transComment.empty()) {
          if(newComment.empty())
            newComment = newMessage.insert_child_before("translatorcomment", newTranslation);

          xml_text newCommentText = newComment.text();
          newCommentText.set(transComment.child_value());
        }
      }

    }


    QDir outputDir(QDir::separator() + outputPath);
    QString path = outputDir.filePath(transFile->fileName());
    QString absPath = outputDir.absoluteFilePath(transFile->fileName());
    QString root = outputDir.rootPath();


    qDebug() << "Saving File:" << path;
    qDebug() << "Success:" << newFile->saveFile(path);

  }

  m_newModel->reset();
  m_transModel->reset();

}





























