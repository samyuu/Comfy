#pragma once
#include "../Direct3D.h"
#include "Core/TimeSpan.h"

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

		uint64_t GetCoveredPixels() const;

	private:
		// NOTE: In case requersting the data takes too long or worst case never succeeds at all
		static constexpr TimeSpan getDataSafetyTimeout = TimeSpan::FromMilliseconds(5.0);

		bool hasQueryStarted = false;
		uint64_t coveredPixels = 0;

		D3D11_QUERY_DESC queryDescription;
		ComPtr<ID3D11Query> query;
	};
}
