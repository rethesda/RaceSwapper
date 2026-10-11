#pragma once

namespace hook
{
	constexpr auto hookTrampolineSize = 18 * 0x14; // TODO: Not accurate, but has enough grace room for all 18 hooks
	void InstallHooks();
}
