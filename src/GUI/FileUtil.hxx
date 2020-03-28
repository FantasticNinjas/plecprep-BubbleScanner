#pragma once

#include <fstream>

class QFile;

class FileUtil {
public:
	///
	/// <summary> Get an input filestream to read data from a QFile. </summary>
	/// <param name="file"> The file to read from; must already be open with read privledges </param>
	/// <returns> An std::ifstream open to the QFile </returns>
	///
	static std::ifstream getInputStream(QFile& file);

	///
	/// <summary> Get an output filestream to write data to a QFile. </summary>
	/// <param name="file"> The file to read from; must already be open with write privledges </param>
	/// <returns> An std::ofstream open to the QFile </returns>
	///
	static std::ofstream getOutputStream(QFile& file);
};

