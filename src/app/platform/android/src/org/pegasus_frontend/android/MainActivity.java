// Pegasus Frontend
// Copyright (C) 2017-2021  Mátyás Mustoha
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

import android.app.ActivityManager;
import android.content.Context;
import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.UriPermission;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.BatteryManager;
import android.os.Build;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.Settings;
import androidx.core.content.FileProvider;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import android.hardware.display.DisplayManager;
import android.view.Display;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.atomic.AtomicBoolean;
import android.graphics.Point;
import android.os.Handler;
import android.os.Looper;


public class MainActivity extends org.qtproject.qt5.android.bindings.QtActivity {
    private static Activity m_self;
    private static PackageManager m_pm;
    private static int m_icon_density;

    private static DualScreen m_presentation;


    @Override
    protected void onStart() {
        super.onStart();
        m_self = this;
        m_pm = getPackageManager();
        registerDisplayListener();

        ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        m_icon_density = am.getLauncherLargeIconDensity();
    }


    public static App[] appList() {
        Intent intent = new Intent(Intent.ACTION_MAIN, null);
        intent.addCategory(Intent.CATEGORY_LAUNCHER);
        List<ResolveInfo> activities = m_pm.queryIntentActivities(intent, 0);

        App[] entries = new App[activities.size()];
        for (int i = 0; i < activities.size(); i++)
            entries[i] = new App(m_pm, activities.get(i));

        return entries;
    }


    public static byte[] appIcon(String packageName) {
        Drawable drawable = null;
        try {
            // NOTE: while there is m_pm.getApplicationInfo(), unfortunately
            //       that returns low density images for most apps
            ApplicationInfo appinfo = m_pm.getApplicationInfo(packageName, 0);
            Resources resources = m_pm.getResourcesForApplication(appinfo);
            Intent launch_intent = m_pm.getLaunchIntentForPackage(packageName);
            ResolveInfo resolveinfo = m_pm.resolveActivity(launch_intent, 0);
            // NOTE: getDrawableForDensity() has changed in API 21-22
            drawable = resources.getDrawableForDensity(resolveinfo.activityInfo.getIconResource(), m_icon_density);
        }
        catch (Exception ex) { }
        if (drawable == null)
            drawable = m_pm.getDefaultActivityIcon();

        Bitmap bitmap = drawableToBitmap(drawable);
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream);
        return stream.toByteArray();
    }


    private static Bitmap drawableToBitmap(Drawable drawable) {
        if (drawable instanceof BitmapDrawable) {
            // TODO: handle null
            return ((BitmapDrawable) drawable).getBitmap();
        }

        int w = Math.max(1, drawable.getIntrinsicWidth());
        int h = Math.max(1, drawable.getIntrinsicHeight());
        Bitmap bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);
        return bitmap;
    }


    public static String primaryStoragePath() {
        final File storage = Environment.getExternalStorageDirectory();
        return storage.getAbsolutePath();
    }


    public static String[] sdcardPaths() {
        // Functions with high API level dependencies:
        // - https://developer.android.com/reference/android/os/storage/StorageManager#getStorageVolumes()
        // - https://developer.android.com/reference/android/os/storage/StorageVolume#getDirectory()

        final StorageManager storage_man = (StorageManager) m_self.getSystemService(Context.STORAGE_SERVICE);

        List<StorageVolume> storage_vols = null;
        if (Build.VERSION.SDK_INT >= 24) {
            storage_vols = storage_man.getStorageVolumes();
        }
        else {
            try {
                final Method volumelist_getter = StorageManager.class.getMethod("getVolumeList");
                final StorageVolume[] storage_vols_arr = (StorageVolume[]) volumelist_getter.invoke(storage_man);
                storage_vols = Arrays.asList(storage_vols_arr);
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            }
        }

        List<File> mount_points = new ArrayList<File>();
        if (Build.VERSION.SDK_INT >= 30) {
            for (StorageVolume sv : storage_vols)
                mount_points.add(sv.getDirectory());
        }
        else {
            try {
                final Method dir_getter = StorageVolume.class.getMethod("getPathFile");
                for (StorageVolume sv : storage_vols) {
                    final File mount_point = (File) dir_getter.invoke(sv);
                    mount_points.add(mount_point);
                }
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            }
        }

        List<String> paths = new ArrayList<String>();
        for (File mp : mount_points) {
            if (mp != null)
                paths.add(mp.getAbsolutePath());
        }
        paths.add("/"); // Always add the root
        return paths.toArray(new String[paths.size()]);
    }


    public static String[] grantedPaths() {
        List<String> paths = new ArrayList();
        for (UriPermission uriperm : m_self.getContentResolver().getPersistedUriPermissions()) {
            final Uri uri = uriperm.getUri();
            paths.add(uri.getPath());
        }
        return paths.toArray(new String[paths.size()]);
    }


    public static void rememberGrantedPath(Uri uri) {
        m_self
                .getContentResolver()
                .takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
    }


    public static boolean getAllStorageAccess() {
        if (Build.VERSION.SDK_INT < 30)
            return true;

        if (Environment.isExternalStorageManager())
            return true;

        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
        intent.setData(Uri.parse("package:" + m_self.getPackageName()));
        m_self.startActivity(intent);
        return false;
    }

    public static BatteryInfo queryBattery() {
        final IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        final Intent batIntent = m_self.registerReceiver(null, ifilter);

        final int batStatus = batIntent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        if (batStatus == BatteryManager.BATTERY_STATUS_UNKNOWN)
            return null;

        final boolean hasBattery = batIntent.getBooleanExtra(BatteryManager.EXTRA_PRESENT, true);
        final boolean batPlugged = batIntent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1) > 0;
        final boolean batCharged = batStatus == BatteryManager.BATTERY_STATUS_FULL;

        final int batLevel = batIntent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        final int batScale = batIntent.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        final float batPercent = (batLevel >= 0 && batScale > 0)
                ? batLevel / (float) batScale
                : Float.NaN;

        return new BatteryInfo(hasBattery, batPlugged, batCharged, batPercent);
    }

    public static String launchAmCommand(String[] args_arr) {
        final LinkedList<String> args = new LinkedList(Arrays.asList(args_arr));
        if (args.isEmpty())
            return "No arguments provided to 'am'";

        final String am_command = args.pop().toLowerCase();
        if (!am_command.equals("start"))
            return "For 'am', only the 'start' command is supported at the moment, '" + am_command + "' is not";

        try {
            Intent intent = IntentHelper.parseIntentCommand(args);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            m_self.startActivity(intent);
        }
        catch (Exception e) {
            return e.toString() + ": " + e.getMessage();
        }

        return null;
    }

    public static int runScript(String[] args) {
        try {
            final ProcessBuilder pb = new ProcessBuilder(args);
            pb.redirectErrorStream(true);
            final Process proc = pb.start();

            // consume output so a full pipe can't block the script
            final java.io.BufferedReader reader = new java.io.BufferedReader(
                    new java.io.InputStreamReader(proc.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null)
                android.util.Log.i("Pegasus/script", line);

            return proc.waitFor();
        }
        catch (Exception e) {
            android.util.Log.w("Pegasus/script", e.toString());
            return -1;
        }
    }

    public static String toContentUri(String path) {
        final Uri uri = FileProvider.getUriForFile(
                m_self,
                m_self.getPackageName() + ".files",
                new File(path));
        return uri.toString();
    }

    @Override
    protected void onDestroy() {
        if (m_display_listener != null) {
            final DisplayManager dm = (DisplayManager) getSystemService(Context.DISPLAY_SERVICE);
            dm.unregisterDisplayListener(m_display_listener);
            m_display_listener = null;
        }
        if (m_presentation != null) {
            m_presentation.dismiss();
            m_presentation = null;
        }
        super.onDestroy();
    }

    // ---- Secondary display (dual screen) support ----

    // ---- Frame transport: the Qt thread writes, the UI thread reads ----
    private static ByteBuffer m_frame_buf;
    private static int m_frame_w, m_frame_h;
    private static Bitmap m_frame_bmp;   // last frame, kept to paint a Presentation shown late
    private static final AtomicBoolean m_frame_busy = new AtomicBoolean(false);
    private static native void nativeDisplaysChanged();
    private static DisplayManager.DisplayListener m_display_listener;

    private static Display secondaryDisplay() {
        if (m_self == null)
            return null;
        final DisplayManager dm = (DisplayManager) m_self.getSystemService(Context.DISPLAY_SERVICE);
        final Display[] displays = dm.getDisplays(DisplayManager.DISPLAY_CATEGORY_PRESENTATION);
        return displays.length > 0 ? displays[0] : null;
    }

    private void registerDisplayListener() {
        if (m_display_listener != null)
            return;

        m_display_listener = new DisplayManager.DisplayListener() {
            @Override public void onDisplayAdded(int displayId) { notifyDisplaysChanged(); }
            @Override public void onDisplayRemoved(int displayId) { notifyDisplaysChanged(); }
            @Override public void onDisplayChanged(int displayId) { notifyDisplaysChanged(); }
        };
        final DisplayManager dm = (DisplayManager) getSystemService(Context.DISPLAY_SERVICE);
        dm.registerDisplayListener(m_display_listener, new Handler(Looper.getMainLooper()));
    }

    private static void notifyDisplaysChanged() {
        try {
            nativeDisplaysChanged();
        } catch (UnsatisfiedLinkError e) {
            android.util.Log.w("Pegasus/dualscreen", "nativeDisplaysChanged not linked: " + e);
        }
    }

    // True if Pegasus is the default home app (launcher)
    private static boolean isDefaultLauncher() {
        final Intent home = new Intent(Intent.ACTION_MAIN);
        home.addCategory(Intent.CATEGORY_HOME);
        final ResolveInfo info = m_pm.resolveActivity(home, PackageManager.MATCH_DEFAULT_ONLY);
        return info != null
                && info.activityInfo != null
                && m_self.getPackageName().equals(info.activityInfo.packageName);
    }

    public static boolean secondaryAvailable() {
        return secondaryDisplay() != null;
    }

    public static int secondaryWidth() {
        final Display display = secondaryDisplay();
        if (display == null)
            return 0;
        final Point size = new Point();
        display.getRealSize(size);
        return size.x;
    }

    public static int secondaryHeight() {
        final Display display = secondaryDisplay();
        if (display == null)
            return 0;
        final Point size = new Point();
        display.getRealSize(size);
        return size.y;
    }

    public static void secondaryShow() {
        if (m_self == null)
            return;

        m_self.runOnUiThread(() -> {
            final Display display = secondaryDisplay();
            if (display == null)
                return;

            if (m_presentation != null) {
                if (m_presentation.isShowing())
                    return;
                m_presentation = null;
            }

            try {
                m_presentation = new DualScreen(m_self, display, isDefaultLauncher());
                m_presentation.show();
                if (m_frame_bmp != null)
                    m_presentation.showBitmap(m_frame_bmp);
            } catch (Exception e) {
                android.util.Log.w("Pegasus/dualscreen", e.toString());
                m_presentation = null;
            }
        });
    }

    public static void secondaryHide() {
        if (m_self == null)
            return;

        m_self.runOnUiThread(() -> {
            m_frame_bmp = null;
            if (m_presentation != null) {
                m_presentation.dismiss();
                m_presentation = null;
            }
        });
    }

    // Qt thread. Returns null if the previous frame is still being consumed (drop this one).
    public static ByteBuffer secondaryBeginFrame(int w, int h) {
        if (!m_frame_busy.compareAndSet(false, true))
            return null;
        if (m_frame_buf == null || m_frame_w != w || m_frame_h != h) {
            m_frame_buf = ByteBuffer.allocateDirect(w * h * 4).order(ByteOrder.nativeOrder());
            m_frame_w = w;
            m_frame_h = h;
        }
        return m_frame_buf;
    }

    // Qt thread. Gives up the buffer without showing it.
    public static void secondaryAbortFrame() {
        m_frame_busy.set(false);
    }

    // Qt thread. The buffer has been filled.
    public static void secondaryCommitFrame() {
        if (m_self == null) {
            m_frame_busy.set(false);
            return;
        }
        m_self.runOnUiThread(() -> {
            try {
                applyFrame();
            } finally {
                m_frame_busy.set(false);
            }
        });
    }

    // UI thread only
    private static void applyFrame() {
        final int w = m_frame_w;
        final int h = m_frame_h;
        if (m_frame_bmp == null || m_frame_bmp.getWidth() != w || m_frame_bmp.getHeight() != h)
            m_frame_bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);

        m_frame_buf.rewind();
        m_frame_bmp.copyPixelsFromBuffer(m_frame_buf);

        if (m_presentation != null && m_presentation.isShowing())
            m_presentation.showBitmap(m_frame_bmp);
    }
}
