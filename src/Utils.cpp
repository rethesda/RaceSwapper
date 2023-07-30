#include "Utils.h"
#include "PCH.h"



namespace utils
{
	std::string UniqueStringFromForm(RE::TESForm* a_form_seed)
	{
		if (!a_form_seed) {
			return std::string();
		}
		auto fileName = "DynamicForm";
		if (!a_form_seed->IsDynamicForm()) {
			fileName = a_form_seed->GetFile()->fileName;
		}
		return std::to_string(a_form_seed->GetFormID() & 0x00FFFFFF) + "_" + fileName;
	}

	size_t HashForm(RE::TESForm* a_form_seed)
	{
		std::string data = UniqueStringFromForm(a_form_seed);

		long p = 16777619;
		size_t hash = 2166136261L;
		for (int i = 0; i < data.length(); i++) {
			hash = (hash ^ data[i]) * p;
			hash += hash << 13;
			hash ^= hash >> 7;
			hash += hash << 3;
			hash ^= hash << 17;
			hash += hash >> 5;
		}

		// TODO: Use currentPlayerID to have different hash/"seed" for each playthrough
		//if (true) {
		//	hash += RE::BGSSaveLoadManager::GetSingleton()->currentPlayerID;
		//} 
		return hash;
	}

	RE::BSTArray<RE::TESNPC::Layer*>* CopyTintLayers(RE::BSTArray<RE::TESNPC::Layer*>* a_tintLayers)
	{
		if (!a_tintLayers) {
			return nullptr;
		}
		auto copiedTintLayers = utils::AllocateMemoryCleanly<RE::BSTArray<RE::TESNPC::Layer*>>();

		if (!a_tintLayers->empty()) {
			for (auto tint : *a_tintLayers) {
				auto newLayer = AllocateMemoryCleanly<RE::TESNPC::Layer>();
				newLayer->tintColor = tint->tintColor;
				newLayer->tintIndex = tint->tintIndex;
				newLayer->preset = tint->preset;
				newLayer->interpolationValue = tint->interpolationValue;
				copiedTintLayers->emplace_back(newLayer);
			}
		}

		return copiedTintLayers;
	}

	RE::TESNPC::HeadRelatedData* CopyHeadRelatedData(RE::TESNPC::HeadRelatedData* a_data)
	{
		auto newHeadData = AllocateMemoryCleanly<RE::TESNPC::HeadRelatedData>();
		if (a_data) {
			newHeadData->hairColor = a_data->hairColor;
			newHeadData->faceDetails = a_data->faceDetails;
		}
		return newHeadData;
	}

	RE::BGSHeadPart** CopyHeadParts(RE::BGSHeadPart** a_parts, std::uint32_t a_numHeadParts)
	{
		if (!a_parts) {
			return nullptr;
		}
		auto newHeadParts = AllocateMemoryCleanly<RE::BGSHeadPart*>(sizeof(void*) * a_numHeadParts);
		for (std::uint32_t index = 0; index < a_numHeadParts; index++) {
			newHeadParts[index] = a_parts[index];
		}
		return newHeadParts;
	}

	RE::TESNPC::FaceData* DeepCopyFaceData(RE::TESNPC::FaceData* a_faceData)
	{
		if (!a_faceData) {
			return nullptr;
		}

		auto newFaceData = AllocateMemoryCleanly<RE::TESNPC::FaceData>();

		for (std::uint32_t i = 0; i < 19; i++) {
			newFaceData->morphs[i] = a_faceData->morphs[i];
		}

		for (std::uint32_t i = 0; i < 4; i++) {
			newFaceData->parts[i] = a_faceData->parts[i];
		}
		return newFaceData;
	}

	std::vector<std::string> split_string(std::string& a_string, char a_delimiter)
	{
		std::vector<std::string> list;
		std::string strCopy = a_string;
		size_t pos = 0;
		std::string token;
		while ((pos = strCopy.find(a_delimiter)) != std::string::npos) {
			token = strCopy.substr(0, pos);
			list.push_back(token);
			strCopy.erase(0, pos + 1);
		}
		list.push_back(strCopy);
		return list;
	}


	std::string GetEditorID(RE::FormID a_formID)
	{
		static auto tweaks = GetModuleHandle(L"po3_Tweaks");
		static auto function = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
		if (function) {
			return function(a_formID);
		}
		return {};
	}

	std::string GetFormEditorID(const RE::TESForm* a_form)
	{
		if (!a_form) {
			return {};
		}
		if (a_form->IsDynamicForm()) {
			return a_form->GetFormEditorID();
		}
		switch (a_form->GetFormType()) {
		case RE::FormType::Keyword:
		case RE::FormType::LocationRefType:
		case RE::FormType::Action:
		case RE::FormType::MenuIcon:
		case RE::FormType::Global:
		case RE::FormType::HeadPart:
		case RE::FormType::Race:
		case RE::FormType::Sound:
		case RE::FormType::Script:
		case RE::FormType::Navigation:
		case RE::FormType::Cell:
		case RE::FormType::WorldSpace:
		case RE::FormType::Land:
		case RE::FormType::NavMesh:
		case RE::FormType::Dialogue:
		case RE::FormType::Quest:
		case RE::FormType::Idle:
		case RE::FormType::AnimatedObject:
		case RE::FormType::ImageAdapter:
		case RE::FormType::VoiceType:
		case RE::FormType::Ragdoll:
		case RE::FormType::DefaultObject:
		case RE::FormType::MusicType:
		case RE::FormType::StoryManagerBranchNode:
		case RE::FormType::StoryManagerQuestNode:
		case RE::FormType::StoryManagerEventNode:
		case RE::FormType::SoundRecord:
			return a_form->GetFormEditorID();
		default:
			return GetEditorID(a_form->GetFormID());
		}
	};

	RE::TESRace* GetValidRaceForArmorRecursive(RE::TESObjectARMO* a_armor, RE::TESRace* a_race) {
		if (a_race == nullptr) {
			return nullptr;
		}
		bool isValidRace = false;
		for (auto addon : a_armor->armorAddons) {
			if (addon->race == a_race || is_amongst(addon->additionalRaces, a_race)) {
				isValidRace = true;
				break;
			}
		}
		
		return isValidRace ? a_race : GetValidRaceForArmorRecursive(a_armor, a_race->armorParentRace);
	}

	template <class T>
	T* AllocateMemoryCleanly() {
		return AllocateMemoryCleanly<T>((std::uint32_t) sizeof(T));
	}
	
	template <class T>
	T* AllocateMemoryCleanly(std::uint32_t a_size)
	{
		auto data = RE::MemoryManager::GetSingleton()->Allocate(a_size, 0, 0);
		std::memset(data, 0, a_size);

		return reinterpret_cast<T*>(data);
	}
}
