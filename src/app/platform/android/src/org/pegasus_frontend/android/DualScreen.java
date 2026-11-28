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


package org.pegasus_frontend.android;

import android.app.Presentation;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;


public class DualScreen extends Presentation {
    private static final int TOUCH_DOWN = 0;
    private static final int TOUCH_MOVE = 1;
    private static final int TOUCH_UP = 2;

    private final ImageView m_view;
    private final boolean m_block_back;
    private int m_bmp_w = 0;
    private int m_bmp_h = 0;
    private boolean m_tracking = false;

    private static native void nativeTouch(int action, float x, float y);

    public DualScreen(Context outerContext, Display display, boolean blockBack) {
        super(outerContext, display);
        m_block_back = blockBack;

        m_view = new ImageView(getContext());
        m_view.setBackgroundColor(Color.BLACK);
        m_view.setScaleType(ImageView.ScaleType.FIT_CENTER);
        m_view.setOnTouchListener((v, ev) -> handleTouch(v, ev));
        setContentView(m_view);
    }

    @Override
    public void onBackPressed() {
        // When Pegasus is the launcher there is nothing to go back to
        if (!m_block_back)
            super.onBackPressed();
    }

    public void showBitmap(Bitmap bmp) {
        m_bmp_w = bmp.getWidth();
        m_bmp_h = bmp.getHeight();
        m_view.setImageBitmap(bmp);  // new drawable each time: forces a redraw of the reused bitmap
    }

    // View coordinates -> frame (image) coordinates, accounting for FIT_CENTER letterboxing
    private boolean handleTouch(View v, MotionEvent ev) {
        if (m_bmp_w <= 0 || m_bmp_h <= 0)
            return false;

        final float vw = v.getWidth();
        final float vh = v.getHeight();
        final float scale = Math.min(vw / m_bmp_w, vh / m_bmp_h);
        final float ox = (vw - m_bmp_w * scale) / 2f;
        final float oy = (vh - m_bmp_h * scale) / 2f;
        final float x = (ev.getX() - ox) / scale;
        final float y = (ev.getY() - oy) / scale;

        switch (ev.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                m_tracking = x >= 0 && y >= 0 && x < m_bmp_w && y < m_bmp_h;
                if (m_tracking)
                    sendTouch(TOUCH_DOWN, x, y);
                return true;
            case MotionEvent.ACTION_MOVE:
                if (m_tracking)
                    sendTouch(TOUCH_MOVE, x, y);
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                if (m_tracking) {
                    sendTouch(TOUCH_UP, x, y);
                    m_tracking = false;
                }
                return true;
            default:
                return true;  // extra pointers are ignored for now
        }
    }

    private void sendTouch(int action, float x, float y) {
        try {
            nativeTouch(action, x, y);
        } catch (UnsatisfiedLinkError e) {
            android.util.Log.w("Pegasus/dualscreen", "nativeTouch not linked: " + e);
        }
    }
}