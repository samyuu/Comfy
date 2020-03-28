#include "OcclusionQuery.h"

namespace Comfy::Graphics
{
	D3D_OcclusionQuery::D3D_OcclusionQuery()
	{
		queryDescription.Query = D3D11_QUERY_OCCLUSION;
		queryDescription.MiscFlags = 0;

		D3D.Device->CreateQuery(&queryDescription, &query);
	}

	D3D_OcclusionQuery::D3D_OcclusionQuery(const char* debugName) 
		: D3D_OcclusionQuery()
	{
		D3D_SetObjectDebugName(query.Get(), debugName);
	}

	void D3D_OcclusionQuery::BeginQuery()
	{
		if (isMidQuery)
			return;

		isFirstQuery = false;

		D3D.Context->Begin(query.Get());
		isMidQuery = true;
	}

	void D3D_OcclusionQuery::EndQuery()
	{
		isMidQuery = false;
		D3D.Context->End(query.Get());
	}

	void D3D_OcclusionQuery::QueryData()
	{
		if (isMidQuery)
			return;

		lastCoveredPixels = coveredPixels;
		coveredPixels.reset();

		const auto startTime = TimeSpan::GetTimeNow();
		size_t iterationsUntilSuccess = 0;

		while (true)
		{
			UINT64 queryData;
			const HRESULT getDataResult = D3D.Context->GetData(query.Get(), &queryData, sizeof(UINT64), 0);
			
			if (getDataResult == S_OK)
			{
				coveredPixels = queryData;
				break;
			}

			// NOTE: Should hopefully never happen
			if (const auto elapsedLoopTime = (TimeSpan::GetTimeNow() - startTime); elapsedLoopTime > getDataSafetyTimeout)
			{
				assert(false);
				lastCoveredPixels.reset();
				return;
			}

			iterationsUntilSuccess++;
		}
	}

	bool D3D_OcclusionQuery::IsFirstQuery() const
	{
		return isFirstQuery;
	}

	bool D3D_OcclusionQuery::HasCoveredPixelsReady() const
	{
		return lastCoveredPixels.has_value();
	}

	uint64_t D3D_OcclusionQuery::GetCoveredPixels() const
	{
		return lastCoveredPixels.value_or(0);
	}
}
