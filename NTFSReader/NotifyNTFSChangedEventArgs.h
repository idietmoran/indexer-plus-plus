// This file is the part of the Indexer++ project.
// Copyright (C) 2016 Anna Krykora <krykoraanna@gmail.com>. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.

#pragma once

#include <map>
#include <set>
#include <memory>

#include "Macros.h"
#include "typedefs.h"

namespace indexer_common { class FileInfo; }

namespace ntfs_reader {

	// This arguments used to store and pass changes, made in the file system. The intended
	// consumer is an IndexManager, which must process this changes, namely update an Index
	// and fire index changed event.
	// This args are accumulated result of one reading session.

    class NotifyNTFSChangedEventArgs {
       public:
        NotifyNTFSChangedEventArgs() = default;

        NO_COPY(NotifyNTFSChangedEventArgs)

        // Contains newly created files.

		std::map<indexer_common::uint, indexer_common::FileInfo*> CreatedItems;


        // Contains IDs of deleted files.

		std::set<indexer_common::uint> DeletedItems;


        // Contains new FileInfo objects, that could be matched to the old ones by ID.

		std::map<indexer_common::uint, indexer_common::FileInfo*> ChangedItems;


        // Returns string representation of the arguments.

        std::wstring ToWString() const;
    };

	using uNotifyNTFSChangedEventArgs = std::unique_ptr<NotifyNTFSChangedEventArgs>;

} // namespace ntfs_reader