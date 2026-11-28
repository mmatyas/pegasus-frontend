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

/// Exposes an optional secondary "presentation" display (eg. the lower
/// screen of dual-screen handhelds) to themes. The theme QML is rendered
/// offscreen and pushed as raw frames to the Android side.
///
/// - Touches on the secondary screen are delivered to the QML as mouse events.
/// - Key events (keyboard and gamepad) are also delivered, as a copy, to the
///   root item of the secondary QML; the main window receives them as usual.

#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QUrl>

class QEvent;
class QOffscreenSurface;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QQmlEngine;
class QQuickItem;
class QQuickRenderControl;
class QQuickWindow;

namespace model {
class SecondaryScreen : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(int displayWidth READ displayWidth NOTIFY displayChanged)
    Q_PROPERTY(int displayHeight READ displayHeight NOTIFY displayChanged)
    Q_PROPERTY(QObject* game READ game WRITE setGame NOTIFY gameChanged)

public:
    explicit SecondaryScreen(QObject* parent = nullptr);
    ~SecondaryScreen() override;

    void setApi(QObject* api);
    bool available() const;
    bool active() const { return m_active; }
    int displayWidth() const { return m_display_width; }
    int displayHeight() const { return m_display_height; }
    QObject* game() const { return m_game.data(); }
    void setGame(QObject* game);
    Q_INVOKABLE void load(const QUrl& qml_file);
    Q_INVOKABLE void hide();

    // Thread-safe: called from the Android UI thread through JNI
    static void postTouch(int action, float x, float y);

    static void postDisplaysChanged();

    signals:
        void availableChanged();
        void activeChanged();
        void displayChanged();
        void gameChanged();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void pushFrame();
    void onDisplaysChanged();

private:
    void teardown(bool dismiss_presentation);
    void injectTouch(int action, float x, float y);  // Qt thread only

    QObject* m_api = nullptr;
    QQmlEngine* m_engine = nullptr;
    QQuickWindow* m_window = nullptr;
    QQuickItem* m_root_item = nullptr;
    QQuickRenderControl* m_rc = nullptr;
    QOpenGLContext* m_gl = nullptr;
    QOffscreenSurface* m_gl_surface = nullptr;
    QOpenGLFramebufferObject* m_fbo = nullptr;
    QTimer m_frame_timer;
    QTimer m_display_debounce;
    QPointer<QObject> m_game;
    QUrl m_qml_url;
    int m_display_width = 0;
    int m_display_height = 0;
    bool m_active = false;
    bool m_requested = false;
    bool m_last_available = false;
    bool m_dirty = true;
};
} // namespace model