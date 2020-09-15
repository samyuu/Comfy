#include "ComfyStudioChartFile.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Studio::Editor
{
	ComfyStudioChartFile::ComfyStudioChartFile(const Chart& sourceChart)
	{
		FromChart(sourceChart);
	}

	std::unique_ptr<Chart> ComfyStudioChartFile::ToChart() const
	{
		auto chart = std::make_unique<Chart>();

		// TODO:

		return chart;
	}

	IO::StreamResult ComfyStudioChartFile::Read(IO::StreamReader& reader)
	{
		return IO::StreamResult::Success;
	}

	IO::StreamResult ComfyStudioChartFile::Write(IO::StreamWriter& writer)
	{
		// TODO:
		writer.WriteU32_BE('CSFM');
		return IO::StreamResult::Success;
	}

	void ComfyStudioChartFile::FromChart(const Chart& sourceChart)
	{
		// TODO:
	}
}
