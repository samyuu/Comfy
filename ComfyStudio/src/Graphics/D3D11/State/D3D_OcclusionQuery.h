#pragma once
#include "../Direct3D.h"
#include "Core/TimeSpan.h"
#include <optional>

namespace Comfy::Graphics
{
	class D3D_OcclusionQuery final : ID3DGraphicsResource
	{
	public:
		D3D_OcclusionQuery();
		D3D_OcclusionQuery(const char* debugName);
		~D3D_OcclusionQuery() = default;

	public:
		// NOTE: Call before issuing draw calls
		void BeginQuery();
		// NOTE: Call after issuing draw calls
		void EndQuery();

		// NOTE: Call after having begun and then ended the query
		void QueryData();

		bool IsFirstQuery() const;
		bool HasCoveredPixelsReady() const;
		uint64_t GetCoveredPixels() const;

	private:
		// NOTE: In case requersting the data takes too long or worst case never succeeds at all
		static constexpr TimeSpan getDataSafetyTimeout = TimeSpan::FromMilliseconds(75.0);

		bool isMidQuery = false;
		bool isFirstQuery = true;

		std::optional<uint64_t> coveredPixels, lastCoveredPixels;

		D3D11_QUERY_DESC queryDescription;
		ComPtr<ID3D11Query> query;
	};
}
