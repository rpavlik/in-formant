#include "../gui/canvas.h"
#include "contextmanager.h"
#include "qsgrendererinterface.h"
#include "qsurfaceformat.h"
#include "rendercontext.h"
#include <QQmlContext>
#include <iostream>
#include <chrono>
#include <stdexcept>

using namespace Main;
using namespace std::chrono_literals;

#ifndef WITHOUT_SYNTH
GuiContext::GuiContext(Config *config, RenderContext *renderContext, SynthWrapper *synthWrapper, DataVisWrapper *dataVisWrapper)
#else
GuiContext::GuiContext(Config *config, RenderContext *renderContext, DataVisWrapper *dataVisWrapper)
#endif
    : mConfig(config),
      mRenderContext(renderContext),
      mSelectedView(nullptr)
{
    QCoreApplication::setApplicationName("InFormant");
    QCoreApplication::setApplicationVersion(INFORMANT_VERSION_STR);
    QCoreApplication::setOrganizationDomain("in-formant.app");
    QCoreApplication::setOrganizationName("InFormant");

    QQuickStyle::setStyle("Material");
    qmlRegisterType<Gui::CanvasItem>("IfCanvas", 1, 0, "IfCanvas");

    mApp = std::make_unique<QApplication>(argc, argv);

    // Only supports OpenGL for now.
    QQuickWindow::setSceneGraphBackend("rhi");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication::setWindowIcon(QIcon(":/icons/in-formant.png"));

    mQmlEngine = std::make_unique<QQmlApplicationEngine>();
    mQmlEngine->addImportPath(QCoreApplication::applicationDirPath() + "/qml");
#ifdef __APPLE__
    mQmlEngine->addImportPath(QCoreApplication::applicationDirPath() + "/../Resources/qml");
#endif

    mQmlEngine->rootContext()->setContextProperty("appName", "InFormant " INFORMANT_VERSION_STR);

    mQmlEngine->rootContext()->setContextProperty("config", mConfig);

#ifndef WITHOUT_SYNTH
    mQmlEngine->rootContext()->setContextProperty("synth", synthWrapper);
    mQmlEngine->rootContext()->setContextProperty("HAS_SYNTH", true);
#else
    mQmlEngine->rootContext()->setContextProperty("HAS_SYNTH", false);
#endif

#ifdef ENABLE_TORCH
    mQmlEngine->rootContext()->setContextProperty("HAS_TORCH", true);
#else
    mQmlEngine->rootContext()->setContextProperty("HAS_TORCH", false);
#endif

    mQmlEngine->rootContext()->setContextProperty("dataVis", dataVisWrapper);

    mQmlEngine->load(QUrl("qrc:/MainWindow.qml"));

    auto root = mQmlEngine->rootObjects();
    if (root.empty()) {
        throw std::runtime_error("Could not create QML main window, probably missing qml6-module-*");
    }
    auto window = static_cast<QQuickWindow *>(root.first());
    auto canvasItem = window->findChild<Gui::CanvasItem *>("IfCanvas");

    canvasItem->setRenderContext(mRenderContext);

    canvasItem->installEventFilter(this);
    window->installEventFilter(this);

    window->show();
}

int GuiContext::exec()
{
    return mApp->exec();
}

void GuiContext::setView(GuiView *view)
{
    mSelectedView = view;
}

bool GuiContext::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (mSelectedView != nullptr && mSelectedView->onKeyPress(keyEvent)) {
            return true;
        }
        else {
        }
    }
    else if (event->type() == QEvent::KeyRelease) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (mSelectedView != nullptr && mSelectedView->onKeyRelease(keyEvent)) {
            return true;
        }
        else {
        }
    }

    return false;
}

void GuiContext::setShowSpectrogram(bool b)
{
    mConfig->setViewShowSpectrogram(b);
}

void GuiContext::setShowPitch(bool b)
{
    mConfig->setViewShowPitch(b);
}

void GuiContext::setShowFormants(bool b)
{
    mConfig->setViewShowFormants(b);
}
