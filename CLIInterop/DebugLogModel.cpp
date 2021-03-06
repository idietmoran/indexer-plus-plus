// This file is the part of the Indexer++ project.
// Copyright (C) 2016 Anna Krykora <krykoraanna@gmail.com>. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.

#include "DebugLogModel.h"

#include "SearchEngineLogListener.h"

using namespace System;
using namespace Collections::ObjectModel;
using namespace System::Windows::Threading;
// clang-format off

namespace CLIInterop 
{
	DebugLogModel::DebugLogModel()
	{
		LogMessages = gcnew ObservableCollection<System::String^>();
		dispatcher = Dispatcher::CurrentDispatcher;
		listener = new SearchEngineLogListener(this);
	}

	DebugLogModel::~DebugLogModel()
	{
		delete listener;
	}

	void DebugLogModel::OnNewMessage(String^ msg)
	{
		if (dispatcher->CheckAccess())
		{
			if (LogMessages->Count > 5000 /*max_log_msg_number*/)
				Clear();

			LogMessages->Add(msg);

		}
		else
		{
			dispatcher->BeginInvoke(
				DispatcherPriority::Normal, 
				gcnew Action<String^>(this, &DebugLogModel::OnNewMessage), 
				msg);
		}
	}

	void DebugLogModel::Clear()
	{
		LogMessages->Clear();
	}
}