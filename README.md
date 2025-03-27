## Comfy... Studio?
Comfy Studio is a *totally epic ⚡⚡* Chart Editor for creating custom Project DIVA Arcade-Style charts for use in modding.  *Holy smokes*, would you believe it!

*"Wow this totally sucks ass!"*, - Someone, probably.

![editor_example_chart](ComfyStudio/manual/image/other/editor_example_chart.png)

### Preamble
Dear Diary, Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.


So with that important message aside, here's some shitty code I worked on from 2019-2021 all under the "Comfy" umbrella. 
A name representing the deep, warm, cozy feeling of peace and comfort that only a loving passion project like this could provide.
... Or rather just some random name because I guess I kinda liked using that funny frog emote with his comfy blanket on a chair or something? Naming do be hard.
Time sure flies, when you're having fun (ᴵ ʰᵃᵗᵉ ᵐʸ ˡⁱᶠᵉ) ... --- ... h̶̶̵͘͞e̵̢ļ̷́p̴̷̡ ̶̡͘͝Mɏ ȼᵾɍɍɇNŦ ŁØȼȺŦɨønɨS...


#### :warning: **Disclaimer** :warning: 
I didn't check if the repository is even set up correctly and everything is included and compiles as it should. 
The code is provided as is and is licensed under GPL-3 License so basically do whatever the fuck you want as long as you keep it open source type shit. #YOLO #SWAG #WHENWILLTHEPAINFINALLYEND


## So I guess that's where we're at
I'm generally very unhappy with the overall structure looking back at it but hey at least it was a valuable learning exercise.
Lots of small deeply nested yet shallow OOP/C# style files, projects and directories garbo scattered across with very little actual meat inside that makes me wanna puke nowadays but somehow has enterprise gooners dripping wet - Would not recommend!

I didn't initially plan on making this public at all (...at least anything more that the chart editor) but I also didn't want to spend more time and effort ripping it apart while also losing the git history and OCD baiting myself into rewriting half of in the process just to bring it up to my current "standards" until I'm satisfied with it.
Letting it sit here to just rot away also feels kinda shitty for anybody who might want to keep hacking on it, so myeah fuck it.

A lot of the code was me playing around, just having fun with different ideas and I guess also some kind of chart editor..? for some fucked up weeb game with dancing dolls 'n shit?? I dunno man, feels like a fever dream tbh.

## The Tower of Babel
Schizo rambling over with, the overall project structure I *think* was about the following:

- `ComfyLib`:
General common code and file format handling stuff (much wow)

- `ComfyEngine`:
Built on top of ComfyLib providing platform specific graphics, audio, input, window handling and so on using those file formats (very pog)

- `ComfyStudio`:
Although I *have* previously dabbled with more authoring tools like a **completly unfinished** Aet (2D animation) editor as well as some other 3D stuff *(before getting utterly cucked by After Effects)*
the important part here is of course the *mostly complete* **chart editor** which **"Comfy Studio"** is known for! Built on top of ComfyLib + ComfyEngine

- `ComfyData`:
Build-step program and data for packing static resources into a single data file which is then loaded by ComfyStudio at startup (great example of stuff I didn't really need but my heart desired!)

- `ComfyVersion`:
Build-step program for automatically generating version info source code via git to be included by ComfyStudio (K.I.S.S. I guess)

- `ComfySandbox`:
Unethical Unit 731 ComfyEngine experiments (completly useless garbage, do not unshackle the demons *or thou shall witness the wrath*)

*(Oh and to run it it also requires a couple game files extracted and placed in a `dev_rom/` directory next to `ComfyStudio.exe` which are not included in this repository for obvious reasons. Just yoink 'em from an already compiled build I guess)*


## I Guess That's it
[So thats how you wanna be? Ok, ... hey guys..... I guess thats it, 🤯🔫 KAPOW 📱 🎶](https://www.youtube.com/watch?v=UMLCSbDLkTU)
