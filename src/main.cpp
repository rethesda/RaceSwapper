#include "configuration/Configuration.h"
#include "Hooks.h"
#include "Utils.h"
#include "settings/Settings.h"
#include "swap/RaceSwapDatabase.h"
#include "MergeMapperPluginAPI.h"
#include "swap/RaceSwapDatabase.h"
#include "Papyrus.h"
#include "Version.h"

void MessageInterface(SKSE::MessagingInterface::Message* msg) {
	switch (msg->type) {
		case SKSE::MessagingInterface::kPostPostLoad:
		{
			logger::info("Dependencies check...");
			if (!GetModuleHandle(L"po3_Tweaks")) {
				logger::critical("po3_Tweaks not detected, mod will not function right!");
			}
			MergeMapperPluginAPI::GetMergeMapperInterface001();  // Request interface
			if (g_mergeMapperInterface) {                        // Use Interface
				const auto version = g_mergeMapperInterface->GetBuildNumber();
				logger::info("Got MergeMapper interface buildnumber {}", version);
			} else
				logger::info("MergeMapper not detected");
			logger::info("Dependencies check complete!");
			break;
		}
		case SKSE::MessagingInterface::kDataLoaded:
		{
			// TODO: Add console commands to revert appearance
			ConfigurationDatabase::GetSingleton()->Initialize();
			raceswap::DataBase::GetSingleton();
			hook::InstallHooks();
			break;
		}
	}
}

SKSEPluginInfo(
	.Version = {Project::Version::MAJOR, Project::Version::MINOR, Project::Version::PATCH},
	.Name = Project::NAME,
	.Author = Project::AUTHOR,
	.SupportEmail = "N/A",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
)

extern "C" DLLEXPORT const char* APIENTRY GetPluginVersion()
{
	return Project::Version::NAME.data();
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	Settings::GetSingleton()->Load();
	auto logLevel = spdlog::level::info;
	if (Settings::GetSingleton()->features.any(Settings::Features::kDebugLogging)) {
		logLevel = spdlog::level::debug;
	}
	const auto initInfo = SKSE::InitInfo {
		.log = true,
		.logLevel = logLevel,
		.trampoline = true,
		.trampolineSize = hook::hookTrampolineSize
	};
	SKSE::Init(a_skse, initInfo);

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageInterface);

	const auto papyrusInterface = SKSE::GetPapyrusInterface();
	papyrusInterface->Register(Papyrus::Bind);

	logger::info("Loaded Plugin");
	return true;
}
