package org.pegasus_frontend.android;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
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


public class MainActivity extends org.qtproject.qt5.android.bindings.QtActivity {
    private static PackageManager m_pm;

    @Override
    protected void onStart() {
        super.onStart();
        m_pm = getPackageManager();
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
}
