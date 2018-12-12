#pragma once

#include "stdafx.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <cctype>
#include <locale>
#include "FileSystem.h"

using namespace FileSystem;

class FileSystemSearch
{
public:
	FileSystemSearch();

	~FileSystemSearch();

	std::string FirstFile(
		const std::string& path = "."
		, const std::string& pattern = "*.*"
	);

	std::string NextFile();

	std::string FirstDirectory(
		const std::string& path = "."
		, const std::string& pattern = "*.*"
	);

	std::string NextDirectory();

	VOID close();

private:
	HANDLE hFindFile;
	WIN32_FIND_DATAA FindFileData;
	WIN32_FIND_DATAA* pFindFileData;
};

FileSystemSearch::FileSystemSearch() : pFindFileData(&FindFileData) {}
FileSystemSearch::~FileSystemSearch() { ::FindClose(hFindFile); }
VOID FileSystemSearch::close() { ::FindClose(hFindFile); }

Block::Block(
	Byte* beg
	, Byte* end
) : bytes_(beg, end) {}

VOID Block::push_back(
	Byte b
)
{
	bytes_.push_back(b);
}

Byte& Block::operator[](
	size_t i
	)
{

	if (i < 0 || bytes_.size() <= i)
		EDRException("index out of range in Block");
	return bytes_[i];
}

Byte Block::operator[](
	size_t i
	) const
{
	if (i < 0 || bytes_.size() <= i)
		EDRException("index out of range in Block");
	return bytes_[i];
}

BOOL Block::operator==(
	const Block& block
	) const
{
	return bytes_ == block.bytes_;
}

BOOL Block::operator!=(
	const Block& block
	) const
{
	return bytes_ != block.bytes_;
}

size_t Block::size() const
{
	return bytes_.size();
}


File::File(
	const std::string& filespec
)
	: name_(filespec)
	, pIStream(nullptr)
	, pOStream(nullptr)
	, dirn_(in)
	, typ_(text)
	, good_(TRUE)
{
}

File::~File()
{
	if (pIStream)
	{
		pIStream->close();
		EDRDelete(pIStream);
		pIStream = nullptr;
		good_ = FALSE;
	}
	if (pOStream)
	{
		pOStream->close();
		EDRDelete(pOStream);
		pOStream = nullptr;
		good_ = FALSE;
	}
}

BOOL File::Open(
	direction dirn
	, type typ
	, opt op

)
{
	dirn_ = dirn;
	typ_ = typ;
	opt_ = op;
	good_ = TRUE;
	if (dirn == in)
	{
		pIStream = EDRNew std::ifstream;
		if (typ == binary)
			pIStream->open(name_.c_str(), std::ios::in | std::ios::binary);
		else
			pIStream->open(name_.c_str(), std::ios::in);
		if (!(*pIStream).good())
		{
			good_ = FALSE;
			pIStream = nullptr;
		}
	}
	else
	{
		pOStream = EDRNew std::ofstream;
		if (typ == binary)
			pOStream->open(name_.c_str(), std::ios::out | std::ios::binary | op);
		else
			pOStream->open(name_.c_str(), std::ios::out | op);
		if (!(*pOStream).good())
		{
			good_ = FALSE;
			pOStream = nullptr;
		}
	}
	return good_;
}

std::string File::GetLine(
	BOOL keepNewLines
)
{
	if (pIStream == nullptr || !pIStream->good())
		EDRException("input stream not Open");
	if (typ_ == binary)
		EDRException("getting text line from binary file");
	if (dirn_ == out)
		EDRException("reading output file");

	std::string store;
	while (TRUE)
	{
		CHAR ch = pIStream->get();
		if (!IsGood())
			return store;
		if (ch == '\n')
		{
			if (keepNewLines)
				store += ch;
			return store;
		}
		store += ch;
	}
}

std::string File::ReadAll(
	BOOL keepNewLines
)
{
	std::string store;
	while (TRUE)
	{
		if (!IsGood())
			return store;
		store += GetLine(keepNewLines);
		std::locale loc;
		
		/*
		if (store.size() > 0 && !std::isspace(store[store.size() - 1], loc))
			store += ' ';
		*/
	}
	return store;
}

VOID File::PutLine(
	const std::string& s
	, BOOL wantReturn
)
{
	if (pOStream == nullptr || !pOStream->good())
		EDRException("output stream not Open");
	if (typ_ == binary)
		EDRException("writing text line to binary file");
	if (dirn_ == in)
		EDRException("writing input file");
	for (size_t i = 0; i < s.size(); ++i)
		pOStream->put(s[i]);
	if (wantReturn)
		pOStream->put('\n');
	pOStream->flush();
}

Block File::GetBlock(
	size_t size
)
{
	if (pIStream == nullptr || !pIStream->good())
		EDRException("input stream not Open");
	if (typ_ != binary)
		EDRException("reading binary from text file");
	if (dirn_ == out)
		EDRException("reading output file");
	Block blk;
	if (pIStream)
	{
		for (size_t i = 0; i < size; ++i)
		{
			Byte b;
			pIStream->get(b);
			if (pIStream->good())
				blk.push_back(b);
			else
				break;
		}
	}
	return blk;
}

VOID File::PutBlock(
	const Block& blk
)
{
	if (pOStream == nullptr || !pOStream->good())
		EDRException("output stream not Open");
	if (typ_ != binary)
		EDRException("writing binary to text file");
	if (dirn_ == in)
		EDRException("writing input file");
	if (!pOStream->good())
		return;
	for (size_t i = 0; i < blk.size(); ++i)
	{
		pOStream->put(blk[i]);
	}
}

BOOL File::IsGood()
{
	if (!good_)
		return FALSE;
	if (pIStream != nullptr)
		return (good_ = pIStream->good());
	if (pOStream != nullptr)
		return (good_ = pOStream->good());
	return (good_ = FALSE);
}

VOID File::flush()
{
	if (pOStream != nullptr && pOStream->good())
		pOStream->flush();
}

VOID File::Clear()
{
	if (pIStream != nullptr)
		pIStream->clear();
	if (pOStream != nullptr)
		pOStream->clear();
}

VOID File::Close()
{
	File::flush();
	if (pIStream != nullptr)
	{
		pIStream->close();
		pIStream = nullptr;
		good_ = FALSE;
	}
	if (pOStream)
	{
		pOStream->close();
		pOStream = nullptr;
		good_ = FALSE;
	}
}

BOOL File::exists(
	const std::string& file
)
{
	return ::GetFileAttributesA(file.c_str()) != INVALID_FILE_ATTRIBUTES;
}

BOOL File::copy(
	const std::string& src
	, const std::string& dst
	, BOOL failIfExists
)
{
	return ::CopyFileA(src.c_str(), dst.c_str(), failIfExists) != 0;
}

BOOL File::Remove(
	const std::string& file
)
{
	return ::DeleteFileA(file.c_str()) != 0;
}

FileInfo::FileInfo(
	const std::string& fileSpec
)
{
	HANDLE hFile = ::FindFirstFileA(fileSpec.c_str(), &data);
	if (hFile == INVALID_HANDLE_VALUE)
		good_ = FALSE;
	else
		good_ = TRUE;
}

BOOL FileInfo::Good()
{
	return good_;
}

std::string FileInfo::Name() const
{
	return Path::GetName(data.cFileName);
}

std::string FileInfo::IntToString(
	long i
)
{
	std::ostringstream out;
	out.fill('0');
	out << std::setw(2) << i;
	return out.str();
}

std::string FileInfo::Date(
	dateFormat df
) const
{
	std::string dateStr, timeStr;
	FILETIME ft;
	SYSTEMTIME st;
	::FileTimeToLocalFileTime(&data.ftLastWriteTime, &ft);
	::FileTimeToSystemTime(&ft, &st);
	dateStr = IntToString(st.wMonth) + CharSlash + IntToString(st.wDay) + CharSlash + IntToString(st.wYear);
	timeStr = IntToString(st.wHour) + ':' + IntToString(st.wMinute) + ':' + IntToString(st.wSecond);
	if (df == dateformat)
		return dateStr;
	if (df == timeformat)
		return timeStr;
	return dateStr + " " + timeStr;
}

size_t FileInfo::Size() const
{
	return (size_t)(data.nFileSizeLow + (data.nFileSizeHigh << 8));
}

BOOL FileInfo::IsArchive() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0;
}

BOOL FileInfo::IsCompressed() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0;
}

BOOL FileInfo::IsDirectory() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

BOOL FileInfo::IsEncrypted() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0;
}

BOOL FileInfo::IsHidden() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
}

BOOL FileInfo::IsNormal() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) != 0;
}

BOOL FileInfo::isOffLine() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0;
}

BOOL FileInfo::isReadOnly() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
}

BOOL FileInfo::IsSystem() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
}

BOOL FileInfo::IsTemporary() const
{
	return (data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0;
}

BOOL FileInfo::operator<(
	const FileInfo& fi
	) const
{
	return strcmp(data.cFileName, fi.data.cFileName) == -1;
}

BOOL FileInfo::operator==(
	const FileInfo& fi
	) const
{
	return strcmp(data.cFileName, fi.data.cFileName) == 0;
}

BOOL FileInfo::operator>(
	const FileInfo& fi
	) const
{
	return strcmp(data.cFileName, fi.data.cFileName) == 1;
}

BOOL FileInfo::Earlier(
	const FileInfo& fi
) const
{
	FILETIME ft1 = data.ftLastWriteTime;
	FILETIME ft2 = fi.data.ftLastWriteTime;
	return ::CompareFileTime(&ft1, &ft2) == -1;
}

BOOL FileInfo::Later(
	const FileInfo& fi
) const
{
	FILETIME ft1 = data.ftLastWriteTime;
	FILETIME ft2 = fi.data.ftLastWriteTime;
	return ::CompareFileTime(&ft1, &ft2) == 1;
}

BOOL FileInfo::Smaller(
	const FileInfo &fi
) const
{
	return Size() < fi.Size();
}

BOOL FileInfo::Larger(
	const FileInfo &fi
) const
{
	return Size() > fi.Size();
}

std::string Path::ToLower(
	const std::string& src
)
{
	std::string temp;
	for (size_t i = 0; i < src.length(); ++i)
		temp += tolower(src[i]);
	return temp;
}

std::string Path::ToUpper(
	const std::string& src
)
{
	std::string temp;
	for (size_t i = 0; i < src.length(); ++i)
		temp += toupper(src[i]);
	return temp;
}

std::string Path::GetName(
	const std::string &fileSpec
	, BOOL withExt
)
{
	size_t pos = fileSpec.find_last_of(StringSlash);
	if (pos >= fileSpec.length())
	{
		pos = fileSpec.find_last_of(PathDelimeter);
		if (pos >= fileSpec.length())
		{
			// no path prepended
			if (withExt)
				return fileSpec;
			else
			{
				// remove ext
				size_t pos = fileSpec.find(StringDot);
				if (pos > fileSpec.size())
					return fileSpec;
				return fileSpec.substr(0, pos - 1);
			}
		}
	}
	if (withExt)
		return fileSpec.substr(pos + 1, fileSpec.length() - pos);
	else
	{
		// remove ext
		size_t pos2 = fileSpec.find(StringDot, pos);
		if (pos2 > fileSpec.size())
			// no ext
			return fileSpec.substr(pos + 1);
		return fileSpec.substr(pos + 1, pos2 - pos - 1);
	}
}

std::string Path::GetExt(
	const std::string& fileSpec
)
{
	size_t pos1 = fileSpec.find_last_of(CharSlash);
	size_t pos2 = fileSpec.find_last_of(CharBackslash);
	size_t pos = fileSpec.find_last_of(CharDot);
	// handle ../ or ..\\ with no extension
	if (pos1 < fileSpec.length() || pos2 < fileSpec.length())
	{
		if (pos < min(pos1, pos2))
			return std::string(StringNull);
	}
	// only . is extension delimiter
	if (0 <= pos && pos < fileSpec.length())
		return ToLower(fileSpec.substr(pos + 1, fileSpec.length() - pos));
	return std::string(StringNull);
}

std::string Path::GetPath(
	const std::string &fileSpec
)
{
	size_t pos = fileSpec.find_last_of(StringSlash);
	if (pos >= fileSpec.length())
		pos = fileSpec.find_last_of(PathDelimeter);
	if (pos >= fileSpec.length())
		return StringDot;
	if (fileSpec.find(StringDot, pos + 1))
		return fileSpec.substr(0, pos + 1);
	return fileSpec;
}

std::string Path::GetFullFileSpec(
	const std::string &fileSpec
)
{
	const size_t BufSize = 256;
	CHAR buffer[BufSize];
	CHAR filebuffer[BufSize];
	CHAR* name = filebuffer;
	::GetFullPathNameA(fileSpec.c_str(), BufSize, buffer, &name);
	return std::string(buffer);
}

std::string Path::FileSpec(
	const std::string &path
	, const std::string &name
)
{
	std::string fs;
	size_t len = path.size();
	if (path[len - 1] == CharSlash || path[len - 1] == CharBackslash)
		fs = path + name;
	else
	{
		if (path.find(StringSlash) < path.size())
			fs = path + StringSlash + name;
		else if (path.find(PathDelimeter) < path.size())
			fs = path + PathDelimeter + name;
		else
			fs = path + StringSlash + name;
	}
	return fs;
}

std::string Directory::GetCurrentDirectory()
{
	CHAR buffer[MAX_PATH];
	::GetCurrentDirectoryA(MAX_PATH, buffer);
	return std::string(buffer);
}

BOOL Directory::SetCurrentDirectory(
	const std::string& path
)
{
	return ::SetCurrentDirectoryA(path.c_str()) != 0;
}

std::vector<std::string> Directory::GetFiles(
	const std::string& path
	, const std::string& pattern
)
{
	std::vector<std::string> files;
	FileSystemSearch fss;
	std::string file = fss.FirstFile(path, pattern);
	if (file.size() == 0)
		return files;
	files.push_back(file);
	while (TRUE)
	{
		file = fss.NextFile();
		if (file.size() == 0)
			return files;
		files.push_back(file);
	}
	return files;
}

std::vector<std::string> Directory::GetDirectories(
	const std::string& path
	, const std::string& pattern
)
{
	std::vector<std::string> dirs;
	FileSystemSearch fss;
	std::string dir = fss.FirstDirectory(path, pattern);
	if (dir.size() == 0)
		return dirs;
	dirs.push_back(dir);
	while (TRUE)
	{
		dir = fss.NextDirectory();
		if (dir.size() == 0)
			return dirs;
		dirs.push_back(dir);
	}
	return dirs;
}

BOOL Directory::Create(
	const std::string& path
)
{
	return ::CreateDirectoryA(path.c_str(), NULL) == 0;
}

BOOL Directory::Exist(
	const std::string& path
)
{
	DWORD dwAttrib = GetFileAttributesA(path.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL Directory::Remove(
	const std::string& path
)
{
	return ::RemoveDirectoryA(path.c_str()) == 0;
}

std::string FileSystemSearch::FirstFile(
	const std::string& path
	, const std::string& pattern
)
{
	hFindFile = ::FindFirstFileA(Path::FileSpec(path, pattern).c_str(), pFindFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		if (!(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return pFindFileData->cFileName;
		else
			while (::FindNextFileA(hFindFile, pFindFileData))
				if (!(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					return pFindFileData->cFileName;
	}
	return StringNull;
}

std::string FileSystemSearch::NextFile()
{
	while (::FindNextFileA(hFindFile, pFindFileData))
		if (!(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return pFindFileData->cFileName;
	return StringNull;
}

std::string FileSystemSearch::FirstDirectory(
	const std::string& path
	, const std::string& pattern
)
{
	hFindFile = ::FindFirstFileA(Path::FileSpec(path, pattern).c_str(), pFindFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return pFindFileData->cFileName;
		else
			while (::FindNextFileA(hFindFile, pFindFileData))
				if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					return pFindFileData->cFileName;
	}
	return StringNull;
}

std::string FileSystemSearch::NextDirectory()
{
	while (::FindNextFileA(hFindFile, pFindFileData))
		if (pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return pFindFileData->cFileName;
	return StringNull;
}

