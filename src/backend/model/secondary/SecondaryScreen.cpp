// Pegasus Frontend
// Copyright (C) 2026  Giorgio Longobardo
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "SecondaryScreen.h"

#include "Log.h"
#include "imggen/BlurhashProvider.h"

#ifdef Q_OS_ANDROID
#include "platform/AndroidAppIconProvider.h"
#include "platform/AndroidHelpers.h"
#endif

#include <QCoreApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QWindow>

#ifdef Q_OS_ANDROID
#include <jni.h>
#endif


namespace {
    QMutex g_mutex;
    model::SecondaryScreen* g_active = nullptr;  // current session: receives touch input
    model::SecondaryScreen* g_object = nullptr;  // alive object: receives display changes
} // namespace


namespace model {
SecondaryScreen::SecondaryScreen(QObject* parent) : QObject(parent)
{
    connect(&m_frame_timer, &QTimer::timeout, this, &SecondaryScreen::pushFrame);
    m_display_debounce.setSingleShot(true);
    m_display_debounce.setInterval(250);  // display events often come in bursts
    connect(&m_display_debounce, &QTimer::timeout, this, &SecondaryScreen::onDisplaysChanged);
    QMutexLocker lock(&g_mutex);
    g_object = this;
}

SecondaryScreen::~SecondaryScreen()
{
    hide();
    QMutexLocker lock(&g_mutex);
    if (g_object == this)
        g_object = nullptr;
}

void SecondaryScreen::setApi(QObject* api)
{
    Q_ASSERT(api);
    m_api = api;
    // NOTE: no setParent() here, this object is a by-value member of ApiObject
}

bool SecondaryScreen::available() const
{
#ifdef Q_OS_ANDROID
    return android::has_secondary_display();
#else
    return false;
#endif
}

void SecondaryScreen::load(const QUrl& qml_file)
{
    // Remember the request: if the display is not there (yet), it is retried when it shows up
    m_qml_url = qml_file;
    m_requested = true;

    if (!available()) {
        Log::warning(LOGMSG("Secondary screen requested, but no secondary display found"));
        return;
    }
    teardown(false);  // keeps the Presentation (and its last frame): no flicker on reloads

#ifdef Q_OS_ANDROID
    android::secondary_show();
    const int width = android::secondary_width();
    const int height = android::secondary_height();
    if (width != m_display_width || height != m_display_height) {
        m_display_width = width;
        m_display_height = height;
        emit displayChanged();
    }
#endif
    if (m_display_width <= 0 || m_display_height <= 0) {
        Log::warning(LOGMSG("Secondary display has invalid metrics"));
        return;
    }

    // 1. Offscreen OpenGL context
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(16);
    fmt.setStencilBufferSize(8);

    m_gl = new QOpenGLContext(this);
    m_gl->setFormat(fmt);
    if (!m_gl->create()) {
        Log::warning(LOGMSG("Secondary screen: could not create the OpenGL context"));
        hide();
        return;
    }

    m_gl_surface = new QOffscreenSurface();
    m_gl_surface->setFormat(m_gl->format());
    m_gl_surface->create();

    // 2. Render control + window (never shown)
    m_rc = new QQuickRenderControl(this);
    connect(m_rc, &QQuickRenderControl::sceneChanged, this, [this] { m_dirty = true; });
    connect(m_rc, &QQuickRenderControl::renderRequested, this, [this] { m_dirty = true; });
    m_window = new QQuickWindow(m_rc);
    m_window->setGeometry(0, 0, m_display_width, m_display_height);

    // 3. QML engine
    m_engine = new QQmlEngine(this);
    if (!m_engine->incubationController())
        m_engine->setIncubationController(m_window->incubationController());

    m_engine->addImportPath(QStringLiteral("lib/qml"));
    m_engine->addImportPath(QStringLiteral("qml"));
    m_engine->addImageProvider(QStringLiteral("blurhash"), new BlurhashProvider);
#ifdef Q_OS_ANDROID
    m_engine->addImageProvider(QStringLiteral("androidicons"), new AndroidAppIconProvider);
#endif
    connect(m_engine, &QQmlEngine::quit, this, &SecondaryScreen::hide, Qt::QueuedConnection);
    if (m_api) {
        m_engine->rootContext()->setContextProperty(QStringLiteral("api"), m_api);
        m_engine->rootContext()->setContextProperty(QStringLiteral("Api"), m_api);
    }

    // 4. Load the QML
    QQmlComponent component(m_engine, qml_file);
    if (component.isError()) {
        Log::warning(LOGMSG("Secondary screen: QML load failed: %1").arg(component.errorString()));
        hide();
        return;
    }
    QObject* root_obj = component.create();
    if (!root_obj) {
        Log::warning(LOGMSG("Secondary screen: QML create failed: %1").arg(component.errorString()));
        hide();
        return;
    }
    auto* root_item = qobject_cast<QQuickItem*>(root_obj);
    if (!root_item) {
        Log::warning(LOGMSG("Secondary screen: the root QML element must be an Item"));
        delete root_obj;
        hide();
        return;
    }
    root_item->setParentItem(m_window->contentItem());
    root_item->setSize(QSizeF(m_display_width, m_display_height));
    m_root_item = root_item;

    // 5. Initialize the scene graph on the offscreen context, with an FBO target
    if (!m_gl->makeCurrent(m_gl_surface)) {
        Log::warning(LOGMSG("Secondary screen: makeCurrent failed"));
        hide();
        return;
    }
    m_rc->initialize(m_gl);
    m_fbo = new QOpenGLFramebufferObject(QSize(m_display_width, m_display_height), QOpenGLFramebufferObject::CombinedDepthStencil);
    m_window->setRenderTarget(m_fbo);
    m_gl->doneCurrent();

    {
        QMutexLocker lock(&g_mutex);
        g_active = this;
    }

    m_active = true;
    QCoreApplication::instance()->installEventFilter(this);
    emit activeChanged();
    m_dirty = true;
    m_frame_timer.start(33);  // ~30 fps, frames are only produced when the scene changes
}

void SecondaryScreen::pushFrame()
{
    if (!m_active || !m_rc || !m_gl || !m_fbo)
        return;
    if (!m_dirty)
        return;
    m_dirty = false;

    if (!m_gl->makeCurrent(m_gl_surface)) {
        m_dirty = true;  // retry on the next tick
        Log::warning(LOGMSG("Secondary screen: makeCurrent failed while rendering"));
        return;
    }

    m_rc->polishItems();
    m_rc->sync();
    m_rc->render();
    m_window->resetOpenGLState();

    const QImage frame = m_fbo->toImage();
    m_gl->doneCurrent();

    if (frame.isNull()) {
        Log::warning(LOGMSG("Secondary screen: empty frame"));
        return;
    }

#ifdef Q_OS_ANDROID
    // QImage ARGB32_Premultiplied -> the byte order Android's ARGB_8888 expects (R,G,B,A)
    const QImage rgba = frame.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    if (!android::secondary_push_frame(rgba))
        m_dirty = true;  // dropped (the Android side is busy): try again on the next tick
#endif
}

void SecondaryScreen::hide()
{
    m_requested = false;
    teardown(true);
}

void SecondaryScreen::teardown(bool dismiss_presentation)
{
    m_root_item = nullptr;

    if (QCoreApplication* const app = QCoreApplication::instance())
        app->removeEventFilter(this);

    {
        QMutexLocker lock(&g_mutex);
        if (g_active == this)
            g_active = nullptr;
    }

    m_dirty = true;

    m_frame_timer.stop();
    if (m_active) {
        m_active = false;
        emit activeChanged();
    }

    // Order matters: window (and its items) first, then the engine,
    // then the GL resources with the context current.
    delete m_window;
    m_window = nullptr;
    delete m_engine;
    m_engine = nullptr;

    if (m_gl && m_gl_surface && m_gl->makeCurrent(m_gl_surface)) {
        if (m_rc)
            m_rc->invalidate();

        delete m_fbo;
        m_fbo = nullptr;
        m_gl->doneCurrent();
    }
    delete m_fbo;  // no-op if already deleted above
    m_fbo = nullptr;
    delete m_rc;
    m_rc = nullptr;
    delete m_gl_surface;
    m_gl_surface = nullptr;
    delete m_gl;
    m_gl = nullptr;

#ifdef Q_OS_ANDROID
    if (dismiss_presentation)
        android::secondary_hide();
#else
    Q_UNUSED(dismiss_presentation)
#endif
}

void SecondaryScreen::setGame(QObject* game)
{
    if (m_game == game)
        return;

    if (m_game)
        disconnect(m_game, nullptr, this, nullptr);

    m_game = game;

    // If the game is destroyed (eg. the game list was reloaded), the QPointer
    // turns null by itself; just let the QML know and redraw
    if (m_game) {
        connect(m_game, &QObject::destroyed, this, [this] {
            emit gameChanged();
            m_dirty = true;
        });
    }

    emit gameChanged();
    m_dirty = true;
}

void SecondaryScreen::postTouch(int action, float x, float y)
{
    QMutexLocker lock(&g_mutex);
    SecondaryScreen* const self = g_active;
    if (!self)
        return;

    // The mutex is held until the event is queued; teardown() clears g_active
    // under the same mutex, so `self` can't be deleted in between.
    QMetaObject::invokeMethod(self, [self, action, x, y] { self->injectTouch(action, x, y); }, Qt::QueuedConnection);
}

void SecondaryScreen::injectTouch(int action, float x, float y)
{
    if (!m_active || !m_window)
        return;

    QEvent::Type type;
    Qt::MouseButton button = Qt::LeftButton;
    Qt::MouseButtons buttons = Qt::LeftButton;
    switch (action) {
        case 0:
            type = QEvent::MouseButtonPress;
            break;
        case 1:
            type = QEvent::MouseMove;
            button = Qt::NoButton;
            break;
        default:
            type = QEvent::MouseButtonRelease;
            buttons = Qt::NoButton;
            break;
    }

    const QPointF pos(x, y);
    QMouseEvent event(type, pos, pos, pos, button, buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(m_window, &event);
}

bool SecondaryScreen::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_active || !m_root_item)
        return false;
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return false;
    // Only the window-level delivery: the items of the main window get the same event afterwards
    if (!qobject_cast<QWindow *>(watched))
        return false;

    // The offscreen window never gets real input focus, so the copy is addressed to the
    // root item directly. The original event continues to the main window as usual.
    QKeyEvent copy(*static_cast<QKeyEvent *>(event));
    QCoreApplication::sendEvent(m_root_item, &copy);
    return false;
}

void SecondaryScreen::postDisplaysChanged()
{
    QMutexLocker lock(&g_mutex);
    SecondaryScreen* const self = g_object;
    if (!self)
        return;

    QMetaObject::invokeMethod(self, [self] { self->m_display_debounce.start(); }, Qt::QueuedConnection);
}

void SecondaryScreen::onDisplaysChanged()
{
    const bool now_available = available();
    if (now_available != m_last_available) {
        m_last_available = now_available;
        emit availableChanged();
    }

    if (!m_requested)
        return;

    if (!now_available) {
        if (m_active)
            teardown(true);  // the request is kept: the screen comes back with the display

        return;
    }

    bool size_changed = false;
#ifdef Q_OS_ANDROID
    size_changed = m_active
        && (android::secondary_width() != m_display_width
        || android::secondary_height() != m_display_height);
#endif
    if (!m_active || size_changed)
        load(m_qml_url);
}
} // namespace model

#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_org_pegasus_1frontend_android_DualScreen_nativeTouch(JNIEnv*, jclass, jint action, jfloat x, jfloat y)
{
    model::SecondaryScreen::postTouch(action, x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_org_pegasus_1frontend_android_MainActivity_nativeDisplaysChanged(JNIEnv*, jclass)
{
    model::SecondaryScreen::postDisplaysChanged();
}
#endif