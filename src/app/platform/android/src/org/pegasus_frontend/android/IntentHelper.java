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

import android.content.ComponentName;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedList;


final class IntentHelper {
    // Based on the Intent.parseCommandArgs function of the Android source code,
    // at commit 0fd623d6c3c4e65ab8c306f541fecae8a77393be
    public static Intent parseIntentCommand(LinkedList<String> args) throws URISyntaxException {
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
}
