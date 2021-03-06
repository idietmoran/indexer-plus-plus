// This file is the part of the Indexer++ project.
// Copyright (C) 2016 Anna Krykora <krykoraanna@gmail.com>. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.

#pragma once

#include <vector>

#include "Macros.h"

#include "FileInfoComparatorFactory.h"

namespace indexer_common {
	class FileInfo;
	class Log;
}

namespace indexer {

    enum class SortingProperty;

    class Sorter {

       public:
        Sorter(SortingProperty prop, int direction);

        NO_COPY(Sorter);

        void ResetSortingProperties(SortingProperty prop, int direction);

        // Parameter: |file_info| - a collection of FileInfos to sort.
        void Sort(std::vector<const indexer_common::FileInfo*>* file_info) const;

        PropertyComparatorFunc GetCurrentPropertyComparator() const;

       private:
        void SortByExtensionOrType(std::vector<const indexer_common::FileInfo*>* file_infos) const;

        void SortParallel(std::vector<const indexer_common::FileInfo*>* file_infos, PropertyComparatorFunc cmp) const;

        static const int kMaxFilesCanSortByPath = 50000;

        static const int kMinNumberOfFileInfosToSortParallel = 10000;

        SortingProperty prop_;

        int direction_;

		indexer_common::Log* logger_;
    };

} // namespace indexer