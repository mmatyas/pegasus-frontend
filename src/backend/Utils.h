#pragma once

class QString;


/// Returns true if the path is an existing regular file
bool validFile(const QString& path);

/// If the string can be converted to an integer,
/// it will be saved to the provided field
void parseStoreInt(const QString& str, int& val);
