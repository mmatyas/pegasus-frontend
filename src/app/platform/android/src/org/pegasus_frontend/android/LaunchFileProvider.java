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

import android.content.Context;
import android.content.ContentValues;
import androidx.core.content.FileProvider;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import java.io.File;
import java.io.FileNotFoundException;
import java.net.URLEncoder;
import java.util.List;

public class LaunchFileProvider extends FileProvider {

    public static final String LAUNCHFILEPROVIDER_AUTHORITY = "org.pegasus_frontend.android.files";

    public int delete(Uri uri, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException();
    }

    public String getType(Uri uri) {
        return super.getType(convertToFileProviderUri(uri));
    }

    public static Uri getUriForFile(Context context, String authority, File file) {
        return convertFromFileProviderUri(FileProvider.getUriForFile(context, authority, file));
    }

    public static Uri getUriForFile(Context context, String authority, File file, String displayName) {
        return convertFromFileProviderUri(FileProvider.getUriForFile(context, authority, file, displayName));
    }

    public Uri insert(Uri uri, ContentValues values) {
        throw new UnsupportedOperationException();
    }

    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        if (mode != "r") {
            throw new IllegalArgumentException("Launch Files must be opened read-only");
        }
        return super.openFile(convertToFileProviderUri(uri), mode);
    }

    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        return super.query(uri, projection, selection, selectionArgs, sortOrder);
    }

    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException();
    }

    private static Uri convertToFileProviderUri(Uri launchFileProviderUri) {
        List<String> pathSegments = launchFileProviderUri.getPathSegments();
        if (pathSegments.size() != 2) {
            throw new IllegalArgumentException(
                "Pegasus content uris must be in the form: content://org.pegasus_frontend.android.files/document/<url encoded path>");
        }
        return Uri.parse("content://" + LAUNCHFILEPROVIDER_AUTHORITY + Uri.decode(pathSegments.get(1)));
    }

    private static Uri convertFromFileProviderUri(Uri fileProviderUri) {
        String fp = Uri.encode(fileProviderUri.getPath());
        return Uri.parse("content://" + LAUNCHFILEPROVIDER_AUTHORITY + "/document/" + fp);
    }
}
