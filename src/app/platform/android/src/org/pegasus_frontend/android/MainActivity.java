package org.pegasus_frontend.android;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


final class App {
    private final String m_package_name;
    private final String m_app_name;
    private final String m_launch_action;
    private final String m_launch_component;

    public App(PackageManager pm, ResolveInfo info) {
        m_package_name = info.activityInfo.packageName;
        m_app_name = info.loadLabel(pm).toString();

        Intent launch_intent = pm.getLaunchIntentForPackage(m_package_name);
        if (launch_intent != null) {
            m_launch_action = launch_intent.getAction();
            m_launch_component = launch_intent.getComponent().flattenToShortString();
        }
        else {
            m_launch_action = "";
            m_launch_component = "";
        }
    }

    public String packageName() { return m_package_name; }
    public String appName() { return m_app_name; }
    public String launchAction() { return m_launch_action; }
    public String launchComponent() { return m_launch_component; }
}


final class BatteryInfo {
    private final boolean m_present;
    private final boolean m_plugged;
    private final boolean m_charged;
    private final float m_percent;

    public BatteryInfo(boolean present, boolean plugged, boolean charged, float percent) {
        m_present = present;
        m_plugged = plugged;
        m_charged = charged;
        m_percent = percent;
    }

    public boolean present() { return m_present; }
    public boolean plugged() { return m_plugged; }
    public boolean charged() { return m_charged; }
    public float percent() { return m_percent; }
}


public class MainActivity extends org.qtproject.qt5.android.bindings.QtActivity {
    private static Context m_context;
    private static PackageManager m_pm;
    private static int m_icon_density;


    @Override
    protected void onStart() {
        super.onStart();
        m_context = this;
        m_pm = getPackageManager();

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


    public static String[] sdcardPaths() {
        // Functions with high API level dependencies:
        // - https://developer.android.com/reference/android/os/storage/StorageManager#getStorageVolumes()
        // - https://developer.android.com/reference/android/os/storage/StorageVolume#getDirectory()

        final StorageManager storage_man = (StorageManager) m_context.getSystemService(Context.STORAGE_SERVICE);

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


    public static boolean getAllStorageAccess() {
        if (Build.VERSION.SDK_INT < 30)
            return true;

        if (Environment.isExternalStorageManager())
            return true;

        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
        intent.setData(Uri.parse("package:" + m_context.getPackageName()));
        m_context.startActivity(intent);
        return false;
    }

    public static BatteryInfo queryBattery() {
        final IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        final Intent batIntent = m_context.registerReceiver(null, ifilter);

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
}
