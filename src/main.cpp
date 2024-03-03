#include "configuration/Configuration.h"
#include "Hooks.h"
#include "Utils.h"
#include "settings/Settings.h"
#include "swap/RaceSwapDatabase.h"
#include "MergeMapperPluginAPI.h"
#include "swap/RaceSwapDatabase.h"

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
			Settings::GetSingleton()->Load();
			break;
		}
		case SKSE::MessagingInterface::kDataLoaded:
		{
			// TODO: Add console commands to revert appearance
			ConfigurationDatabase::GetSingleton()->Initialize();
			auto lock = RE::TESForm::GetAllForms().second.get();
			lock.LockForRead();
			raceswap::DataBase::GetSingleton();
			lock.UnlockForRead();
			hook::InstallHooks();
			break;
		}
	}
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifdef _DEBUG
	log->set_level(spdlog::level::debug);
	log->flush_on(spdlog::level::debug);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif
	

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("RaceSwapper");
	v.AuthorName("Nightfallstorm and Hanotak");
	v.UsesAddressLibrary(true);
	v.HasNoStructUse(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	SKSE::Init(a_skse);
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageInterface);
	logger::info("Loaded Plugin");
	return true;
}

extern "C" DLLEXPORT const char* APIENTRY GetPluginVersion()
{
	return Version::NAME.data();
}
