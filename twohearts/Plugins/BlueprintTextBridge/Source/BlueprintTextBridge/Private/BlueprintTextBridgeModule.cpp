#include "BlueprintTextBridgeModule.h"

#include "BlueprintEditor.h"
#include "BlueprintEditorContext.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphUtilities.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/Attribute.h"
#include "Styling/AppStyle.h"
#include "ToolMenu.h"
#include "ToolMenuDelegates.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "BlueprintTextBridge"

namespace BlueprintTextBridge
{
	static constexpr TCHAR BlueprintFence[] = TEXT("```blueprint");

	static FBlueprintEditor* GetBlueprintEditor(const FToolMenuContext& Context)
	{
		if (const UBlueprintEditorToolMenuContext* BlueprintContext = Context.FindContext<UBlueprintEditorToolMenuContext>())
		{
			if (BlueprintContext->BlueprintEditor.IsValid())
			{
				return BlueprintContext->BlueprintEditor.Pin().Get();
			}
		}

		return nullptr;
	}

	static void ShowNotification(const FText& Message, const SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Message);
		Info.ExpireDuration = 4.0f;
		Info.bUseSuccessFailIcons = true;
		Info.bUseLargeFont = false;

		if (TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(State);
		}
	}

	static FString BuildAIPrompt(FBlueprintEditor* BlueprintEditor, const FString& ExportedText)
	{
		const UBlueprint* Blueprint = BlueprintEditor ? BlueprintEditor->GetBlueprintObj() : nullptr;
		const UEdGraph* Graph = BlueprintEditor ? BlueprintEditor->GetFocusedGraph() : nullptr;
		const FString BlueprintName = Blueprint ? Blueprint->GetPathName() : TEXT("(Unknown Blueprint)");
		const FString GraphName = Graph ? Graph->GetName() : TEXT("(Unknown Graph)");

		return FString::Printf(
			TEXT("你是 UE5 蓝图剪贴板文本助手。\n")
			TEXT("你的任务是根据我的修改要求，编辑下面这段 Unreal Engine 蓝图剪贴板文本，并返回一段可直接粘贴回蓝图图表的结果。\n\n")
			TEXT("硬性规则：\n")
			TEXT("1. 只返回一个 ```blueprint 代码块。\n")
			TEXT("2. 代码块里只能放原始 Unreal 蓝图剪贴板文本，从 Begin Object 开始，到最后一个 End Object 结束。\n")
			TEXT("3. 不要添加解释、标题、列表、Markdown 引号，代码块内也不要出现省略号。\n")
			TEXT("4. 优先在现有节点基础上修改；除非我明确要求，否则不要随意改动无关节点、变量名、函数名、Pin 名或连线结构。\n")
			TEXT("5. 如果需要引用当前蓝图里不存在的变量、函数、组件、宏或自定义类型，请尽量复用现有名字；无法确认时仍返回最接近可粘贴的完整文本。\n\n")
			TEXT("当前蓝图：%s\n")
			TEXT("当前图表：%s\n\n")
			TEXT("修改要求：\n")
			TEXT("[把你的需求写在这里]\n\n")
			TEXT("原始蓝图文本：\n")
			TEXT("```blueprint\n")
			TEXT("%s\n")
			TEXT("```\n"),
			*BlueprintName,
			*GraphName,
			*ExportedText);
	}

	static bool ExtractBlueprintPayload(const FString& Input, FString& OutPayload)
	{
		FString Working = Input;
		Working.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
		Working.ReplaceInline(TEXT("\r"), TEXT("\n"));

		const int32 BeginObjectIndex = Working.Find(TEXT("Begin Object"), ESearchCase::CaseSensitive);
		const int32 EndObjectIndex = Working.Find(TEXT("End Object"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);

		if (BeginObjectIndex == INDEX_NONE || EndObjectIndex == INDEX_NONE || EndObjectIndex < BeginObjectIndex)
		{
			return false;
		}

		const int32 EndObjectLength = FString(TEXT("End Object")).Len();
		OutPayload = Working.Mid(BeginObjectIndex, EndObjectIndex - BeginObjectIndex + EndObjectLength).TrimStartAndEnd();
		return !OutPayload.IsEmpty();
	}

	static bool CanCopyForAI(const FToolMenuContext& Context)
	{
		if (FBlueprintEditor* BlueprintEditor = GetBlueprintEditor(Context))
		{
			return BlueprintEditor->CanCopyNodes();
		}

		return false;
	}

	static void CopyForAI(const FToolMenuContext& Context)
	{
		FBlueprintEditor* BlueprintEditor = GetBlueprintEditor(Context);
		if (!BlueprintEditor || !BlueprintEditor->CanCopyNodes())
		{
			ShowNotification(LOCTEXT("CopyFailedNoSelection", "Select one or more Blueprint nodes first."), SNotificationItem::CS_Fail);
			return;
		}

		BlueprintEditor->CopySelectedNodes();

		FString ExportedText;
		FPlatformApplicationMisc::ClipboardPaste(ExportedText);
		if (ExportedText.IsEmpty())
		{
			ShowNotification(LOCTEXT("CopyFailedClipboard", "Blueprint text export failed."), SNotificationItem::CS_Fail);
			return;
		}

		const FString Prompt = BuildAIPrompt(BlueprintEditor, ExportedText);
		FPlatformApplicationMisc::ClipboardCopy(*Prompt);

		ShowNotification(LOCTEXT("CopySuccess", "AI prompt copied. Paste it into your AI chat and describe the change you want."), SNotificationItem::CS_Success);
	}

	static bool CanPasteFromAI(const FToolMenuContext& Context)
	{
		FBlueprintEditor* BlueprintEditor = GetBlueprintEditor(Context);
		if (!BlueprintEditor)
		{
			return false;
		}

		UEdGraph* FocusedGraph = BlueprintEditor->GetFocusedGraph();
		if (!FocusedGraph)
		{
			return false;
		}

		FString ClipboardText;
		FPlatformApplicationMisc::ClipboardPaste(ClipboardText);

		FString Payload;
		return ExtractBlueprintPayload(ClipboardText, Payload)
			&& FEdGraphUtilities::CanImportNodesFromText(FocusedGraph, Payload);
	}

	static void PasteFromAI(const FToolMenuContext& Context)
	{
		FBlueprintEditor* BlueprintEditor = GetBlueprintEditor(Context);
		if (!BlueprintEditor)
		{
			ShowNotification(LOCTEXT("PasteFailedEditor", "Open a Blueprint graph before importing AI output."), SNotificationItem::CS_Fail);
			return;
		}

		FString ClipboardText;
		FPlatformApplicationMisc::ClipboardPaste(ClipboardText);

		FString Payload;
		if (!ExtractBlueprintPayload(ClipboardText, Payload))
		{
			ShowNotification(LOCTEXT("PasteFailedExtract", "No Blueprint clipboard text was found in the clipboard."), SNotificationItem::CS_Fail);
			return;
		}

		UEdGraph* FocusedGraph = BlueprintEditor->GetFocusedGraph();
		if (!FocusedGraph || !FEdGraphUtilities::CanImportNodesFromText(FocusedGraph, Payload))
		{
			ShowNotification(LOCTEXT("PasteFailedValidate", "The AI output is not compatible with the current Blueprint graph."), SNotificationItem::CS_Fail);
			return;
		}

		FPlatformApplicationMisc::ClipboardCopy(*Payload);
		BlueprintEditor->PasteNodes();

		ShowNotification(LOCTEXT("PasteSuccess", "Blueprint nodes imported from AI output."), SNotificationItem::CS_Success);
	}

	static void PopulateToolbar(FToolMenuSection& Section)
	{
		const UBlueprintEditorToolMenuContext* Context = Section.FindContext<UBlueprintEditorToolMenuContext>();
		if (!Context || !Context->BlueprintEditor.IsValid() || !Context->GetBlueprintObj())
		{
			return;
		}

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"BlueprintTextBridge.CopyForAI",
			FToolUIAction(
				FToolMenuExecuteAction::CreateStatic(&CopyForAI),
				FToolMenuCanExecuteAction::CreateStatic(&CanCopyForAI),
				FToolMenuGetActionCheckState()),
			LOCTEXT("CopyForAILabel", "Copy For AI"),
			LOCTEXT("CopyForAITooltip", "Copy selected Blueprint nodes as a ready-to-send AI prompt."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Copy")));

		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"BlueprintTextBridge.PasteFromAI",
			FToolUIAction(
				FToolMenuExecuteAction::CreateStatic(&PasteFromAI),
				FToolMenuCanExecuteAction::CreateStatic(&CanPasteFromAI),
				FToolMenuGetActionCheckState()),
			LOCTEXT("PasteFromAILabel", "Paste From AI"),
			LOCTEXT("PasteFromAITooltip", "Extract Blueprint clipboard text from the clipboard and paste it into the current graph."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.PasteHere")));
	}
}

void FBlueprintTextBridgeModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBlueprintTextBridgeModule::RegisterMenus));
}

void FBlueprintTextBridgeModule::ShutdownModule()
{
	if (UToolMenus::IsAvailable())
	{
		UToolMenus::UnregisterOwner(this);
	}
}

void FBlueprintTextBridgeModule::RegisterMenus()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("AssetEditor.BlueprintEditor.ToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("BlueprintTextBridge");
	Section.InsertPosition = FToolMenuInsert("Script", EToolMenuInsertType::After);
	Section.AddDynamicEntry("BlueprintTextBridge.DynamicToolbar", FNewToolMenuSectionDelegate::CreateStatic(&BlueprintTextBridge::PopulateToolbar));
}

IMPLEMENT_MODULE(FBlueprintTextBridgeModule, BlueprintTextBridge)

#undef LOCTEXT_NAMESPACE
