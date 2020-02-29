#pragma once

#include <fstream>

class QFile;

class FileUtil {
public:
	///
	/// <summary> Get an std::ifstream to read data from a QFile. </summary>
	/// <param name="file"> The file to read from; must already be open with read privledges </param>
	/// <returns> An std::ifstream open to the qfile </returns>
	///
	static std::ifstream getInputStream(QFile& file);
};

