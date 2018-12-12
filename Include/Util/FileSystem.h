#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

namespace FileSystem
{
	typedef CHAR Byte;

	class Block
	{
	public:
		Block(
			size_t size = 0
		) : bytes_(size) {}

		Block(
			Byte* beg
			, Byte* end
		);

		VOID push_back(
			Byte b
		);

		Byte& operator[](size_t i);

		Byte operator[](size_t i) const;

		BOOL operator==(const Block&) const;

		BOOL operator!=(const Block&) const;

		size_t size() const;

	private:
		std::vector<Byte> bytes_;
	};

	class File
	{
	public:
		enum direction
		{
			in,
			out
		};

		enum type
		{
			text,
			binary
		};

		enum opt
		{
			truncate = fstream::trunc,
			append = fstream::app
		};

		File(
			const std::string& filespec
		);

		BOOL Open(
			direction dirn
			, type typ = File::text
			, opt op = File::truncate
		);

		~File();
		std::string Name();

		std::string GetLine(
			BOOL keepNewLine = FALSE
		);

		std::string ReadAll(
			BOOL KeepNewLines = FALSE
		);

		VOID PutLine(
			const std::string& line
			, BOOL wantReturn = TRUE
		);

		Block GetBlock(
			size_t size
		);

		VOID PutBlock(
			const Block&
		);

		BOOL IsGood();

		VOID Clear();

		VOID flush();

		VOID Close();

		static BOOL exists(
			const std::string& file
		);

		static BOOL copy(
			const std::string& src
			, const std::string& dst
			, BOOL failIfExists = FALSE
		);

		static BOOL Remove(
			const std::string& filespec
		);

	private:
		std::string		name_;
		std::ifstream*	pIStream;
		std::ofstream*	pOStream;
		direction		dirn_;
		type			typ_;
		opt				opt_;
		BOOL			good_;
	};

	inline std::string File::Name() { return name_; }

	class FileInfo
	{
	public:
		enum dateFormat
		{
			fullformat,
			timeformat,
			dateformat
		};

		FileInfo(
			const std::string& fileSpec
		);

		BOOL Good();

		std::string Name() const;

		std::string Date(
			dateFormat df = fullformat
		) const;

		size_t Size() const;


		BOOL IsArchive() const;

		BOOL IsCompressed() const;

		BOOL IsDirectory() const;

		BOOL IsEncrypted() const;

		BOOL IsHidden() const;

		BOOL IsNormal() const;

		BOOL isOffLine() const;

		BOOL isReadOnly() const;

		BOOL IsSystem() const;

		BOOL IsTemporary() const;


		BOOL operator<(const FileInfo& fi) const;

		BOOL operator==(const FileInfo& fi) const;

		BOOL operator>(const FileInfo& fi) const;

		BOOL Earlier(const FileInfo& fi) const;

		BOOL Later(const FileInfo& fi) const;

		BOOL Smaller(const FileInfo& fi) const;

		BOOL Larger(const FileInfo& fi) const;

	private:
		BOOL good_;
		static std::string IntToString(long i);
		WIN32_FIND_DATAA data;
	};

	class Path
	{
	public:
		static std::string GetFullFileSpec(
			const std::string& fileSpec
		);
		static std::string GetPath(
			const std::string& fileSpec
		);

		static std::string GetName(
			const std::string& fileSpec
			, BOOL withExt = TRUE
		);

		static std::string GetExt(
			const std::string& fileSpec
		);

		static std::string FileSpec(
			const std::string& path
			, const std::string& name
		);

		static std::string ToLower(
			const std::string& src
		);

		static std::string ToUpper(
			const std::string& src
		);

	};

	class Directory
	{
	public:
		static BOOL Create(
			const std::string& path
		);

		static BOOL Remove(
			const std::string& path
		);

		static BOOL Exist(
			const std::string& path
		);

		static std::string GetCurrentDirectory();

		static BOOL SetCurrentDirectory(
			const std::string& path
		);

		static std::vector<std::string> GetFiles(
			const std::string& path = "."
			, const std::string& pattern = "*.*"
		);

		static std::vector<std::string> GetDirectories(
			const std::string& path = "."
			, const std::string& pattern = "*.*"
		);

	private:
	};
}

using namespace FileSystem;