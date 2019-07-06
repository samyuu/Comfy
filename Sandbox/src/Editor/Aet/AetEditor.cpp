#include "AetEditor.h"
#include "FileSystem/MemoryStream.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	AetEditor::AetEditor(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
	}

	AetEditor::~AetEditor()
	{
	}

	void AetEditor::Initialize()
	{
		OpenAetSet(testAetPath);

		treeView = std::make_unique<AetTreeView>();
		treeView->Initialize();

		inspector = std::make_unique<AetInspector>();
		inspector->Initialize();

		timeline = std::make_unique<AetTimeline>();
		timeline->InitializeTimelineGuiState();

		renderWindow = std::make_unique<AetRenderWindow>();
		renderWindow->Initialize();
	}

	void AetEditor::DrawGui()
	{
		ImGui::GetCurrentWindow()->Hidden = true;
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None; // ImGuiWindowFlags_NoBackground;

		if (ImGui::Begin("AetSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawSetLoader();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Tree View##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetTreeViewChild##AetEditor", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			treeView->DrawGui(aetSet.get());
			ImGui::EndChild();
		}
		ImGui::End();

		RenderWindowBase::PushWindowPadding();
		if (ImGui::Begin("Aet Render Window##AetEditor", nullptr, windowFlags))
		{
			AetItemTypePtr selected = treeView->GetSelected();
			renderWindow->SetAetObj(treeView->GetActiveAetLyo(), selected.Type == AetSelectionType::AetObj ? selected.AetObj : nullptr);
			renderWindow->DrawGui();
		}
		ImGui::End();
		RenderWindowBase::PopWindowPadding();

		if (ImGui::Begin("Aet Inspector##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetInspectorChild##AetEditor");
			inspector->DrawGui(aetSet.get(), treeView->GetSelected());
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Properties##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetPropertiesChild##AetEditor");
			DrawProperties();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Timeline##AetEditor", nullptr))
		{
			AetItemTypePtr selected = treeView->GetSelected();
			timeline->SetAetObj(treeView->GetActiveAetLyo(), selected.Type == AetSelectionType::AetObj ? selected.AetObj : nullptr);
			timeline->DrawTimelineGui();
		}
		ImGui::End();
	}

	const char* AetEditor::GetGuiName() const
	{
		return u8"Aet Editor";
	}

	ImGuiWindowFlags AetEditor::GetWindowFlags() const
	{
		return BaseWindow::GetNoWindowFlags();
	}

	void AetEditor::DrawSetLoader()
	{
		if (fileViewer.DrawGui())
		{
			std::string aetPath = fileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(aetPath), "aet_") && EndsWithInsensitive(aetPath, ".bin"))
				OpenAetSet(aetPath.c_str());
		}
	}

	void AetEditor::DrawProperties()
	{
		AetItemTypePtr selected = treeView->GetSelected();
		if (selected.Type != AetSelectionType::AetObj || selected.AetObj == nullptr)
			return;

		if (selected.AetObj->AnimationData.Properties == nullptr)
			return;

		float frame = timeline->GetFrame().Frames();
		AetMgr::Interpolate(selected.AetObj->AnimationData, frame, &currentProperties);

		ImGui::InputFloat2("Origin", glm::value_ptr(currentProperties.Origin));
		ImGui::InputFloat2("Position", glm::value_ptr(currentProperties.Position));
		ImGui::InputFloat("Rotation", &currentProperties.Rotation);
		ImGui::InputFloat("Scale", glm::value_ptr(currentProperties.Scale));
		ImGui::InputFloat("Opacity", &currentProperties.Opcaity);
	}

	bool AetEditor::OpenAetSet(const std::string& filePath)
	{
		if (!FileExists(filePath))
			return false;

		aetSet.release();
		aetSet = std::make_unique<AetSet>();
		aetSet->Name = GetFileName(filePath, false);
		aetSet->Load(filePath);

		return true;
	}
}