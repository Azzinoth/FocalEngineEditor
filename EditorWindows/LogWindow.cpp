#include "../FEEditor.h"

void FEEditor::DisplayLogWindow() const
{
	if (!bLogWindowVisible)
		return;

	const auto TopicList = LOG.GetTopicList();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Log", nullptr, ImGuiWindowFlags_None);

	static std::string SelectedChannel = "FE_LOG_GENERAL";
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
	ImGui::Text("Channel:");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
	ImGui::SetNextItemWidth(200);
	if (ImGui::BeginCombo("##Channel", (SelectedChannel.empty() ? "ALL" : SelectedChannel).c_str(), ImGuiWindowFlags_None))
	{
		for (int i = -1; i < int(TopicList.size()); i++)
		{
			ImGui::PushID(i);

			if (i == -1)
			{
				const bool bIsSelected = (SelectedChannel.empty());

				if (ImGui::Selectable("ALL", bIsSelected))
				{
					SelectedChannel = "";
				}

				if (bIsSelected)
					ImGui::SetItemDefaultFocus();
			}
			else
			{
				const bool bIsSelected = (SelectedChannel == TopicList[i]);
				if (ImGui::Selectable(TopicList[i].c_str(), bIsSelected))
				{
					SelectedChannel = TopicList[i];
				}

				if (bIsSelected)
					ImGui::SetItemDefaultFocus();
			}
			
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	std::string LogMessages;
	std::vector<LogItem> LogItems;

	if (SelectedChannel.empty())
	{
		std::vector<LogItem> TempItems;
		for (int i = 0; i < int(TopicList.size()); i++)
		{
			TempItems = LOG.GetLogItems(TopicList[i]);
			for (size_t j = 0; j < TempItems.size(); j++)
			{
				LogItems.push_back(TempItems[j]);
			}
		}
	}
	else
	{
		LogItems = LOG.GetLogItems(SelectedChannel);
	}

	std::sort(LogItems.begin(), LogItems.end(),
	[](const LogItem& A, const LogItem& B) -> bool
	{
		return A.TimeStamp < B.TimeStamp;
	});

	for (size_t i = 0; i < LogItems.size(); i++)
	{
		LogMessages += LogItems[i].Text;

		if (LogItems[i].Count < 1000)
		{
			LogMessages += " | COUNT: " + std::to_string(LogItems[i].Count);
		}
		else
		{
			LogMessages += " | COUNT: 1000+(Suppressed)";
		}

		LogMessages += " | SEVERITY: " + LOG.SeverityLevelToString(LogItems[i].Severity);

		if (SelectedChannel.empty())
		{
			LogMessages += " | CHANNEL: " + LogItems[i].Topic;
		}

		if (i < LogItems.size() - 1)
			LogMessages += "\n";
	}

	/*static TextEditor LogEditor;
	LogEditor.SetReadOnly(true);
	LogEditor.SetShowWhitespaces(false);
	LogEditor.SetText(LogMessages);
	LogEditor.Update("Log messages");*/

	static char LogMessages_[1000000];
	strcpy_s(LogMessages_, LogMessages.c_str());
	ImGui::InputTextMultiline("Log messages", LogMessages_, 1000000, ImVec2(ImGui::GetWindowWidth() - 30, ImGui::GetWindowHeight() - 100), ImGuiInputTextFlags_ReadOnly);

	ImGui::PopStyleVar();
	ImGui::End();
}