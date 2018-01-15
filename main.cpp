#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>

#include "documentmanager.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  DocumentManager *manager = &DocumentManager::sharedManager();

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("manager", manager);
  engine.rootContext()->setContextProperty("newTsModel", manager->newModel());
  engine.rootContext()->setContextProperty("transTsModel", manager->transModel());

  engine.load(QUrl(QLatin1String("qrc:/main.qml")));



  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
