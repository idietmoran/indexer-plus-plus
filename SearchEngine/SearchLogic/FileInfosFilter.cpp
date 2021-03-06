// This file is the part of the Indexer++ project.
// Copyright (C) 2016 Anna Krykora <krykoraanna@gmail.com>. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.

#include "FileInfosFilter.h"

#include "../Common/Helpers.h"

#include "IndexManagersContainer.h"
#include "LetterCaseMatching.h"
#include "TextComparison.h"

namespace indexer {

	using std::unique_ptr;
	using std::make_unique;
	using std::vector;

	using namespace indexer_common;

	FileInfosFilter::FileInfosFilter() : buffer_(make_unique<char[]>(kBufferSize)), match_case_table_(nullptr) {
	}

	FileInfosFilter::~FileInfosFilter() = default;

	void FileInfosFilter::ResetQuery(uSearchQuery last_query) {

		query_.swap(last_query);

		match_case_table_ = query_->MatchCase ? GetIdentityTable() : GetLowerMatchTable();

		if (!query_->UseRegex) {
			filename_filter_ = ParseFilenameQuery(query_->Text);
			return;
		}

		// Create and compile re2 object.

		auto empty_reg_exp = make_unique<re2::RE2>("");
		unique_ptr<re2::RE2> reg_exp;

		bool ok = helpers::Utf16ToUtf8(query_->Text, buffer_.get(), kBufferSize);

		if (ok) {
			reg_exp = make_unique<re2::RE2>(buffer_.get());
			ok = reg_exp->ok();  // compiled
		}

		re_.swap(ok ? reg_exp : empty_reg_exp);
	}

	bool FileInfosFilter::PassesAllQueryFilters(const FileInfo& fi) const {

		auto res = PassesFilterByFilename(fi) && PassesSizeDatesFilter(fi) && PassesAttributesFilter(fi);
		return res;
	}

	bool FileInfosFilter::PassesFilterByFilename(const FileInfo& fi) const {

		// Use regex engine:
		if (query_->UseRegex) {
			if (!helpers::Utf16ToUtf8(fi.GetName(), buffer_.get(), kBufferSize)) return false;

			return re2::RE2::PartialMatch(buffer_.get(), *re_);
		}

		// Use Indexer++ implemented search (only with * ):
		if (filename_filter_->MinLength == 0) {
			return true;
		}

		if (fi.NameLength < filename_filter_->MinLength) {
			return false;
		}

		if (filename_filter_->NChars.size() == 0)  // No wildcards.
		{
			return search(fi.GetName(), filename_filter_->Strs[0], match_case_table_) != nullptr;
		}

		int index = 0;
		for (auto i = 0; i < filename_filter_->NStrs; ++i) {
			auto* res = search(fi.GetName() + index, filename_filter_->Strs[i], match_case_table_);

			if (res == nullptr) return false;

			index = (res - fi.GetName()) + filename_filter_->StrLengths[i];
		}

		return true;
	}

	bool FileInfosFilter::PassesSizeDatesFilter(const FileInfo& fi) const {

		bool res = fi.SizeReal >= query_->SizeFrom && fi.SizeReal <= query_->SizeTo &&
			fi.CreationTime >= query_->CTimeFrom && fi.CreationTime <= query_->CTimeTo &&
			fi.LastAccessTime >= query_->ATimeFrom && fi.LastAccessTime <= query_->ATimeTo &&
			fi.LastWriteTime >= query_->MTimeFrom && fi.LastWriteTime <= query_->MTimeTo;

		return res;
	}

	bool FileInfosFilter::PassesAttributesFilter(const FileInfo& fi) const {

		if (query_->ExcludeHiddenAndSystem && fi.IsHiddenOrSystem()) {
			return false;
		}

		if (query_->ExcludeFolders && fi.IsDirectory()) {
			return false;
		}

		if (query_->ExcludeFiles && !fi.IsDirectory()) {
			return false;
		}

		return true;
	}

	bool FileInfosFilter::TraverseCurrentDir(const FileInfo& dir) const {

		bool not_include = query_->ExcludeHiddenAndSystem && dir.IsHiddenOrSystem();
		return !not_include;
	}

	bool FileInfosFilter::FilePassesSearchDirPathFilter(const FileInfo& fi) const {

		if (!SearchInDirectorySpecified()) return true;

		// Check if the file belongs to directory in query filter.

		auto lookup_dir = IndexManagersContainer::Instance().GetFileInfoByPath(query_->SearchDirPath);
		if (!lookup_dir) return true;

		auto root_id = IndexManagersContainer::Instance().GetIndexRootID(fi.DriveLetter);

		const FileInfo* current = &fi;

		while (current->ID != root_id) {

			if (current->ID == lookup_dir->ID) return true;
			current = current->Parent;
		}

		return false;
	}

	bool FileInfosFilter::FilePassesExcludeHiddenAndSystemFilter(const FileInfo& fi) const {

		if (!query_->ExcludeHiddenAndSystem) return true;

		const FileInfo* current = &fi;
		while (current->ID != current->ParentID)  // True only for the index root.
		{
			if (current->IsHiddenOrSystem()) return false;

			current = current->Parent;
		}

		return true;
	}

	vector<const FileInfo*> FileInfosFilter::FilterFiles(const vector<const FileInfo*>& input) {

		vector<const FileInfo*> filtered_items;

		if (input.size() == 0) return filtered_items;

		for (auto item : input) {
			if (
				// Check if the file and its parents pass exclude hidden filter.
				FilePassesExcludeHiddenAndSystemFilter(*item) &&
				// Check if the file is inside filter directory.
				FilePassesSearchDirPathFilter(*item) && PassesAllQueryFilters(*item)) {
				filtered_items.push_back(item);
			}
		}

		return filtered_items;
	}

	bool FileInfosFilter::SearchInDirectorySpecified() const {
		return !query_->SearchDirPath.empty();
	}

	const FileInfo* FileInfosFilter::GetSearchDirectory() const {
		return IndexManagersContainer::Instance().GetFileInfoByPath(query_->SearchDirPath.c_str());
	}

} // namespace indexer