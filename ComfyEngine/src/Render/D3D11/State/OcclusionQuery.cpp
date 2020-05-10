#include "OcclusionQuery.h"

namespace Comfy::Render::D3D11
{
	OcclusionQuery::OcclusionQuery()
	{
		queryDescription.Query = D3D11_QUERY_OCCLUSION;
		queryDescription.MiscFlags = 0;

		D3D.Device->CreateQuery(&queryDescription, &query);
	}

	OcclusionQuery::OcclusionQuery(const char* debugName) 
		: OcclusionQuery()
	{
		D3D11_SetObjectDebugName(query.Get(), debugName);
	}

	void OcclusionQuery::BeginQuery()
	{
		if (isMidQuery)
			return;

		isFirstQuery = false;

		D3D.Context->Begin(query.Get());
		isMidQuery = true;
	}

	void OcclusionQuery::EndQuery()
	{
		isMidQuery = false;
		D3D.Context->End(query.Get());
	}

	void OcclusionQuery::QueryData()
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

	bool OcclusionQuery::IsFirstQuery() const
	{
		return isFirstQuery;
	}

	bool OcclusionQuery::HasCoveredPixelsReady() const
	{
		return lastCoveredPixels.has_value();
	}

	u64 OcclusionQuery::GetCoveredPixels() const
	{
		return lastCoveredPixels.value_or(0);
	}
}
