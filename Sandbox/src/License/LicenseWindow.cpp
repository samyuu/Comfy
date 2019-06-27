#include "LicenseWindow.h"
#include "ImGui/imgui.h"
#include "Misc/StringHelper.h"
#include <array>
#include <FontIcons.h>

static struct LicensedCode
{
	std::string Name;
	std::string Description;
	std::string LicenseName;
	std::string License;

	LicensedCode(const char* name, const char* description, const char* licenseName, const char* license)
		: Name(name), Description(description), LicenseName(licenseName), License(license)
	{
		for (std::string* stringPtr = &Name; stringPtr <= &License; stringPtr++)
			Trim(*stringPtr);
	};
};

static LicensedCode LicenseData[] =
{
	// GLAD
	{
		// Name
		R"~(
GLAD
		)~",

		// Description
		R"~(
GitHub: https://github.com/Dav1dde/glad
		)~",

		// LicenseName
		R"~(
MIT License
		)~",

		// License
		R"~(
Copyright (c) 2013-2018 David Herberth

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
		)~"
	},

	// GLFW
	{
		// Name
		R"~(
GLFW
		)~",

		// Description
		R"~(
Site: https://www.glfw.org/
GitHub: https://github.com/glfw/glfw
		)~",

		// LicenseName
		R"~(
zlib License
		)~",

		// License
		R"~(
Copyright (c) 2006-2016 Camilla Lowy <elmindreda@glfw.org>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
		)~"
	},

	// GLM
	{
		// Name
		R"~(
OpenGL Mathematics (GLM)
		)~",

		// Description
		R"~(
Site: https://glm.g-truc.net/0.9.9/index.html
GitHub: https://github.com/g-truc/glm
		)~",

		// LicenseName
		R"~(
Happy Bunny License and MIT License
		)~",

		// License
		R"~(
Copyright (c) 2005 - 2014 G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Restrictions:
 By making use of the Software for military purposes, you choose to make a
 Bunny unhappy.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

--------------------------------------------------------------------------------

Copyright (c) 2005 - 2014 G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
		)~"
	},

	// Font Awesome
	{
		// Name
		R"~(
Font Awesome
		)~",

		// Description
		R"~(
Site: https://fontawesome.com/
GitHub: https://github.com/FortAwesome/Font-Awesome
		)~",

		// LicenseName
		R"~(
Font Awesome Free License
		)~",

		// License
		R"~(
Font Awesome Free is free, open source, and GPL friendly. You can use it for
commercial projects, open source projects, or really almost whatever you want.
Full Font Awesome Free license: https://fontawesome.com/license/free.

# Icons: CC BY 4.0 License (https://creativecommons.org/licenses/by/4.0/)
In the Font Awesome Free download, the CC BY 4.0 license applies to all icons
packaged as SVG and JS file types.

# Fonts: SIL OFL 1.1 License (https://scripts.sil.org/OFL)
In the Font Awesome Free download, the SIL OFL license applies to all icons
packaged as web and desktop font files.

# Code: MIT License (https://opensource.org/licenses/MIT)
In the Font Awesome Free download, the MIT license applies to all non-font and
non-icon files.

# Attribution
Attribution is required by MIT, SIL OFL, and CC BY licenses. Downloaded Font
Awesome Free files already contain embedded comments with sufficient
attribution, so you shouldn't need to do anything additional when using these
files normally.

We've kept attribution comments terse, so we ask that you do not actively work
to remove them from files, especially code. They're a great way for folks to
learn about Font Awesome.

# Brand Icons
All brand icons are trademarks of their respective owners. The use of these
trademarks does not indicate endorsement of the trademark holder by Font
Awesome, nor vice versa. **Please do not use brand logos for any purpose except
to represent the company, product, or service to which they refer.**
		)~"
	},

	// IconFontCppHeaders
	{
		// Name
		R"~(
IconFontCppHeaders
		)~",

		// Description
		R"~(
GitHub: https://github.com/juliettef/IconFontCppHeaders
		)~",

		// LicenseName
		R"~(
zlib License
		)~",

		// License
		R"~(
Copyright (c) 2017 Juliette Foucaut and Doug Binks

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
		)~"
	},

	// Dear ImGui
	{
		// Name
		R"~(
ImGui
		)~",

		// Description
		R"~(
GitHub: https://github.com/ocornut/imgui
		)~",

		// LicenseName
		R"~(
MIT License
		)~",

		// License
		R"~(
Copyright (c) 2014-2019 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
		)~"
	},

	// RtAudio
	{
		// Name
		R"~(
RtAudio
		)~",

		// Description
		R"~(
Site: http://www.music.mcgill.ca/~gary/rtaudio/
GitHub: https://github.com/thestk/rtaudio
		)~",

		// LicenseName
		R"~(
RtAudio license
		)~",

		// License
		R"~(
Copyright (c) 2001-2019 Gary P. Scavone

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

Any person wishing to distribute modifications to the Software is
asked to send the modifications to the original developer so that
they can be incorporated into the canonical version.  This is,
however, not a binding provision of this license.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
		)~"
	},

	// stb_image
	{
		// Name
		R"~(
stb_image
		)~",

		// Description
		R"~(
GitHub: https://github.com/nothings/stb
		)~",

		// LicenseName
		R"~(
MIT License
		)~",

		// License
		R"~(
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
		)~"
	},

	// stb_image_write
	{
		// Name
		R"~(
stb_image_write
		)~",

		// Description
		R"~(
GitHub: https://github.com/nothings/stb
		)~",

		// LicenseName
		R"~(
MIT License
		)~",

		// License
		R"~(
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
		)~"
	},

	// --- TEMPLATE --- // 
	/*
	// DUMMY
	{
		// Name
		R"~(
DUMMY
		)~",

		// Description
		R"~(
Site: https://www.dummy.org/
GitHub: https://github.com/Name/Repository
		)~",

		// LicenseName
		R"~(
Dummy License
		)~",

		// License
		R"~(
Copyright (c) 2000-2019 Dummy Name
		)~"
	},
	*/
};

bool LicenseWindow::DrawGui()
{
	constexpr ImGuiWindowFlags scrollBarWindowFlags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;

	ImGui::BulletText("License:");
	ImGui::Separator();

	ImGui::BeginChild("##LicenseWindowListChild", ImVec2(ImGui::GetWindowWidth() * .2f, 0), true, scrollBarWindowFlags);
	{
		for (size_t i = 0; i < IM_ARRAYSIZE(LicenseData); i++)
			if (ImGui::Selectable(LicenseData[i].Name.c_str(), i == selectedIndex))
				selectedIndex = i;
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##LicenseWindowInfoChild", ImVec2(0, 0), true);
	{
		if (selectedIndex >= 0 && selectedIndex < IM_ARRAYSIZE(LicenseData))
		{
			auto data = &LicenseData[selectedIndex];

			ImGui::BulletText("%s / %s:", data->Name.c_str(), data->LicenseName.c_str());
			ImGui::Separator();

			ImGui::BeginChild("##LicenseWindowInfoChildInner", ImVec2(0, 0), true);
			{
				ImGui::Text("%s", data->Description.c_str());

				ImGui::BeginChild("##LicenseWindowInfoChildLicense", ImVec2(0, 0), true, scrollBarWindowFlags);
				{
					ImGui::Text("%s", data->License.c_str());
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
	}
	ImGui::EndChild();
	return true;
}

bool* LicenseWindow::GetIsOpen()
{
	return &isOpen;
}

const char* LicenseWindow::GetWindowName() const
{
	return "License Window";
}
