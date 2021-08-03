package org.pegasus_frontend.android;

import android.app.ActivityManager;
import android.content.ComponentName;
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
import android.os.Bundle;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.Settings;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
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

    // Based on the Intent.parseCommandArgs function of the Android source code,
    // at commit 0fd623d6c3c4e65ab8c306f541fecae8a77393be
    private static Intent parseIntentCommand(LinkedList<String> args) throws URISyntaxException {
        Intent intent = new Intent();
        Intent baseIntent = intent;
        boolean hasIntentInfo = false;

        Uri data = null;
        String type = null;

        while (!args.isEmpty()) {
            final String opt = args.pop();
            switch (opt) {
                case "-a":
                    intent.setAction(args.pop());
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                case "-d":
                    data = Uri.parse(args.pop());
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                case "-t":
                    type = args.pop();
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                case "-i":
                    intent.setIdentifier(args.pop());
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                case "-c":
                    intent.addCategory(args.pop());
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                case "-e":
                case "--es": {
                    String key = args.pop();
                    String value = args.pop();
                    intent.putExtra(key, value);
                    break;
                }
                case "--esn": {
                    String key = args.pop();
                    intent.putExtra(key, (String) null);
                    break;
                }
                case "--ei": {
                    String key = args.pop();
                    String value = args.pop();
                    intent.putExtra(key, Integer.decode(value));
                    break;
                }
                case "--eu": {
                    String key = args.pop();
                    String value = args.pop();
                    intent.putExtra(key, Uri.parse(value));
                    break;
                }
                case "--ecn": {
                    String key = args.pop();
                    String value = args.pop();
                    ComponentName cn = ComponentName.unflattenFromString(value);
                    if (cn == null)
                        throw new IllegalArgumentException("Bad component name: " + value);
                    intent.putExtra(key, cn);
                    break;
                }
                case "--eia": {
                    String key = args.pop();
                    String value = args.pop();
                    String[] strings = value.split(",");
                    int[] list = new int[strings.length];
                    for (int i = 0; i < strings.length; i++) {
                        list[i] = Integer.decode(strings[i]);
                    }
                    intent.putExtra(key, list);
                    break;
                }
                case "--eial": {
                    String key = args.pop();
                    String value = args.pop();
                    String[] strings = value.split(",");
                    ArrayList<Integer> list = new ArrayList<>(strings.length);
                    for (int i = 0; i < strings.length; i++) {
                        list.add(Integer.decode(strings[i]));
                    }
                    intent.putExtra(key, list);
                    break;
                }
                case "--el": {
                    String key = args.pop();
                    String value = args.pop();
                    intent.putExtra(key, Long.valueOf(value));
                    break;
                }
                case "--ela": {
                    String key = args.pop();
                    String value = args.pop();
                    String[] strings = value.split(",");
                    long[] list = new long[strings.length];
                    for (int i = 0; i < strings.length; i++) {
                        list[i] = Long.valueOf(strings[i]);
                    }
                    intent.putExtra(key, list);
                    hasIntentInfo = true;
                    break;
                }
                case "--elal": {
                    String key = args.pop();
                    String value = args.pop();
                    String[] strings = value.split(",");
                    ArrayList<Long> list = new ArrayList<>(strings.length);
                    for (int i = 0; i < strings.length; i++) {
                        list.add(Long.valueOf(strings[i]));
                    }
                    intent.putExtra(key, list);
                    hasIntentInfo = true;
                    break;
                }
                case "--ef": {
                    String key = args.pop();
                    String value = args.pop();
                    intent.putExtra(key, Float.valueOf(value));
                    hasIntentInfo = true;
                    break;
                }
                case "--efa": {
                    String key = args.pop();
                    String value = args.pop();
                    String[] strings = value.split(",");
                    float[] list = new float[strings.length];
                    for (int i = 0; i < strings.length; i++) {
                        list[i] = Float.valueOf(strings[i]);
                    }
                    intent.putExtra(key, list);
                    hasIntentInfo = true;
                    break;
                }
                case "--efal": {
                    String key = args.pop();
                    String value = args.pop();
                    String[] strings = value.split(",");
                    ArrayList<Float> list = new ArrayList<>(strings.length);
                    for (int i = 0; i < strings.length; i++) {
                        list.add(Float.valueOf(strings[i]));
                    }
                    intent.putExtra(key, list);
                    hasIntentInfo = true;
                    break;
                }
                case "--esa": {
                    String key = args.pop();
                    String value = args.pop();
                    // Split on commas unless they are preceeded by an escape.
                    // The escape character must be escaped for the string and
                    // again for the regex, thus four escape characters become one.
                    String[] strings = value.split("(?<!\\\\),");
                    intent.putExtra(key, strings);
                    hasIntentInfo = true;
                    break;
                }
                case "--esal": {
                    String key = args.pop();
                    String value = args.pop();
                    // Split on commas unless they are preceeded by an escape.
                    // The escape character must be escaped for the string and
                    // again for the regex, thus four escape characters become one.
                    String[] strings = value.split("(?<!\\\\),");
                    ArrayList<String> list = new ArrayList<>(strings.length);
                    for (int i = 0; i < strings.length; i++) {
                        list.add(strings[i]);
                    }
                    intent.putExtra(key, list);
                    hasIntentInfo = true;
                    break;
                }
                case "--ez": {
                    String key = args.pop();
                    String value = args.pop().toLowerCase();
                    // Boolean.valueOf() results in false for anything that is not "true", which is
                    // error-prone in shell commands
                    boolean arg;
                    if ("true".equals(value) || "t".equals(value)) {
                        arg = true;
                    } else if ("false".equals(value) || "f".equals(value)) {
                        arg = false;
                    } else {
                        try {
                            arg = Integer.decode(value) != 0;
                        } catch (NumberFormatException ex) {
                            throw new IllegalArgumentException("Invalid boolean value: " + value);
                        }
                    }

                    intent.putExtra(key, arg);
                    break;
                }
                case "-n": {
                    String str = args.pop();
                    ComponentName cn = ComponentName.unflattenFromString(str);
                    if (cn == null)
                        throw new IllegalArgumentException("Bad component name: " + str);
                    intent.setComponent(cn);
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                }
                case "-p": {
                    String str = args.pop();
                    intent.setPackage(str);
                    if (intent == baseIntent)
                        hasIntentInfo = true;
                    break;
                }
                case "-f":
                    String str = args.pop();
                    intent.setFlags(Integer.decode(str).intValue());
                    break;
                case "--grant-read-uri-permission":
                    intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    break;
                case "--grant-write-uri-permission":
                    intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                    break;
                case "--grant-persistable-uri-permission":
                    intent.addFlags(Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
                    break;
                case "--grant-prefix-uri-permission":
                    intent.addFlags(Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);
                    break;
                case "--exclude-stopped-packages":
                    intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                    break;
                case "--include-stopped-packages":
                    intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
                    break;
                case "--debug-log-resolution":
                    intent.addFlags(Intent.FLAG_DEBUG_LOG_RESOLUTION);
                    break;
                case "--activity-brought-to-front":
                    intent.addFlags(Intent.FLAG_ACTIVITY_BROUGHT_TO_FRONT);
                    break;
                case "--activity-clear-top":
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    break;
                case "--activity-clear-when-task-reset":
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    break;
                case "--activity-exclude-from-recents":
                    intent.addFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                    break;
                case "--activity-launched-from-history":
                    intent.addFlags(Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY);
                    break;
                case "--activity-multiple-task":
                    intent.addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                    break;
                case "--activity-no-animation":
                    intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                    break;
                case "--activity-no-history":
                    intent.addFlags(Intent.FLAG_ACTIVITY_NO_HISTORY);
                    break;
                case "--activity-no-user-action":
                    intent.addFlags(Intent.FLAG_ACTIVITY_NO_USER_ACTION);
                    break;
                case "--activity-previous-is-top":
                    intent.addFlags(Intent.FLAG_ACTIVITY_PREVIOUS_IS_TOP);
                    break;
                case "--activity-reorder-to-front":
                    intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                    break;
                case "--activity-reset-task-if-needed":
                    intent.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
                    break;
                case "--activity-single-top":
                    intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
                    break;
                case "--activity-clear-task":
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                    break;
                case "--activity-task-on-home":
                    intent.addFlags(Intent.FLAG_ACTIVITY_TASK_ON_HOME);
                    break;
                case "--activity-match-external":
                    intent.addFlags(Intent.FLAG_ACTIVITY_MATCH_EXTERNAL);
                    break;
                case "--receiver-registered-only":
                    intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY);
                    break;
                case "--receiver-replace-pending":
                    intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                    break;
                case "--receiver-foreground":
                    intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
                    break;
                case "--receiver-no-abort":
                    intent.addFlags(Intent.FLAG_RECEIVER_NO_ABORT);
                    break;
                // NOTE: FLAG_RECEIVER_INCLUDE_BACKGROUND doesn't seem to exist?
                // case "--receiver-include-background":
                //     intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
                //     break;
                case "--selector":
                    intent.setDataAndType(data, type);
                    intent = new Intent();
                    break;
                // Direct options of 'am', quietly ignored for now
                case "-D":
                case "-N":
                case "-W":
                case "-S":
                case "--streaming":
                case "--track-allocation":
                case "--task-overlay":
                case "--lock-task":
                case "--allow-background-activity-starts":
                    break;
                case "-P":
                case "--start-profiler":
                case "--sampling":
                case "--attach-agent":
                case "--attach-agent-bind":
                case "-R":
                case "--user":
                case "--receiver-permission":
                case "--display":
                case "--windowingMode":
                case "--activityType":
                case "--task":
                    args.pop();
                    break;
                default:
                    throw new IllegalArgumentException("Unknown option: " + opt);
            }
        }
        intent.setDataAndType(data, type);

        final boolean hasSelector = intent != baseIntent;
        if (hasSelector) {
            // A selector was specified; fix up.
            baseIntent.setSelector(intent);
            intent = baseIntent;
        }

        String arg = args.isEmpty() ? null : args.pop();
        baseIntent = null;
        if (arg == null) {
            if (hasSelector) {
                // If a selector has been specified, and no arguments
                // have been supplied for the main Intent, then we can
                // assume it is ACTION_MAIN CATEGORY_LAUNCHER; we don't
                // need to have a component name specified yet, the
                // selector will take care of that.
                baseIntent = new Intent(Intent.ACTION_MAIN);
                baseIntent.addCategory(Intent.CATEGORY_LAUNCHER);
            }
        } else if (arg.indexOf(':') >= 0) {
            // The argument is a URI.  Fully parse it, and use that result
            // to fill in any data not specified so far.
            baseIntent = Intent.parseUri(arg, Intent.URI_INTENT_SCHEME
                    | Intent.URI_ANDROID_APP_SCHEME | Intent.URI_ALLOW_UNSAFE);
        } else if (arg.indexOf('/') >= 0) {
            // The argument is a component name.  Build an Intent to launch
            // it.
            baseIntent = new Intent(Intent.ACTION_MAIN);
            baseIntent.addCategory(Intent.CATEGORY_LAUNCHER);
            baseIntent.setComponent(ComponentName.unflattenFromString(arg));
        } else {
            // Assume the argument is a package name.
            baseIntent = new Intent(Intent.ACTION_MAIN);
            baseIntent.addCategory(Intent.CATEGORY_LAUNCHER);
            baseIntent.setPackage(arg);
        }
        if (baseIntent != null) {
            Bundle extras = intent.getExtras();
            intent.replaceExtras((Bundle)null);
            Bundle uriExtras = baseIntent.getExtras();
            baseIntent.replaceExtras((Bundle)null);
            if (intent.getAction() != null && baseIntent.getCategories() != null) {
                HashSet<String> cats = new HashSet<String>(baseIntent.getCategories());
                for (String c : cats) {
                    baseIntent.removeCategory(c);
                }
            }
            intent.fillIn(baseIntent, Intent.FILL_IN_COMPONENT | Intent.FILL_IN_SELECTOR);
            if (extras == null) {
                extras = uriExtras;
            } else if (uriExtras != null) {
                uriExtras.putAll(extras);
                extras = uriExtras;
            }
            intent.replaceExtras(extras);
            hasIntentInfo = true;
        }

        if (!hasIntentInfo)
            throw new IllegalArgumentException("No intent supplied");

        return intent;
    }

    public static String launchAmCommand(String[] args_arr) {
        final LinkedList<String> args = new LinkedList(Arrays.asList(args_arr));
        if (args.isEmpty())
            return "No arguments provided to 'am'";

        final String am_command = args.pop().toLowerCase();
        if (!am_command.equals("start"))
            return "For 'am', only the 'start' command is supported at the moment, '" + am_command + "' is not";

        try {
            Intent intent = parseIntentCommand(args);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            m_context.startActivity(intent);
        }
        catch (Exception e) {
            return e.toString() + ": " + e.getMessage();
        }

        return null;
    }
}
