#include "service/ResourceManager.h"
#include "viewmodel/GameWorldViewModel.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    skybound::ResourceManager resourceManager;
    skybound::GameWorldViewModel gameWorld(resourceManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameWorld", &gameWorld);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

    return app.exec();
}
