#pragma once
#include <iosfwd>
#include <string>
#include <vector>

class  Path {
public:

	Path() = default;
	~Path() = default;
	Path(const char* pathname);
	Path(const std::string& pathname);
	bool exists() const;
	bool isFile() const;
	bool isDirectory() const;
	bool isEmpty() const { return m_path.empty(); }
	const char* c_str() const { return m_path.c_str(); }
	void setPath(const std::string& pathname) {
		m_path = getCanonicalPath(pathname);
	}
	const std::string& getPath() const { return m_path; }
	Path getParent() const;
	Path getAncestor(int n) const;
	std::string getName() const;
	std::string getNameWithoutExtension() const;
	std::string getExtension() const;
	Path getAbsolutePath() const;
	bool isAbsolute() const;
	std::vector<std::string> split() const;
	Path concat(const Path& path) const;
	void concatToSelf(const Path& path);
	operator std::string const& () const { return m_path; }

	Path operator+(const Path& rhs) const { return concat(rhs); }
	Path& operator+=(const Path& rhs) {
		concatToSelf(rhs);
		return *this;
	}

	bool operator==(const Path& rhs) const { return m_path == rhs.m_path; }
	bool operator!=(const Path& rhs) const { return m_path != rhs.m_path; }

	bool operator<(const Path& rhs) const { return m_path < rhs.m_path; }
	bool operator>(const Path& rhs) const { return m_path > rhs.m_path; }

	friend std::ostream& operator<<(std::ostream& os, const Path& path);


	static std::string getCanonicalPath(const std::string& pathname);


	static Path concat(const std::string& root, const std::string& leaf);


	static Path getCurrentDirectory();


	static Path getCurrentExecutable();


	static Path getTemporaryDirectory();

	static Path getUserSettingsDirectory();

	bool mkdir() const;
	bool mkdirRecursive() const;
	bool unlinkFile();
	std::vector<Path> listContents() const;

private:
	std::string m_path;
};