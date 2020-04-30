#pragma once
#include "Types.h"
#include "../Direct3D.h"
#include "Time/TimeSpan.h"
#include <optional>

namespace Comfy::Graphics::D3D11
{
	class OcclusionQuery final : IGraphicsResource
	{
	public:
		OcclusionQuery();
		OcclusionQuery(const char* debugName);
		~OcclusionQuery() = default;

	public:
		// NOTE: Call before issuing draw calls
		void BeginQuery();
		// NOTE: Call after issuing draw calls
		void EndQuery();

		// NOTE: Call after having begun and then ended the query
		void QueryData();

		bool IsFirstQuery() const;
		bool HasCoveredPixelsReady() const;
		u64 GetCoveredPixels() const;

	private:
		// NOTE: In case requersting the data takes too long or worst case never succeeds at all
		static constexpr TimeSpan getDataSafetyTimeout = TimeSpan::FromMilliseconds(75.0);

		bool isMidQuery = false;
		bool isFirstQuery = true;

		std::optional<u64> coveredPixels, lastCoveredPixels;

		D3D11_QUERY_DESC queryDescription;
		ComPtr<ID3D11Query> query;
	};
}
