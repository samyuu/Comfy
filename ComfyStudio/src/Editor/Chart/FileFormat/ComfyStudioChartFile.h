#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Chart/Chart.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Studio::Editor
{
	// NOTE: A separate file class has the advantage of making async saving (due to a copy) much easier
	//		 and better abstracts away the file format details
	class ComfyStudioChartFile : public IO::IStreamReadable, public IO::IStreamWritable, NonCopyable
	{
	public:
		// NOTE: Comfy Studio ���� (Fumen)
		static constexpr std::string_view Extension = ".csfm";
		static constexpr std::string_view FilterName = "Comfy Studio Chart (*.csfm)";
		static constexpr std::string_view FilterSpec = "*.csfm";

	public:
		ComfyStudioChartFile() = default;
		ComfyStudioChartFile(const Chart& sourceChart);
		~ComfyStudioChartFile() = default;

	public:
		std::unique_ptr<Chart> ToChart() const;

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
		void FromChart(const Chart& sourceChart);

	private:
	};
}
