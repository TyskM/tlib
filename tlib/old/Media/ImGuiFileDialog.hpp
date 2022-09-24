
// Taken from this repo: https://github.com/aiekick/ImGuiFileDialog

/*
MIT License

Copyright (c) 2018-2022 Stephane Cuillerdier (aka aiekick)

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
*/

/*
-----------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

github repo : https://github.com/aiekick/ImGuiFileDialog
this section is the content of the ReadMe.md file

# ImGuiFileDialog

## Purpose

ImGuiFileDialog is a file selection dialog built for (and using only) [Dear ImGui](https://github.com/ocornut/imgui).

My primary goal was to have a custom pane with widgets according to file extension. This was not possible using other
solutions.

## ImGui Supported Version

ImGuiFileDialog follow the master and docking branch of ImGui . currently ImGui 1.88 WIP

## Structure

* The library is in [Lib_Only branch](https://github.com/aiekick/ImGuiFileDialog/tree/Lib_Only)
* A demo app can be found the [master branch](https://github.com/aiekick/ImGuiFileDialog/tree/master)

This library is designed to be dropped into your source code rather than compiled separately.

From your project directory:

```
mkdir lib    <or 3rdparty, or externals, etc.>
cd lib
git clone https://github.com/aiekick/ImGuiFileDialog.git
git checkout Lib_Only
```

These commands create a `lib` directory where you can store any third-party dependencies used in your project, downloads
the ImGuiFileDialog git repository and checks out the Lib_Only branch where the actual library code is located.

Add `lib/ImGuiFileDialog/ImGuiFileDialog.cpp` to your build system and include
`lib/ImGuiFileDialog/ImGuiFileDialog.h` in your source code. ImGuiFileLib will compile with and be included directly in
your executable file.

If, for example, your project uses cmake, look for a line like `add_executable(my_project_name main.cpp)`
and change it to `add_executable(my_project_name lib/ImGuiFileDialog/ImGuiFileDialog.cpp main.cpp)`. This tells the
compiler where to find the source code declared in `ImGuiFileDialog.h` which you included in your own source code.

## Requirements:

You must also, of course, have added [Dear ImGui](https://github.com/ocornut/imgui) to your project for this to work at
all.

[dirent v1.23](https://github.com/tronkko/dirent/tree/v1.23) is required to use ImGuiFileDialog under Windows. It is
included in the Lib_Only branch for your convenience.

## Features

- Separate system for call and display
	- Can have many function calls with different parameters for one display function, for example
- Can create a custom pane with any widgets via function binding
	- This pane can block the validation of the dialog
	- Can also display different things according to current filter and UserDatas
- Advanced file style for file/dir/link coloring / icons / font
- Multi-selection (ctrl/shift + click) :
	- 0 => Infinite
	- 1 => One file (default)
	- n => n files
- Compatible with MacOs, Linux, Windows
	- Windows version can list drives
- Supports modal or standard dialog types
- Select files or directories
- Filter groups and custom filter names
- can ignore filter Case for file searching
- Keyboard navigation (arrows, backspace, enter)
- Exploring by entering characters (case insensitive)
- Directory bookmarks
- Directory manual entry (right click on any path element)
- Optional 'Confirm to Overwrite" dialog if file exists
- C Api (succesfully tested with CimGui)
- Thumbnails Display (agnostic way for compatibility with any backend, sucessfully tested with OpenGl and Vulkan)
- The dialog can be embedded in another user frame than the standard or modal dialog
- Can tune validation buttons (placements, widths, inversion)
- Can quick select a parrallel directory of a path, in the path composer (when you clikc on a / you have a popup)
- regex support for filters, collection of fitler and filestyle (the regex is recognized when between ( and ) in a filter

## Singleton Pattern vs. Multiple Instances

### Single Dialog :

If you only need to display one file dialog at a time, use ImGuiFileDialog's singleton pattern to avoid explicitly
declaring an object:

```cpp
ImGuiFileDialog::Instance()->method_of_your_choice();
```

### Multiple Dialogs :

If you need to have multiple file dialogs open at once, declare each dialog explicity:

```cpp
ImGuiFileDialog instance_a;
instance_a.method_of_your_choice();
ImGuiFileDialog instance_b;
instance_b.method_of_your_choice();
```

## Simple Dialog :

```cpp
void drawGui()
{
  // open Dialog Simple
  if (ImGui::Button("Open File Dialog"))
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", ".");

  // display
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
  {
	// action if OK
	if (ImGuiFileDialog::Instance()->IsOk())
	{
	  std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
	  std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
	  // action
	}

	// close
	ImGuiFileDialog::Instance()->Close();
  }
}
```

![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/dlg_simple.gif)

## Modal dialog :

you have now a flag for open modal dialog :

```cpp
ImGuiFileDialogFlags_Modal
```

you can use it like that :

```cpp
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", 
	".", 1, nullptr, ImGuiFileDialogFlags_Modal);
```
	
## Directory Chooser :

To have a directory chooser, set the file extension filter to nullptr:

```cpp
ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, ".");
```

In this mode you can select any directory with one click and open a directory with a double-click.

![directoryChooser](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/directoryChooser.gif)

## Dialog with Custom Pane :

The signature of the custom pane callback is:

### for C++ :

```cpp
void(const char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue)
```

### for C :

```c
void(const char *vFilter, void* vUserDatas, bool *vCantContinue)
```

### Example :

```cpp
static bool canValidateDialog = false;
inline void InfosPane(cosnt char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");
	ImGui::Text("Selected Filter : %s", vFilter.c_str());
	if (vUserDatas)
		ImGui::Text("UserDatas : %s", vUserDatas);
	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);
	if (vCantContinue)
		*vCantContinue = canValidateDialog;
}

void drawGui()
{
  // open Dialog with Pane
  if (ImGui::Button("Open File Dialog with a custom pane"))
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp",
			".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 350, 1, UserDatas("InfosPane"));

  // display and action if ok
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
  {
	if (ImGuiFileDialog::Instance()->IsOk())
	{
		std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
		std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
		std::string filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
		// here convert from string because a string was passed as a userDatas, but it can be what you want
		std::string userDatas;
		if (ImGuiFileDialog::Instance()->GetUserDatas())
			userDatas = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
		auto selection = ImGuiFileDialog::Instance()->GetSelection(); // multiselection

		// action
	}
	// close
	ImGuiFileDialog::Instance()->Close();
  }
}
```

![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/doc/dlg_with_pane.gif)

## File Style : Custom icons and colors by extension

You can define style for files/dirs/links in many ways :

the style can be colors, icons and fonts

the general form is :
```cpp
ImGuiFileDialog::Instance()->SetFileStyle(styleType, criteria, color, icon, font);

styleType can be thoses :

IGFD_FileStyleByTypeFile				// define style for all files
IGFD_FileStyleByTypeDir					// define style for all dir
IGFD_FileStyleByTypeLink				// define style for all link
IGFD_FileStyleByExtention				// define style by extention, for files or links
IGFD_FileStyleByFullName				// define style for particular file/dir/link full name (filename + extention)
IGFD_FileStyleByContainedInFullName		// define style for file/dir/link when criteria is contained in full name
```

ImGuiFileDialog accepts icon font macros as well as text tags for file types.

[ImGuIFontStudio](https://github.com/aiekick/ImGuiFontStudio) is useful here. I wrote it to make it easy to create
custom icon sets for use with Dear ImGui.

It is inspired by [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders), which can also be used with
ImGuiFileDialog.

samples :

```cpp
// define style by file extention and Add an icon for .png files
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC, font1);
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");

// define style for all directories
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);
// can be for a specific directory
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, ".git", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);

// define style for all files
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FILE);
// can be for a specific file
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, ".git", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FILE);

// define style for all links
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeLink, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f));
// can be for a specific link
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeLink, "Readme.md", ImVec4(0.5f, 1.0f, 0.9f, 0.9f));

// define style for any files/dirs/links by fullname
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);

// define style by file who are containing this string
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);

all of theses can be miwed with IGFD_FileStyleByTypeDir / IGFD_FileStyleByTypeFile / IGFD_FileStyleByTypeLink
like theses by ex :
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByFullName, "cmake", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);

// for all these,s you can use a regex
// ex for color files like Custom*.h
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "(Custom.+[.]h)", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC, font1);
```

this sample code of [master/main.cpp](https://github.com/aiekick/ImGuiFileDialog/blob/master/main.cpp) produce the picture above :

```cpp
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]"); // add an text for a filter type
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER); // for all dirs
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "CMakeLists.txt", ImVec4(0.1f, 0.5f, 0.5f, 0.9f), ICON_IGFD_ADD);
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);
```

![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/color_filter.png)

## Filter Collections

You can define a custom filter name that corresponds to a group of filters using this syntax:

```custom_name1{filter1,filter2,filter3},custom_name2{filter1,filter2},filter1```

When you select custom_name1, filters 1 to 3 will be applied. The characters `{` and `}` are reserved. Don't use them
for filter names.

this code :

```cpp
const char *filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IMFDLG_FOLDER_OPEN " Choose a File", filters, ".");
```

will produce :
![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/collectionFilters.gif)

## Multi Selection

You can define in OpenDialog call the count file you want to select :

- 0 => infinite
- 1 => one file only (default)
- n => n files only

See the define at the end of these funcs after path.

```cpp
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".*,.cpp,.h,.hpp", ".");
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose 1 File", ".*,.cpp,.h,.hpp", ".", 1);
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose 5 File", ".*,.cpp,.h,.hpp", ".", 5);
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose many File", ".*,.cpp,.h,.hpp", ".", 0);
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg",
   ".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), 350, 1, "SaveFile"); // 1 file
```

![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/multiSelection.gif)

## File Dialog Constraints

You can set the minimum and/or maximum size of the dialog:

```cpp
ImVec2 maxSize = ImVec2((float)display_w, (float)display_h);  // The full display area
ImVec2 minSize = maxSize * 0.5f;  // Half the display area
ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize);
```

![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/dialog_constraints.gif)

## Detail View Mode

Dear ImGui just released an improved table API. If your downloaded version of Dear ImGui includes the beta version of
table support (included for some time now) you can enable table support by uncommenting `#define USE_IMGUI_TABLES` in
you custom config file (CustomImGuiFileDialogConfig.h)

If your version of Dear ImGui has finalized tables support, it will be enabled by default.
![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/imgui_tables_branch.gif)

## Exploring by keys

You can activate this feature by uncommenting `#define USE_EXPLORATION_BY_KEYS`
in your custom config file (CustomImGuiFileDialogConfig.h)

You can also uncomment the next lines to define navigation keys:

* IGFD_KEY_UP => Up key for explore to the top
* IGFD_KEY_DOWN => Down key for explore to the bottom
* IGFD_KEY_ENTER => Enter key for open directory
* IGFD_KEY_BACKSPACE => BackSpace for comming back to the last directory

You can also jump to a point in the file list by pressing the corresponding key of the first filename character.

![alt text](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/explore_ny_keys.gif)

As you see the current item is flashed by default for 1 second. You can define the flashing lifetime with the function

```cpp
ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(1.0f);
```

## Bookmarks

You can create/edit/call path bookmarks and load/save them.

Activate this feature by uncommenting: `#define USE_BOOKMARK` in your custom config file (CustomImGuiFileDialogConfig.h)

More customization options:

```cpp
#define bookmarkPaneWith 150.0f => width of the bookmark pane
#define IMGUI_TOGGLE_BUTTON ToggleButton => customize the Toggled button (button stamp must be : (const char* label, bool *toggle)
#define bookmarksButtonString "Bookmark" => the text in the toggle button
#define bookmarksButtonHelpString "Bookmark" => the helper text when mouse over the button
#define addBookmarkButtonString "+" => the button for add a bookmark
#define removeBookmarkButtonString "-" => the button for remove the selected bookmark
```

* You can select each bookmark to edit the displayed name corresponding to a path
* Double-click on the label to apply the bookmark

![bookmarks.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/bookmarks.gif)

You can also serialize/deserialize bookmarks (for example to load/save from/to a file):
```cpp
Load => ImGuiFileDialog::Instance()->DeserializeBookmarks(bookmarString);
Save => std::string bookmarkString = ImGuiFileDialog::Instance()->SerializeBookmarks();
```
(please see example code for details)

you can also add/remove bookmark by code :
and in this case, you can also avoid serialization of code based bookmark

```cpp
Add => ImGuiFileDialog::Instance()->AddBookmark(bookmark_name, bookmark_path);
Remove => ImGuiFileDialog::Instance()->RemoveBookmark(bookmark_name);
Save => std::string bookmarkString = ImGuiFileDialog::Instance()->SerializeBookmarks(true); // true for prevent serialization of code based bookmarks
```

## Path Edition :

Right clicking on any path element button allows the user to manually edit the path from that portion of the tree.
Pressing the completion key (GLFW uses `enter` by default) validates the new path. Pressing the cancel key (GLFW
uses`escape` by default) cancels the manual entry and restores the original path.

Here's the manual entry operation in action:
![inputPathEdition.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/inputPathEdition.gif)

## Confirm Overwrite Dialog :

If you want avoid overwriting files after selection, ImGuiFileDialog can show a dialog to confirm or cancel the
operation.

To do so, define the flag ImGuiFileDialogFlags_ConfirmOverwrite in your call to OpenDialog.

By default this flag is not set since there is no pre-defined way to define if a dialog will be for Open or Save
behavior. (by design! :) )

Example code For Standard Dialog :

```cpp
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
	ICON_IGFD_SAVE " Choose a File", filters,
	".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
```

Example code For Modal Dialog :

```cpp
ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
	ICON_IGFD_SAVE " Choose a File", filters,
	".", "", 1, nullptr, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite);
```

This dialog will only verify the file in the file field, not with `GetSelection()`.

The confirmation dialog will be a non-movable modal (input blocking) dialog displayed in the middle of the current
ImGuiFileDialog window.

As usual, you can customize the dialog in your custom config file (CustomImGuiFileDialogConfig.h in this example)

Uncomment these line for customization options:

```cpp
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
//#define OverWriteDialogConfirmButtonString "Confirm"
//#define OverWriteDialogCancelButtonString "Cancel"
```

See the result :

![ConfirmToOverWrite.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/ConfirmToOverWrite.gif)

## Open / Save dialog Behavior :

ImGuiFileDialog uses the same code internally for Open and Save dialogs. To distinguish between them access the various
data return functions depending on what the dialog is doing.

When selecting an existing file (for example, a Load or Open dialog), use

```cpp
std::map<std::string, std::string> GetSelection(); // Returns selection via a map<FileName, FilePathName>
UserDatas GetUserDatas();                          // Get user data provided by the Open dialog
```

To selecting a new file (for example, a Save As... dialog), use:

```cpp
std::string GetFilePathName();                     // Returns the content of the selection field with current file extension and current path
std::string GetCurrentFileName();                  // Returns the content of the selection field with current file extension but no path
std::string GetCurrentPath();                      // Returns current path only
std::string GetCurrentFilter();                    // The file extension
```

## Thumbnails Display

You can now, display thumbnails of pictures.

![thumbnails.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/thumbnails.gif)

The file resize use stb/image so the following files extentions are supported :
 * .png (tested sucessfully)
 * .bmp (tested sucessfully)
 * .tga (tested sucessfully)
 * .jpg (tested sucessfully)
 * .jpeg (tested sucessfully)
 * .gif (tested sucessfully_ but not animation just first frame)
 * .psd (not tested)
 * .pic (not tested)
 * .ppm (not tested)
 * .pgm (not tested)

Corresponding to your backend (ex : OpenGl) you need to define two callbacks :
* the first is a callback who will be called by ImGuiFileDialog for create the backend texture
* the second is a callback who will be called by ImGuiFileDialog for destroy the backend texture

After that you need to call the function who is responsible to create / destroy the textures.
this function must be called in your GPU Rendering zone for avoid destroying of used texture.
if you do that at the same place of your imgui code, some backend can crash your app, by ex with vulkan.

To Clarify :

This feature is spliited in two zones :
 - CPU Zone : for load/destroy picture file
 - GPU Zone : for load/destroy gpu textures.
This modern behavior for avoid destroying of used texture,
was needed for vulkan.

This feature was Successfully tested on my side with Opengl and Vulkan.
But im sure is perfectly compatible with other modern apis like DirectX and Metal

ex, for opengl :

```cpp
// Create thumbnails texture
ImGuiFileDialog::Instance()->SetCreateThumbnailCallback([](IGFD_Thumbnail_Info *vThumbnail_Info) -> void
{
	if (vThumbnail_Info &&
		vThumbnail_Info->isReadyToUpload &&
		vThumbnail_Info->textureFileDatas)
	{
		GLuint textureId = 0;
		glGenTextures(1, &textureId);
		vThumbnail_Info->textureID = (void*)textureId;

		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			(GLsizei)vThumbnail_Info->textureWidth, (GLsizei)vThumbnail_Info->textureHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, vThumbnail_Info->textureFileDatas);
		glFinish();
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] vThumbnail_Info->textureFileDatas;
		vThumbnail_Info->textureFileDatas = nullptr;

		vThumbnail_Info->isReadyToUpload = false;
		vThumbnail_Info->isReadyToDisplay = true;
	}
});
```

```cpp
// Destroy thumbnails texture
ImGuiFileDialog::Instance()->SetDestroyThumbnailCallback([](IGFD_Thumbnail_Info* vThumbnail_Info)
{
	if (vThumbnail_Info)
	{
		GLuint texID = (GLuint)vThumbnail_Info->textureID;
		glDeleteTextures(1, &texID);
		glFinish();
	}
});
```

```cpp
// GPU Rendering Zone // To call for Create/ Destroy Textures
ImGuiFileDialog::Instance()->ManageGPUThumbnails();
```

## Embedded in other frames :

The dialog can be embedded in another user frame than the standard or modal dialog

You have to create a variable of type ImGuiFileDialog. (if you are suing the singleton, you will not have the possibility to open other dialog)

ex :

```cpp
ImGuiFileDialog fileDialog;

// open dialog; in this case, Bookmark, directory creation are disabled with, and also the file input field is readonly.
// btw you can od what you want
fileDialog.OpenDialog("embedded", "Select File", ".*", "", -1, nullptr,
	ImGuiFileDialogFlags_NoDialog |
	ImGuiFileDialogFlags_DisableBookmarkMode |
	ImGuiFileDialogFlags_DisableCreateDirectoryButton |
	ImGuiFileDialogFlags_ReadOnlyFileNameField);
// then display, here
// to note, when embedded the ImVec2(0,0) (MinSize) do nothing, only the ImVec2(0,350) (MaxSize) can size the dialog frame
fileDialog.Display("embedded", ImGuiWindowFlags_NoCollapse, ImVec2(0,0), ImVec2(0,350)))
```
the result :

![Embedded.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/Embedded.gif)

## Quick Parallel Path Selection in Path Composer

you have a separator between two directories in the path composer
when you click on it you can explore a list of parrallels directories of this point

this feature is disabled by default
you can enable it with the compiler flag : #define USE_QUICK_PATH_SELECT

you can also customize the spacing between path button's with and without this mode
you can do that by define the compiler flag : #define CUSTOM_PATH_SPACING 2
if undefined the spacing is defined by the imgui theme

![quick_composer_path_select.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/quick_composer_path_select.gif)

## Case Insensitive Filtering

you can use this flag 'ImGuiFileDialogFlags_CaseInsensitiveExtention' when you call the display function

```
by ex :
if the flag ImGuiFileDialogFlags_CaseInsensitiveExtention is used
with filters like .jpg or .Jpg or .JPG
all files with extentions by ex : .jpg and .JPG will be displayed
```

## Tune the validations button group

You can specify :
- the width of "ok" and "cancel" buttons, by the set the defines "okButtonWidth" and "cancelButtonWidth"
- the alignement of the button group (left, right, middle, etc..) by set the define "okCancelButtonAlignement"
- if you want to have the ok button on the left and cancel button on the right or inverted by set the define "invertOkAndCancelButtons"

just see theses defines in the config file
```cpp
//Validation buttons
//#define okButtonString " OK"
//#define okButtonWidth 0.0f
//#define cancelButtonString " Cancel"
//#define cancelButtonWidth 0.0f
//alignement [0:1], 0.0 is left, 0.5 middle, 1.0 right, and other ratios
//#define okCancelButtonAlignement 0.0f
//#define invertOkAndCancelButtons false
```
with Alignement 0.0 => left

![alignement_0.0.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/alignement_0.0.png)

with Alignement 1.0 => right

![alignement_1.0.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/alignement_1.0.png)

with Alignement 0.5 => middle

![alignement_0.5.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/alignement_0.5.png)

ok and cancel buttons inverted (cancel on the left and ok on the right)

![validation_buttons_inverted.gif](https://github.com/aiekick/ImGuiFileDialog/blob/master/doc/validation_buttons_inverted.png)

## Filtering by a regex :

you can use a regex for filtering and file coloring 

for have a filter recognized as a regex, you must have it between a ( and a )

this one will filter files who start by the word "Common" and finish by ".h" 
```cpp
ex : "(Custom.+[.]h)" 
```

use cases :

* Simple filter : 
```cpp
OpenDialog("toto", "Choose File", "(Custom.+[.]h)");
```

* Collections filter : 
for this one the filter is between "{" and "}", so you can use the "(" and ")" outside

```cpp
OpenDialog("toto", "Choose File", "Source files (*.cpp *.h *.hpp){(Custom.+[.]h),.h,.hpp},);
```

* file coloring :
this one will colorized all files who start by the word "Common" and finish by ".h" 
```cpp
SetFileStyle(IGFD_FileStyleByFullName, "(Custom.+[.]h)", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
```

* with this feature you can by ex filter and colorize render frame pictures who have ext like .000, .001, .002, etc..
```cpp
OpenDialog("toto", "Choose File", "([.][0-9]{3})");
SetFileStyle(IGFD_FileStyleByFullName, "([.][0-9]{3})", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
```

## How to Integrate ImGuiFileDialog in your project

### Customize ImGuiFileDialog :

You can customize many aspects of ImGuiFileDialog by overriding `ImGuiFileDialogConfig.h`.

To enable your customizations, define the preprocessor directive CUSTOM_IMGUIFILEDIALOG_CONFIG with the path of your
custom config file. This path must be relative to the directory where you put the ImGuiFileDialog module.

This operation is demonstrated in `CustomImGuiFileDialog.h` in the example project to:

* Have a custom icon font instead of labels for buttons or message titles
* Customize the button text (the button call signature must be the same, by the way! :)

The custom icon font used in the example code ([CustomFont.cpp](CustomFont.cpp) and [CustomFont.h](CustomFont.h)) was made
with [ImGuiFontStudio](https://github.com/aiekick/ImGuiFontStudio), which I wrote. :)

ImGuiFontStudio uses ImGuiFileDialog! Check it out.

## Api's C/C++ :

### the C Api

this api was sucessfully tested with CImGui

A C API is available let you include ImGuiFileDialog in your C project.
btw, ImGuiFileDialog depend of ImGui and dirent (for windows)

Sample code with cimgui :

```cpp
// create ImGuiFileDialog
ImGuiFileDialog *cfileDialog = IGFD_Create();

// open dialog
if (igButton("Open File", buttonSize))
{
	IGFD_OpenDialog(cfiledialog,
		"filedlg",                              // dialog key (make it possible to have different treatment reagrding the dialog key
		"Open a File",                          // dialog title
		"c files(*.c *.h){.c,.h}",              // dialog filter syntax : simple => .h,.c,.pp, etc and collections : text1{filter0,filter1,filter2}, text2{filter0,filter1,filter2}, etc..
		".",                                    // base directory for files scan
		"",                                     // base filename
		0,                                      // a fucntion for display a right pane if you want
		0.0f,                                   // base width of the pane
		0,                                      // count selection : 0 infinite, 1 one file (default), n (n files)
		"User data !",                          // some user datas
		ImGuiFileDialogFlags_ConfirmOverwrite); // ImGuiFileDialogFlags
}

ImGuiIO* ioptr = igGetIO();
ImVec2 maxSize;
maxSize.x = ioptr->DisplaySize.x * 0.8f;
maxSize.y = ioptr->DisplaySize.y * 0.8f;
ImVec2 minSize;
minSize.x = maxSize.x * 0.25f;
minSize.y = maxSize.y * 0.25f;

// display dialog
if (IGFD_DisplayDialog(cfiledialog, "filedlg", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
{
	if (IGFD_IsOk(cfiledialog)) // result ok
	{
		char* cfilePathName = IGFD_GetFilePathName(cfiledialog);
		printf("GetFilePathName : %s\n", cfilePathName);
		char* cfilePath = IGFD_GetCurrentPath(cfiledialog);
		printf("GetCurrentPath : %s\n", cfilePath);
		char* cfilter = IGFD_GetCurrentFilter(cfiledialog);
		printf("GetCurrentFilter : %s\n", cfilter);
		// here convert from string because a string was passed as a userDatas, but it can be what you want
		void* cdatas = IGFD_GetUserDatas(cfiledialog);
		if (cdatas)
			printf("GetUserDatas : %s\n", (const char*)cdatas);
		struct IGFD_Selection csel = IGFD_GetSelection(cfiledialog); // multi selection
		printf("Selection :\n");
		for (int i = 0; i < (int)csel.count; i++)
		{
			printf("(%i) FileName %s => path %s\n", i, csel.table[i].fileName, csel.table[i].filePathName);
		}
		// action

		// destroy
		if (cfilePathName) free(cfilePathName);
		if (cfilePath) free(cfilePath);
		if (cfilter) free(cfilter);

		IGFD_Selection_DestroyContent(&csel);
	}
	IGFD_CloseDialog(cfiledialog);
}

// destroy ImGuiFileDialog
IGFD_Destroy(cfiledialog);
```

-----------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------

Thats all.

You can check by example in this repo with the file CustomImGuiFileDialogConfig.h :
- this trick was used for have custom icon font instead of labels for buttons or messages titles
- you can also use your custom imgui button, the button call stamp must be same by the way :)

The Custom Icon Font (in CustomFont.cpp and CustomFont.h) was made with ImGuiFontStudio (https://github.com/aiekick/ImGuiFontStudio) i wrote for that :)
ImGuiFontStudio is using also ImGuiFileDialog.

-----------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------
*/

#ifndef IMGUIFILEDIALOG_H
#define IMGUIFILEDIALOG_H

// compatible with 1.88 WIP
#define IMGUIFILEDIALOG_VERSION "v0.6.5"

#ifndef CUSTOM_IMGUIFILEDIALOG_CONFIG

// uncomment and modify defines under for customize ImGuiFileDialog

//this options need c++17
//#define USE_STD_FILESYSTEM

//#define MAX_FILE_DIALOG_NAME_BUFFER 1024
//#define MAX_PATH_BUFFER_SIZE 1024

// the slash's buttons in path cna be used for quick select parallles directories
//#define USE_QUICK_PATH_SELECT

// the spacing between button path's can be customized. 
// if disabled the spacing is defined by the imgui theme
// define the space between path buttons 
//#define CUSTOM_PATH_SPACING 2

//#define USE_THUMBNAILS
//the thumbnail generation use the stb_image and stb_resize lib who need to define the implementation
//btw if you already use them in your app, you can have compiler error due to "implemntation found in double"
//so uncomment these line for prevent the creation of implementation of these libs again
//#define DONT_DEFINE_AGAIN__STB_IMAGE_IMPLEMENTATION
//#define DONT_DEFINE_AGAIN__STB_IMAGE_RESIZE_IMPLEMENTATION
//#define IMGUI_RADIO_BUTTON RadioButton
//#define DisplayMode_ThumbailsList_ImageHeight 32.0f
//#define tableHeaderFileThumbnailsString "Thumbnails"
//#define DisplayMode_FilesList_ButtonString "FL"
//#define DisplayMode_FilesList_ButtonHelp "File List"
//#define DisplayMode_ThumbailsList_ButtonString "TL"
//#define DisplayMode_ThumbailsList_ButtonHelp "Thumbnails List"
// todo
//#define DisplayMode_ThumbailsGrid_ButtonString "TG"
//#define DisplayMode_ThumbailsGrid_ButtonHelp "Thumbnails Grid"

//#define USE_EXPLORATION_BY_KEYS
// this mapping by default is for GLFW but you can use another
//#include <GLFW/glfw3.h> 
// Up key for explore to the top
//#define IGFD_KEY_UP ImGuiKey_UpArrow
// Down key for explore to the bottom
//#define IGFD_KEY_DOWN ImGuiKey_DownArrow
// Enter key for open directory
//#define IGFD_KEY_ENTER ImGuiKey_Enter
// BackSpace for comming back to the last directory
//#define IGFD_KEY_BACKSPACE ImGuiKey_Backspace

// by ex you can quit the dialog by pressing the key excape
//#define USE_DIALOG_EXIT_WITH_KEY
//#define IGFD_EXIT_KEY ImGuiKey_Escape

// widget
// filter combobox width
//#define FILTER_COMBO_WIDTH 120.0f
// button widget use for compose path
//#define IMGUI_PATH_BUTTON ImGui::Button
// standard button
//#define IMGUI_BUTTON ImGui::Button

// locales string
//#define createDirButtonString "+"
//#define resetButtonString "R"
//#define drivesButtonString "Drives"
//#define editPathButtonString "E"
//#define searchString "Search"
//#define dirEntryString "[DIR] "
//#define linkEntryString "[LINK] "
//#define fileEntryString "[FILE] "
//#define fileNameString "File Name : "
//#define dirNameString "Directory Path :"
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonEditPathString "Edit path\nYou can also right click on path buttons"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
//#define OverWriteDialogConfirmButtonString "Confirm"
//#define OverWriteDialogCancelButtonString "Cancel"

//Validation buttons
//#define okButtonString " OK"
//#define okButtonWidth 0.0f
//#define cancelButtonString " Cancel"
//#define cancelButtonWidth 0.0f
//alignement [0:1], 0.0 is left, 0.5 middle, 1.0 right, and other ratios
//#define okCancelButtonAlignement 0.0f
//#define invertOkAndCancelButtons 0

// DateTimeFormat
// see strftime functionin <ctime> for customize
// "%Y/%m/%d %H:%M" give 2021:01:22 11:47
// "%Y/%m/%d %i:%M%p" give 2021:01:22 11:45PM
//#define DateTimeFormat "%Y/%m/%d %i:%M%p"

// theses icons will appear in table headers
//#define USE_CUSTOM_SORTING_ICON
//#define tableHeaderAscendingIcon "A|"
//#define tableHeaderDescendingIcon "D|"
//#define tableHeaderFileNameString " File name"
//#define tableHeaderFileTypeString " Type"
//#define tableHeaderFileSizeString " Size"
//#define tableHeaderFileDateTimeString " Date"
//#define fileSizeBytes "o"
//#define fileSizeKiloBytes "Ko"
//#define fileSizeMegaBytes "Mo"
//#define fileSizeGigaBytes "Go"

// default table sort field (must be FIELD_FILENAME, FIELD_TYPE, FIELD_SIZE, FIELD_DATE or FIELD_THUMBNAILS)
//#define defaultSortField FIELD_FILENAME

// default table sort order for each field (true => Descending, false => Ascending)
//#define defaultSortOrderFilename true
//#define defaultSortOrderType true
//#define defaultSortOrderSize true
//#define defaultSortOrderDate true
//#define defaultSortOrderThumbnails true

//#define USE_BOOKMARK
//#define bookmarkPaneWith 150.0f
//#define IMGUI_TOGGLE_BUTTON ToggleButton
//#define bookmarksButtonString "Bookmark"
//#define bookmarksButtonHelpString "Bookmark"
//#define addBookmarkButtonString "+"
//#define removeBookmarkButtonString "-"

#else // CUSTOM_IMGUIFILEDIALOG_CONFIG
#include CUSTOM_IMGUIFILEDIALOG_CONFIG
#endif // CUSTOM_IMGUIFILEDIALOG_CONFIG

// file style enum for file display (color, icon, font)
typedef int IGFD_FileStyleFlags; // -> enum IGFD_FileStyleFlags_
enum IGFD_FileStyleFlags_ // by evaluation / priority order
{
	IGFD_FileStyle_None					= 0,		// define none style
	IGFD_FileStyleByTypeFile			= (1 << 0),	// define style for all files
	IGFD_FileStyleByTypeDir				= (1 << 1),	// define style for all dir
	IGFD_FileStyleByTypeLink			= (1 << 2),	// define style for all link
	IGFD_FileStyleByExtention			= (1 << 3),	// define style by extention, for files or links
	IGFD_FileStyleByFullName			= (1 << 4),	// define style for particular file/dir/link full name (filename + extention)
	IGFD_FileStyleByContainedInFullName = (1 << 5),	// define style for file/dir/link when criteria is contained in full name
};

typedef int ImGuiFileDialogFlags; // -> enum ImGuiFileDialogFlags_
enum ImGuiFileDialogFlags_
{
	ImGuiFileDialogFlags_None							= 0,		// define none default flag
	ImGuiFileDialogFlags_ConfirmOverwrite				= (1 << 0),	// show confirm to overwrite dialog
	ImGuiFileDialogFlags_DontShowHiddenFiles			= (1 << 1),	// dont show hidden file (file starting with a .)
	ImGuiFileDialogFlags_DisableCreateDirectoryButton	= (1 << 2),	// disable the create directory button
	ImGuiFileDialogFlags_HideColumnType					= (1 << 3),	// hide column file type
	ImGuiFileDialogFlags_HideColumnSize					= (1 << 4),	// hide column file size
	ImGuiFileDialogFlags_HideColumnDate					= (1 << 5),	// hide column file date
	ImGuiFileDialogFlags_NoDialog						= (1 << 6),	// let the dialog embedded in your own imgui begin / end scope
	ImGuiFileDialogFlags_ReadOnlyFileNameField			= (1 << 7),	// don't let user type in filename field for file open style dialogs
	ImGuiFileDialogFlags_CaseInsensitiveExtention		= (1 << 8), // the file extentions treatments will not take into account the case 
	ImGuiFileDialogFlags_Modal							= (1 << 9), // modal
#ifdef USE_THUMBNAILS
	ImGuiFileDialogFlags_DisableThumbnailMode			= (1 << 10),	// disable the thumbnail mode
#endif // USE_THUMBNAILS
#ifdef USE_BOOKMARK
	ImGuiFileDialogFlags_DisableBookmarkMode			= (1 << 11),	// disable the bookmark mode
#endif // USE_BOOKMARK
	ImGuiFileDialogFlags_Default = ImGuiFileDialogFlags_ConfirmOverwrite
};

#ifdef USE_THUMBNAILS
struct IGFD_Thumbnail_Info
{
	int isReadyToDisplay = 0;				// ready to be rendered, so texture created
	int isReadyToUpload = 0;				// ready to upload to gpu
	int isLoadingOrLoaded = 0;				// was sent to laoding or loaded
	void* textureID = 0;					// 2d texture id (void* is like ImtextureID type) (GL, DX, VK, Etc..)
 	unsigned char* textureFileDatas = 0;	// file texture datas, will be rested to null after gpu upload
	int textureWidth = 0;					// width of the texture to upload
	int textureHeight = 0;					// height of the texture to upload
	int textureChannels = 0;				// count channels of the texture to upload
	void* userDatas = 0;					// user datas
};
#endif // USE_THUMBNAILS

#ifdef __cplusplus

#include <imgui.h>

#include <cfloat>
#include <utility>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <regex>

namespace IGFD
{
#ifndef defaultSortField
#define defaultSortField FIELD_FILENAME
#endif // defaultSortField

#ifndef defaultSortOrderFilename
#define defaultSortOrderFilename true
#endif // defaultSortOrderFilename
#ifndef defaultSortOrderType
#define defaultSortOrderType true
#endif // defaultSortOrderType
#ifndef defaultSortOrderSize
#define defaultSortOrderSize true
#endif // defaultSortOrderSize
#ifndef defaultSortOrderDate
#define defaultSortOrderDate true
#endif // defaultSortOrderDate
#ifndef defaultSortOrderThumbnails
#define defaultSortOrderThumbnails true
#endif // defaultSortOrderThumbnails

#ifndef MAX_FILE_DIALOG_NAME_BUFFER 
#define MAX_FILE_DIALOG_NAME_BUFFER 1024
#endif // MAX_FILE_DIALOG_NAME_BUFFER

#ifndef MAX_PATH_BUFFER_SIZE
#define MAX_PATH_BUFFER_SIZE 1024
#endif // MAX_PATH_BUFFER_SIZE

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class FileDialogInternal;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class SearchManager
	{
	public:
		std::string puSearchTag;
		char puSearchBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";
		bool puSearchInputIsActive = false;

	public:
		void Clear();																							// clear datas
		void DrawSearchBar(FileDialogInternal& vFileDialogInternal);											// draw the search bar
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class Utils
	{
	public:
		struct PathStruct
		{
			std::string path;
			std::string name;
			std::string ext;
			bool isOk = false;
		};

	public:
		static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
		static bool ReplaceString(std::string& str, const std::string& oldStr, const std::string& newStr);
		static bool IsDirectoryCanBeOpened(const std::string& name);			// by ex protected dirs (not user rights)
		static bool IsDirectoryExist(const std::string& name);
		static bool CreateDirectoryIfNotExist(const std::string& name);
		static PathStruct ParsePathFileName(const std::string& vPathFileName);
		static void AppendToBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr);
		static void ResetBuffer(char* vBuffer);
		static void SetBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr);
		static bool WReplaceString(std::wstring& str, const std::wstring& oldStr, const std::wstring& newStr);
		static std::vector<std::wstring> WSplitStringToVector(const std::wstring& text, char delimiter, bool pushEmpty);
		static std::string utf8_encode(const std::wstring& wstr);
		static std::wstring utf8_decode(const std::string& str);
		static std::vector<std::string> SplitStringToVector(const std::string& text, char delimiter, bool pushEmpty);
		static std::vector<std::string> GetDrivesList();
		static std::string LowerCaseString(const std::string& vString); // turn all text in lower case for search facilitie
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class FileStyle
	{
	public:
		ImVec4 color = ImVec4(0, 0, 0, 0);
		std::string icon;
		ImFont* font = nullptr;
		IGFD_FileStyleFlags flags = 0;

	public:
		FileStyle();
		FileStyle(const FileStyle& vStyle);
		FileStyle(const ImVec4& vColor, const std::string& vIcon = "", ImFont* vFont = nullptr);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class FileInfos;
	class FilterManager
	{
	public:
		class FilterInfos
		{
		public:
			std::string filter;																				// simple filter
			std::regex filter_regex;																		// filter fo type regex
			std::set<std::string> collectionfilters;														// collections of filters
			std::string filter_optimized;																	// opitmized for case insensitive search
			std::set<std::string> collectionfilters_optimized;												// optimized collections of filters for case insensitive search
			std::vector<std::regex> collectionfilters_regex;												// collection of regex filter type

		public:
			void clear();																					// clear the datas
			bool empty() const;																				// is filter empty
			bool exist(const std::string& vFilter, bool vIsCaseInsensitive) const;							// is filter exist
			bool regex_exist(const std::string& vFilter) const;												// is regex filter exist
		};

	private:
#ifdef NEED_TO_BE_PUBLIC_FOR_TESTS
	public:
#endif
		std::vector<FilterInfos> prParsedFilters;
		std::unordered_map<IGFD_FileStyleFlags, std::unordered_map<std::string, std::shared_ptr<FileStyle>>> prFilesStyle;		// file infos for file extention only
		FilterInfos prSelectedFilter;

	public:
		std::string puDLGFilters;
		std::string puDLGdefaultExt;

	public:
		void ParseFilters(const char* vFilters);															// Parse filter syntax, detect and parse filter collection
		void SetSelectedFilterWithExt(const std::string& vFilter);											// Select filter
		
		bool prFillFileStyle(std::shared_ptr<FileInfos> vFileInfos)  const;									// fill with the good style
		
		void SetFileStyle(
			const IGFD_FileStyleFlags& vFlags,
			const char* vCriteria,
			const FileStyle& vInfos);																		// Set FileStyle
		void SetFileStyle(
			const IGFD_FileStyleFlags& vFlags,
			const char* vCriteria,
			const ImVec4& vColor,
			const std::string& vIcon,
			ImFont* vFont);																					// link file style to Color and Icon and Font
		bool GetFileStyle(
			const IGFD_FileStyleFlags& vFlags,
			const std::string& vCriteria,
			ImVec4* vOutColor,
			std::string* vOutIcon,
			ImFont** vOutFont);																				// Get Color and Icon for Filter
		void ClearFilesStyle();																				// clear prFileStyle

		bool IsCoveredByFilters(														// check if current file extention (vExt) is covered by current filter, ro by regex (vNameExt)
			const std::string& vNameExt, 
			const std::string& vExt, 
			bool vIsCaseInsensitive) const;																	
		bool DrawFilterComboBox(FileDialogInternal& vFileDialogInternal);									// draw the filter combobox
		FilterInfos GetSelectedFilter();																	// get the current selected filter
		std::string ReplaceExtentionWithCurrentFilter(const std::string& vFile) const;						// replace the extention of the current file by the selected filter
		void SetDefaultFilterIfNotDefined();																// define the first filter if no filter is selected
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class FileType
	{
	public:
		enum class ContentType {
			// The ordering will be used during sort.
			Invalid = -1,
			Directory = 0,
			File = 1,
			LinkToUnknown = 2, // link to something that is not a regular file or directory.
		};

	private:
		ContentType m_Content = ContentType::Invalid;
		bool m_Symlink = false;

	public:
		FileType() = default;
		FileType(const ContentType& vContentType, const bool& vIsSymlink)
			: m_Content(vContentType), m_Symlink(vIsSymlink)
		{}

		void SetContent(const ContentType& vContentType) { m_Content = vContentType; }
		void SetSymLink(const bool& vIsSymlink) { m_Symlink = vIsSymlink; }

		bool isValid () const { return m_Content != ContentType::Invalid; }
		bool isDir () const { return m_Content == ContentType::Directory; }
		bool isFile () const { return m_Content == ContentType::File; }
		bool isLinkToUnknown () const { return m_Content == ContentType::LinkToUnknown; }
		bool isSymLink() const { return m_Symlink; }

		// Comparisons only care about the content type, ignoring whether it's a symlink or not.
		bool operator== (const FileType& rhs) const { return m_Content == rhs.m_Content; }
		bool operator!= (const FileType& rhs) const { return m_Content != rhs.m_Content; }
		bool operator<  (const FileType& rhs) const { return m_Content < rhs.m_Content; }
		bool operator>  (const FileType& rhs) const { return m_Content > rhs.m_Content; }
	};

	class FileInfos
	{
	public:
		FileType fileType;    								// fileType		
		std::string filePath;								// path of the file
		std::string fileNameExt;							// filename of the file (file name + extention) (but no path)
		std::string fileNameExt_optimized;					// optimized for search => insensitivecase
		std::string fileExt;								// extention of the file
		size_t fileSize = 0;								// for sorting operations
		std::string formatedFileSize;						// file size formated (10 o, 10 ko, 10 mo, 10 go)
		std::string fileModifDate;							// file user defined format of the date (data + time by default)
		std::shared_ptr<FileStyle> fileStyle = nullptr;		// style of the file
#ifdef USE_THUMBNAILS
		IGFD_Thumbnail_Info thumbnailInfo;		// structre for the display for image file tetxure
#endif // USE_THUMBNAILS

	public:
		bool IsTagFound(const std::string& vTag) const;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class FileManager
	{
	public: // types
		enum class SortingFieldEnum		// sorting for filetering of the file lsit
		{
			FIELD_NONE = 0,				// no sorting preference, result indetermined haha..
			FIELD_FILENAME,				// sorted by filename
			FIELD_TYPE,					// sorted by filetype
			FIELD_SIZE,					// sorted by filesize (formated file size)
			FIELD_DATE,					// sorted by filedate
#ifdef USE_THUMBNAILS
			FIELD_THUMBNAILS,			// sorted by thumbnails (comparaison by width then by height)
#endif // USE_THUMBNAILS
		};

	private:
		std::string prCurrentPath;											// current path (to be decomposed in prCurrentPathDecomposition
		std::vector<std::string> prCurrentPathDecomposition;				// part words
		std::vector<std::shared_ptr<FileInfos>> prFileList;					// base container
		std::vector<std::shared_ptr<FileInfos>> prFilteredFileList;			// filtered container (search, sorting, etc..)
		std::vector<std::shared_ptr<FileInfos>> prPathList;					// base container for path selection
		std::vector<std::shared_ptr<FileInfos>> prFilteredPathList;			// filtered container for path selection (search, sorting, etc..)
		std::vector<std::string>::iterator prPopupComposedPath;				// iterator on prCurrentPathDecomposition for Current Path popup
		std::string prLastSelectedFileName;									// for shift multi selection
		std::set<std::string> prSelectedFileNames;							// the user selection of FilePathNames
		bool prCreateDirectoryMode = false;									// for create directory widget

	public:
		char puVariadicBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";			// called by prSelectableItem
		bool puInputPathActivated = false;									// show input for path edition
		bool puDrivesClicked = false;										// event when a drive button is clicked
		bool puPathClicked = false;											// event when a path button was clicked
		char puInputPathBuffer[MAX_PATH_BUFFER_SIZE] = "";					// input path buffer for imgui widget input text (displayed in palce of composer)
		char puFileNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";			// file name buffer in footer for imgui widget input text
		char puDirectoryNameBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";		// directory name buffer in footer for imgui widget input text (when is directory mode)
		std::string puHeaderFileName;										// detail view name of column file
		std::string puHeaderFileType;										// detail view name of column type
		std::string puHeaderFileSize;										// detail view name of column size
		std::string puHeaderFileDate;										// detail view name of column date + time
#ifdef USE_THUMBNAILS
		std::string puHeaderFileThumbnails;									// detail view name of column thumbnails
		bool puSortingDirection[5] = {										// true => Ascending, false => Descending
			defaultSortOrderFilename,
			defaultSortOrderType,
			defaultSortOrderSize,
			defaultSortOrderDate,
			defaultSortOrderThumbnails };
#else
		bool puSortingDirection[4] = {										// true => Ascending, false => Descending
			defaultSortOrderFilename,
			defaultSortOrderType,
			defaultSortOrderSize,
			defaultSortOrderDate };
#endif
		SortingFieldEnum puSortingField = SortingFieldEnum::FIELD_FILENAME;	// detail view sorting column
		bool puShowDrives = false;											// drives are shown (only on os windows)

		std::string puDLGpath;												// base path set by user when OpenDialog was called
		std::string puDLGDefaultFileName;									// base default file path name set by user when OpenDialog was called
		size_t puDLGcountSelectionMax = 1U; // 0 for infinite				// base max selection count set by user when OpenDialog was called
		bool puDLGDirectoryMode = false;									// is directory mode (defiend like : puDLGDirectoryMode = (filters.empty()))

		std::string puFsRoot;

	private:
		static std::string prRoundNumber(double vvalue, int n);											// custom rounding number
		static std::string prFormatFileSize(size_t vByteSize);											// format file size field
		static void prCompleteFileInfos(const std::shared_ptr<FileInfos>& FileInfos);					// set time and date infos of a file (detail view mode)
		void prRemoveFileNameInSelection(const std::string& vFileName);									// selection : remove a file name
		void prAddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName);	// selection : add a file name
		void AddFile(const FileDialogInternal& vFileDialogInternal, 
			const std::string& vPath, const std::string& vFileName, const FileType& vFileType);		    // add file called by scandir
		void AddPath(const FileDialogInternal& vFileDialogInternal,
			const std::string& vPath, const std::string& vFileName, const FileType& vFileType);			// add file called by scandir

#if defined(USE_QUICK_PATH_SELECT)
		void ScanDirForPathSelection(const FileDialogInternal& vFileDialogInternal, const std::string& vPath);	// scan the directory for retrieve the path list
		void OpenPathPopup(const FileDialogInternal& vFileDialogInternal, std::vector<std::string>::iterator vPathIter);
#endif // USE_QUICK_PATH_SELECT

		void SetCurrentPath(std::vector<std::string>::iterator vPathIter);

		void ApplyFilteringOnFileList(
			const FileDialogInternal& vFileDialogInternal,
			std::vector<std::shared_ptr<FileInfos>>& vFileInfosList,
			std::vector<std::shared_ptr<FileInfos>>& vFileInfosFilteredList);
		void SortFields(
			const FileDialogInternal& vFileDialogInternal,
			std::vector<std::shared_ptr<FileInfos>>& vFileInfosList,
			std::vector<std::shared_ptr<FileInfos>>& vFileInfosFilteredList);									// will sort a column
		
	public:
		FileManager();
		bool IsComposerEmpty();
		size_t GetComposerSize();
		bool IsFileListEmpty();
		bool IsPathListEmpty();
		bool IsFilteredListEmpty();
		bool IsPathFilteredListEmpty();
		size_t GetFullFileListSize();
		std::shared_ptr<FileInfos> GetFullFileAt(size_t vIdx);
		size_t GetFilteredListSize();
		size_t GetPathFilteredListSize();
		std::shared_ptr<FileInfos> GetFilteredFileAt(size_t vIdx);
		std::shared_ptr<FileInfos> GetFilteredPathAt(size_t vIdx);
		std::vector<std::string>::iterator GetCurrentPopupComposedPath();
		bool IsFileNameSelected(const std::string& vFileName);
		std::string GetBack();
		void ClearComposer();
		void ClearFileLists();																			// clear file list, will destroy thumbnail textures
		void ClearPathLists();																			// clear path list, will destroy thumbnail textures
		void ClearAll();
		void ApplyFilteringOnFileList(const FileDialogInternal& vFileDialogInternal);
		void SortFields(const FileDialogInternal& vFileDialogInternal);									// will sort a column
		void OpenCurrentPath(const FileDialogInternal& vFileDialogInternal);							// set the path of the dialog, will launch the directory scan for populate the file listview
		bool GetDrives();																				// list drives on windows platform
		bool CreateDir(const std::string& vPath);														// create a directory on the file system
		std::string ComposeNewPath(std::vector<std::string>::iterator vIter);									// compose a path from the compose path widget
		bool SetPathOnParentDirectoryIfAny();															// compose paht on parent directory
		std::string GetCurrentPath();																	// get the current path
		void SetCurrentPath(const std::string& vCurrentPath);											// set the current path
		static bool IsFileExist(const std::string& vFile);
		void SetDefaultFileName(const std::string& vFileName);
		bool SelectDirectory(const std::shared_ptr<FileInfos>& vInfos);										// enter directory
		void SelectFileName(const FileDialogInternal& vFileDialogInternal, 
			const std::shared_ptr<FileInfos>& vInfos);															// select filename
		
		//depend of dirent.h
		void SetCurrentDir(const std::string& vPath);													// define current directory for scan
		void ScanDir(const FileDialogInternal& vFileDialogInternal, const std::string& vPath);			// scan the directory for retrieve the file list
		
	public:
		std::string GetResultingPath();
		std::string GetResultingFileName(FileDialogInternal& vFileDialogInternal);
		std::string GetResultingFilePathName(FileDialogInternal& vFileDialogInternal);
		std::map<std::string, std::string> GetResultingSelection();

	public:
		void DrawDirectoryCreation(const FileDialogInternal& vFileDialogInternal);						// draw directory creation widget
		void DrawPathComposer(const FileDialogInternal& vFileDialogInternal);							// draw path composer widget
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_THUMBNAILS
	typedef std::function<void(IGFD_Thumbnail_Info*)> CreateThumbnailFun;	// texture 2d creation function binding
	typedef std::function<void(IGFD_Thumbnail_Info*)> DestroyThumbnailFun;	// texture 2d destroy function binding
#endif
	class ThumbnailFeature
	{
	protected:
		ThumbnailFeature();
		~ThumbnailFeature();

		void NewThumbnailFrame(FileDialogInternal& vFileDialogInternal);
		void EndThumbnailFrame(FileDialogInternal& vFileDialogInternal);
		void QuitThumbnailFrame(FileDialogInternal& vFileDialogInternal);

#ifdef USE_THUMBNAILS
	protected:
		enum class DisplayModeEnum
		{
			FILE_LIST = 0,
			THUMBNAILS_LIST,
			THUMBNAILS_GRID
		};

	private:
		uint32_t prCountFiles = 0U;
		bool prIsWorking = false;
		std::shared_ptr<std::thread> prThumbnailGenerationThread = nullptr;
		std::list<std::shared_ptr<FileInfos>> prThumbnailFileDatasToGet;	// base container
		std::mutex prThumbnailFileDatasToGetMutex;
		std::list<std::shared_ptr<FileInfos>> prThumbnailToCreate;			// base container
		std::mutex prThumbnailToCreateMutex;
		std::list<IGFD_Thumbnail_Info> prThumbnailToDestroy;				// base container
		std::mutex prThumbnailToDestroyMutex;

		CreateThumbnailFun prCreateThumbnailFun = nullptr;
		DestroyThumbnailFun prDestroyThumbnailFun = nullptr;

	protected:
		DisplayModeEnum prDisplayMode = DisplayModeEnum::FILE_LIST;

	protected:
		// will be call in cpu zone (imgui computations, will call a texture file retrieval thread)
		void prStartThumbnailFileDatasExtraction();								// start the thread who will get byte buffer from image files
		bool prStopThumbnailFileDatasExtraction();								// stop the thread who will get byte buffer from image files
		void prThreadThumbnailFileDatasExtractionFunc();						// the thread who will get byte buffer from image files
		void prDrawThumbnailGenerationProgress();								// a little progressbar who will display the texture gen status
		void prAddThumbnailToLoad(const std::shared_ptr<FileInfos>& vFileInfos);		// add texture to load in the thread
		void prAddThumbnailToCreate(const std::shared_ptr<FileInfos>& vFileInfos);
		void prAddThumbnailToDestroy(const IGFD_Thumbnail_Info& vIGFD_Thumbnail_Info);
		void prDrawDisplayModeToolBar();										// draw display mode toolbar (file list, thumbnails list, small thumbnails grid, big thumbnails grid)
		void prClearThumbnails(FileDialogInternal& vFileDialogInternal);

	public:
		void SetCreateThumbnailCallback(const CreateThumbnailFun& vCreateThumbnailFun);
		void SetDestroyThumbnailCallback(const DestroyThumbnailFun& vCreateThumbnailFun);
		
		// must be call in gpu zone (rendering, possibly one rendering thread)
		void ManageGPUThumbnails();	// in gpu rendering zone, whill create or destroy texture
#endif
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class BookMarkFeature
	{
	protected:
		BookMarkFeature();

#ifdef USE_BOOKMARK
	private:
		struct BookmarkStruct
		{
			std::string name;			// name of the bookmark
			
			// todo: the path could be relative, better if the app is movedn but bookmarked path can be outside of the app
			std::string path;			// absolute path of the bookmarked directory 

			bool defined_by_code = false;	// defined by code, can be used for rpevent serialization / deserialization
		};

	private:
		ImGuiListClipper prBookmarkClipper;
		std::vector<BookmarkStruct> prBookmarks;
		char prBookmarkEditBuffer[MAX_FILE_DIALOG_NAME_BUFFER] = "";

	protected:
		float prBookmarkWidth = 200.0f;
		bool prBookmarkPaneShown = false;
		
	protected:
		void prDrawBookmarkButton();															// draw bookmark button
		bool prDrawBookmarkPane(FileDialogInternal& vFileDialogInternal, const ImVec2& vSize);	// draw bookmark Pane

	public:
		std::string SerializeBookmarks(								// serialize bookmarks : return bookmark buffer to save in a file
			const bool& vDontSerializeCodeBasedBookmarks = true);	// for avoid serialization of bookmarks added by code
		void DeserializeBookmarks(									// deserialize bookmarks : load bookmark buffer to load in the dialog (saved from previous use with SerializeBookmarks())
			const std::string& vBookmarks);							// bookmark buffer to load
		void AddBookmark(											// add a bookmark by code
			const std::string& vBookMarkName,						// bookmark name
			const std::string& vBookMarkPath);						// bookmark path
		bool RemoveBookmark(										// remove a bookmark by code, return true if succeed
			const std::string& vBookMarkName);						// bookmark name to remove

#endif // USE_BOOKMARK
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// file localization by input chat // widget flashing
	class KeyExplorerFeature
	{
	protected:
		KeyExplorerFeature();

#ifdef USE_EXPLORATION_BY_KEYS
	private:
		size_t prFlashedItem = 0;																// flash when select by char
		float prFlashAlpha = 0.0f;																// flash when select by char
		float prFlashAlphaAttenInSecs = 1.0f;													// fps display dependant
		size_t prLocateFileByInputChar_lastFileIdx = 0;
		ImWchar prLocateFileByInputChar_lastChar = 0;
		int prLocateFileByInputChar_InputQueueCharactersSize = 0;
		bool prLocateFileByInputChar_lastFound = false;

	protected:
		void prLocateByInputKey(FileDialogInternal& vFileDialogInternal);						// select a file line in listview according to char key
		bool prLocateItem_Loop(FileDialogInternal& vFileDialogInternal, ImWchar vC);			// restrat for start of list view if not found a corresponding file
		void prExploreWithkeys(FileDialogInternal& vFileDialogInternal, ImGuiID vListViewID);	// select file/directory line in listview accroding to up/down enter/backspace keys
		static bool prFlashableSelectable(																// custom flashing selectable widgets, for flash the selected line in a short time
			const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
			bool vFlashing = false, const ImVec2& size = ImVec2(0, 0));
		void prStartFlashItem(size_t vIdx);														// define than an item must be flashed
		bool prBeginFlashItem(size_t vIdx);														// start the flashing of a line in lsit view
		static void prEndFlashItem();																	// end the fleshing accrdoin to var prFlashAlphaAttenInSecs

	public:
		void SetFlashingAttenuationInSeconds(													// set the flashing time of the line in file list when use exploration keys
			float vAttenValue);																	// set the attenuation (from flashed to not flashed) in seconds
#endif // USE_EXPLORATION_BY_KEYS
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	typedef void* UserDatas;
	typedef std::function<void(const char*, UserDatas, bool*)> PaneFun;							// side pane function binding
	class FileDialogInternal
	{
	public:
		FileManager puFileManager;
		FilterManager puFilterManager;
		SearchManager puSearchManager;

	public:
		std::string puName;
		bool puShowDialog = false;
		ImVec2 puDialogCenterPos = ImVec2(0, 0);												// center pos for display the confirm overwrite dialog
		int puLastImGuiFrameCount = 0;															// to be sure than only one dialog displayed per frame
		float puFooterHeight = 0.0f;
		bool puCanWeContinue = true;															// events
		bool puOkResultToConfirm = false;														// to confim if ok for OverWrite
		bool puIsOk = false;
		bool puFileInputIsActive = false;														// when input text for file or directory is active
		bool puFileListViewIsActive = false;													// when list view is active
		std::string puDLGkey;
		std::string puDLGtitle;
		ImGuiFileDialogFlags puDLGflags = ImGuiFileDialogFlags_None;
		UserDatas puDLGuserDatas = nullptr;
		PaneFun puDLGoptionsPane = nullptr;
		float puDLGoptionsPaneWidth = 0.0f;
		bool puNeedToExitDialog = false;

		bool puUseCustomLocale = false;
		int puLocaleCategory = LC_ALL;	// locale category to use
		std::string puLocaleBegin; // the locale who will be applied at start of the display dialog
		std::string puLocaleEnd; // the locale who will be applaied at end of the display dialog

	public:
		void NewFrame();			// new frame, so maybe neded to do somethings, like reset events
		void EndFrame();			// end frame, so maybe neded to do somethings fater all
		void ResetForNewDialog();	// reset what is needed to reset for the openging of a new dialog
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class FileDialog : 
		public BookMarkFeature, 
		public KeyExplorerFeature, 
		public ThumbnailFeature
	{
	private:
		FileDialogInternal prFileDialogInternal;
		ImGuiListClipper prFileListClipper;
		ImGuiListClipper prPathListClipper;
		float prOkCancelButtonWidth = 0.0f;

	public:
		bool puAnyWindowsHovered = false;							// not remember why haha :) todo : to check if we can remove
		 
	public:
		static FileDialog* Instance()								// Singleton for easier accces form anywhere but only one dialog at a time
		{
			static FileDialog _instance;
			return &_instance;
		}

	public:
		FileDialog();												// ImGuiFileDialog Constructor. can be used for have many dialog at same tiem (not possible with singleton)
		virtual ~FileDialog();										// ImGuiFileDialog Destructor

		// standard dialog
		void OpenDialog(											// open simple dialog (path and fileName can be specified)
			const std::string& vKey,								// key dialog
			const std::string& vTitle,								// title
			const char* vFilters,									// filters
			const std::string& vPath,								// path
			const std::string& vFileName,							// defaut file name
			const int& vCountSelectionMax = 1,						// count selection max
			UserDatas vUserDatas = nullptr,							// user datas (can be retrieved in pane)
			ImGuiFileDialogFlags vFlags = 0);						// ImGuiFileDialogFlags 

		void OpenDialog(											// open simple dialog (path and filename are obtained from filePathName)
			const std::string& vKey,								// key dialog
			const std::string& vTitle,								// title
			const char* vFilters,									// filters
			const std::string& vFilePathName,						// file path name (will be decompsoed in path and fileName)
			const int& vCountSelectionMax = 1,						// count selection max
			UserDatas vUserDatas = nullptr,							// user datas (can be retrieved in pane)
			ImGuiFileDialogFlags vFlags = 0);						// ImGuiFileDialogFlags 

		// with pane
		void OpenDialog(											// open dialog with custom right pane (path and fileName can be specified)
			const std::string& vKey,								// key dialog
			const std::string& vTitle,								// title
			const char* vFilters,									// filters
			const std::string& vPath,								// path
			const std::string& vFileName,							// defaut file name
			const PaneFun& vSidePane,								// side pane
			const float& vSidePaneWidth = 250.0f,					// side pane width
			const int& vCountSelectionMax = 1,						// count selection max
			UserDatas vUserDatas = nullptr,							// user datas (can be retrieved in pane)
			ImGuiFileDialogFlags vFlags = 0);						// ImGuiFileDialogFlags 

		void OpenDialog(											// open dialog with custom right pane (path and filename are obtained from filePathName)
			const std::string& vKey,								// key dialog
			const std::string& vTitle,								// title
			const char* vFilters,									// filters
			const std::string& vFilePathName,						// file path name (will be decompsoed in path and fileName)
			const PaneFun& vSidePane,								// side pane
			const float& vSidePaneWidth = 250.0f,					// side pane width
			const int& vCountSelectionMax = 1,						// count selection max
			UserDatas vUserDatas = nullptr,							// user datas (can be retrieved in pane)
			ImGuiFileDialogFlags vFlags = 0);						// ImGuiFileDialogFlags 

		// Display / Close dialog form
		bool Display(												// Display the dialog. return true if a result was obtained (Ok or not)
			const std::string& vKey,								// key dialog to display (if not the same key as defined by OpenDialog => no opening)
			ImGuiWindowFlags vFlags = ImGuiWindowFlags_NoCollapse,	// ImGuiWindowFlags
			ImVec2 vMinSize = ImVec2(0, 0),							// mininmal size contraint for the ImGuiWindow
			ImVec2 vMaxSize = ImVec2(FLT_MAX, FLT_MAX));			// maximal size contraint for the ImGuiWindow
		void Close();												// close dialog

		// queries
		bool WasOpenedThisFrame(const std::string& vKey) const;		// say if the dialog key was already opened this frame
		bool WasOpenedThisFrame() const;							// say if the dialog was already opened this frame
		bool IsOpened(const std::string& vKey) const;				// say if the key is opened
		bool IsOpened() const;										// say if the dialog is opened somewhere
		std::string GetOpenedKey() const;							// return the dialog key who is opened, return nothing if not opened

		// get result
		bool IsOk() const;											// true => Dialog Closed with Ok result / false : Dialog closed with cancel result
		std::map<std::string, std::string> GetSelection();			// Open File behavior : will return selection via a map<FileName, FilePathName>
		std::string GetFilePathName();								// Save File behavior : will always return the content of the field with current filter extention and current path
		std::string GetCurrentFileName();							// Save File behavior : will always return the content of the field with current filter extention
		std::string GetCurrentPath();								// will return current path
		std::string GetCurrentFilter();								// will return selected filter
		UserDatas GetUserDatas() const;								// will return user datas send with Open Dialog

		// file style by extentions
		void SetFileStyle(											// SetExtention datas for have custom display of particular file type
			const IGFD_FileStyleFlags& vFlags,						// file style
			const char* vCriteria,									// extention filter to tune
			const FileStyle& vInfos);								// Filter Extention Struct who contain Color and Icon/Text for the display of the file with extention filter
		void SetFileStyle(											// SetExtention datas for have custom display of particular file type
			const IGFD_FileStyleFlags& vFlags,							// file style
			const char* vCriteria,									// extention filter to tune
			const ImVec4& vColor,									// wanted color for the display of the file with extention filter
			const std::string& vIcon = "",							// wanted text or icon of the file with extention filter
			ImFont *vFont = nullptr);                               // wantes font
		bool GetFileStyle(											// GetExtention datas. return true is extention exist
			const IGFD_FileStyleFlags& vFlags,							// file style
			const std::string& vCriteria,									// extention filter (same as used in SetExtentionInfos)
			ImVec4* vOutColor,										// color to retrieve
			std::string* vOutIcon = nullptr,						// icon or text to retrieve
            ImFont** vOutFont = nullptr);                           // font to retreive
		void ClearFilesStyle();										// clear extentions setttings

		void SetLocales(											// set locales to use before and after the dialog display
			const int& vLocaleCategory,								// set local category
			const std::string& vLocaleBegin,						// locale to use at begining of the dialog display
			const std::string& vLocaleEnd);							// locale to use at the end of the dialog display

	protected:
		void NewFrame();											// new frame just at begining of display
		void EndFrame();											// end frame just at end of display
		void QuitFrame();											// quit frame when qui quit the dialog

		// others
		bool prConfirm_Or_OpenOverWriteFileDialog_IfNeeded(
			bool vLastAction, ImGuiWindowFlags vFlags);				// treatment of the result, start the confirm to overwrite dialog if needed (if defined with flag)
	
	public:
		// dialog parts
		virtual void prDrawHeader();								// draw header part of the dialog (bookmark btn, dir creation, path composer, search bar)
		virtual void prDrawContent();								// draw content part of the dialog (bookmark pane, file list, side pane)
		virtual bool prDrawFooter();								// draw footer part of the dialog (file field, fitler combobox, ok/cancel btn's)

		// widgets components
#if defined(USE_QUICK_PATH_SELECT)
		virtual void DisplayPathPopup(ImVec2 vSize);				// draw path popup when click on a \ or /
#endif // USE_QUICK_PATH_SELECT
		virtual bool prDrawValidationButtons();						// draw validations btns, ok, cancel buttons
		virtual bool prDrawOkButton();								// draw ok button
		virtual bool prDrawCancelButton();							// draw cancel button
		virtual void prDrawSidePane(float vHeight);					// draw side pane
		virtual void prSelectableItem(int vidx, 
			std::shared_ptr<FileInfos> vInfos, 
			bool vSelected, const char* vFmt, ...);					// draw a custom selectable behavior item
		virtual void prDrawFileListView(ImVec2 vSize);				// draw file list view (default mode)

#ifdef USE_THUMBNAILS
		virtual void prDrawThumbnailsListView(ImVec2 vSize);		// draw file list view with small thumbnails on the same line
		virtual void prDrawThumbnailsGridView(ImVec2 vSize);		// draw a grid of small thumbnails
#endif

		// to be called only by these function and theirs overrides
		// - prDrawFileListView
		// - prDrawThumbnailsListView
		// - prDrawThumbnailsGridView
		void prBeginFileColorIconStyle(
			std::shared_ptr<FileInfos> vFileInfos, 
			bool& vOutShowColor, 
			std::string& vOutStr, 
			ImFont** vOutFont);										// begin style apply of filter with color an icon if any
		void prEndFileColorIconStyle(
			const bool& vShowColor,
			ImFont* vFont);											// end style apply of filter
	};
}

typedef IGFD::UserDatas IGFDUserDatas;
typedef IGFD::PaneFun IGFDPaneFun;
typedef IGFD::FileDialog ImGuiFileDialog;
#else // __cplusplus
typedef struct ImGuiFileDialog ImGuiFileDialog;
typedef struct IGFD_Selection_Pair IGFD_Selection_Pair;
typedef struct IGFD_Selection IGFD_Selection;
#endif // __cplusplus

// C Interface

#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
#ifdef IMGUIFILEDIALOG_NO_EXPORT
#define API
#else // IMGUIFILEDIALOG_NO_EXPORT
#define API __declspec(dllexport)
#endif // IMGUIFILEDIALOG_NO_EXPORT
#else // defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define API  __attribute__((__visibility__("default")))
#else // __GNUC__
#define API
#endif // __GNUC__
#endif // defined _WIN32 || defined __CYGWIN__

#ifdef __cplusplus
#define IMGUIFILEDIALOG_API extern "C" API 
#else // __cplusplus
#define IMGUIFILEDIALOG_API
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// C API ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct IGFD_Selection_Pair
{
	char* fileName;
	char* filePathName;
};

IMGUIFILEDIALOG_API IGFD_Selection_Pair IGFD_Selection_Pair_Get();										// return an initialized IGFD_Selection_Pair			
IMGUIFILEDIALOG_API void IGFD_Selection_Pair_DestroyContent(IGFD_Selection_Pair* vSelection_Pair);		// destroy the content of a IGFD_Selection_Pair

struct IGFD_Selection
{
	IGFD_Selection_Pair* table;	// 0
	size_t count;						// 0U
};

IMGUIFILEDIALOG_API IGFD_Selection IGFD_Selection_Get();											// return an initialized IGFD_Selection
IMGUIFILEDIALOG_API void IGFD_Selection_DestroyContent(IGFD_Selection* vSelection);					// destroy the content of a IGFD_Selection

// constructor / destructor
IMGUIFILEDIALOG_API ImGuiFileDialog* IGFD_Create(void);												// create the filedialog context
IMGUIFILEDIALOG_API void IGFD_Destroy(ImGuiFileDialog* vContext);									// destroy the filedialog context

typedef void (*IGFD_PaneFun)(const char*, void*, bool*);											// callback fucntion for display the pane

#ifdef USE_THUMBNAILS
typedef void (*IGFD_CreateThumbnailFun)(IGFD_Thumbnail_Info*);										// callback function for create thumbnail texture
typedef void (*IGFD_DestroyThumbnailFun)(IGFD_Thumbnail_Info*);										// callback fucntion for destroy thumbnail texture
#endif // USE_THUMBNAILS

IMGUIFILEDIALOG_API void IGFD_OpenDialog(					// open a standard dialog
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vKey,										// key dialog
	const char* vTitle,										// title
	const char* vFilters,									// filters/filter collections. set it to null for directory mode 
	const char* vPath,										// path
	const char* vFileName,									// defaut file name
	const int vCountSelectionMax,							// count selection max
	void* vUserDatas,										// user datas (can be retrieved in pane)
	ImGuiFileDialogFlags vFlags);							// ImGuiFileDialogFlags 

IMGUIFILEDIALOG_API void IGFD_OpenDialog2(					// open a standard dialog
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vKey,										// key dialog
	const char* vTitle,										// title
	const char* vFilters,									// filters/filter collections. set it to null for directory mode 
	const char* vFilePathName,								// defaut file path name (path and filename witl be extracted from it)
	const int vCountSelectionMax,							// count selection max
	void* vUserDatas,										// user datas (can be retrieved in pane)
	ImGuiFileDialogFlags vFlags);							// ImGuiFileDialogFlags 

IMGUIFILEDIALOG_API void IGFD_OpenPaneDialog(				// open a standard dialog with pane
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vKey,										// key dialog
	const char* vTitle,										// title
	const char* vFilters,									// filters/filter collections. set it to null for directory mode 
	const char* vPath,										// path
	const char* vFileName,									// defaut file name
	const IGFD_PaneFun vSidePane,							// side pane
	const float vSidePaneWidth,								// side pane base width
	const int vCountSelectionMax,							// count selection max
	void* vUserDatas,										// user datas (can be retrieved in pane)
	ImGuiFileDialogFlags vFlags);							// ImGuiFileDialogFlags 

IMGUIFILEDIALOG_API void IGFD_OpenPaneDialog2(				// open a standard dialog with pane
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vKey,										// key dialog
	const char* vTitle,										// title
	const char* vFilters,									// filters/filter collections. set it to null for directory mode 
	const char* vFilePathName,								// defaut file name (path and filename witl be extracted from it)
	const IGFD_PaneFun vSidePane,							// side pane
	const float vSidePaneWidth,								// side pane base width
	const int vCountSelectionMax,							// count selection max
	void* vUserDatas,										// user datas (can be retrieved in pane)
	ImGuiFileDialogFlags vFlags);							// ImGuiFileDialogFlags

IMGUIFILEDIALOG_API bool IGFD_DisplayDialog(				// Display the dialog
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vKey,										// key dialog to display (if not the same key as defined by OpenDialog => no opening)
	ImGuiWindowFlags vFlags,								// ImGuiWindowFlags
	ImVec2 vMinSize,										// mininmal size contraint for the ImGuiWindow
	ImVec2 vMaxSize);										// maximal size contraint for the ImGuiWindow

IMGUIFILEDIALOG_API void IGFD_CloseDialog(					// Close the dialog
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context			

IMGUIFILEDIALOG_API bool IGFD_IsOk(							// true => Dialog Closed with Ok result / false : Dialog closed with cancel result
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context		

IMGUIFILEDIALOG_API bool IGFD_WasKeyOpenedThisFrame(		// say if the dialog key was already opened this frame
	ImGuiFileDialog* vContext, 								// ImGuiFileDialog context		
	const char* vKey);

IMGUIFILEDIALOG_API bool IGFD_WasOpenedThisFrame(			// say if the dialog was already opened this frame
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context	

IMGUIFILEDIALOG_API bool IGFD_IsKeyOpened(					// say if the dialog key is opened
	ImGuiFileDialog* vContext, 								// ImGuiFileDialog context		
	const char* vCurrentOpenedKey);							// the dialog key

IMGUIFILEDIALOG_API bool IGFD_IsOpened(						// say if the dialog is opened somewhere	
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context		

IMGUIFILEDIALOG_API IGFD_Selection IGFD_GetSelection(		// Open File behavior : will return selection via a map<FileName, FilePathName>
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context		

IMGUIFILEDIALOG_API char* IGFD_GetFilePathName(				// Save File behavior : will always return the content of the field with current filter extention and current path, WARNINGS you are responsible to free it
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context				

IMGUIFILEDIALOG_API char* IGFD_GetCurrentFileName(			// Save File behavior : will always return the content of the field with current filter extention, WARNINGS you are responsible to free it
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context				

IMGUIFILEDIALOG_API char* IGFD_GetCurrentPath(				// will return current path, WARNINGS you are responsible to free it
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context					

IMGUIFILEDIALOG_API char* IGFD_GetCurrentFilter(			// will return selected filter, WARNINGS you are responsible to free it
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context						

IMGUIFILEDIALOG_API void* IGFD_GetUserDatas(				// will return user datas send with Open Dialog
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context											

IMGUIFILEDIALOG_API void IGFD_SetFileStyle(					// SetExtention datas for have custom display of particular file type
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	IGFD_FileStyleFlags vFileStyleFlags,					// file style type
	const char* vFilter,									// extention filter to tune
	ImVec4 vColor,											// wanted color for the display of the file with extention filter
	const char* vIconText,									// wanted text or icon of the file with extention filter (can be sued with font icon)
	ImFont* vFont);											// wanted font pointer

IMGUIFILEDIALOG_API void IGFD_SetFileStyle2(				// SetExtention datas for have custom display of particular file type
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	IGFD_FileStyleFlags vFileStyleFlags,					// file style type
	const char* vFilter,									// extention filter to tune
	float vR, float vG, float vB, float vA,					// wanted color channels RGBA for the display of the file with extention filter
	const char* vIconText,									// wanted text or icon of the file with extention filter (can be sued with font icon)
	ImFont* vFont);											// wanted font pointer

IMGUIFILEDIALOG_API bool IGFD_GetFileStyle(
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	IGFD_FileStyleFlags vFileStyleFlags,					// file style type
	const char* vFilter,									// extention filter (same as used in SetExtentionInfos)
	ImVec4* vOutColor,										// color to retrieve
	char** vOutIconText,									// icon or text to retrieve, WARNINGS you are responsible to free it
	ImFont** vOutFont);										// font pointer to retrived

IMGUIFILEDIALOG_API void IGFD_ClearFilesStyle(				// clear extentions setttings
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context

IMGUIFILEDIALOG_API void SetLocales(						// set locales to use before and after display
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	const int vCategory,									// set local category
	const char* vBeginLocale,								// locale to use at begining of the dialog display
	const char* vEndLocale);								// locale to set at end of the dialog display

#ifdef USE_EXPLORATION_BY_KEYS
IMGUIFILEDIALOG_API void IGFD_SetFlashingAttenuationInSeconds(	// set the flashing time of the line in file list when use exploration keys
	ImGuiFileDialog* vContext,									// ImGuiFileDialog context 
	float vAttenValue);											// set the attenuation (from flashed to not flashed) in seconds
#endif

#ifdef USE_BOOKMARK
IMGUIFILEDIALOG_API char* IGFD_SerializeBookmarks(			// serialize bookmarks : return bookmark buffer to save in a file, WARNINGS you are responsible to free it
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	bool vDontSerializeCodeBasedBookmarks);					// for avoid serialization of bookmarks added by code

IMGUIFILEDIALOG_API void IGFD_DeserializeBookmarks(			// deserialize bookmarks : load bookmar buffer to load in the dialog (saved from previous use with SerializeBookmarks())
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	const char* vBookmarks);								// bookmark buffer to load 

IMGUIFILEDIALOG_API void IGFD_AddBookmark(					// add a bookmark by code
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vBookMarkName,								// bookmark name
	const char* vBookMarkPath);								// bookmark path

IMGUIFILEDIALOG_API void IGFD_RemoveBookmark(					// remove a bookmark by code, return true if succeed
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context
	const char* vBookMarkName);								// bookmark name to remove
#endif

#ifdef USE_THUMBNAILS
IMGUIFILEDIALOG_API void SetCreateThumbnailCallback(		// define the callback for create the thumbnails texture
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	IGFD_CreateThumbnailFun vCreateThumbnailFun);			// the callback for create the thumbnails texture

IMGUIFILEDIALOG_API void SetDestroyThumbnailCallback(		// define the callback for destroy the thumbnails texture
	ImGuiFileDialog* vContext,								// ImGuiFileDialog context 
	IGFD_DestroyThumbnailFun vDestroyThumbnailFun);			// the callback for destroy the thumbnails texture

IMGUIFILEDIALOG_API void ManageGPUThumbnails(				// must be call in gpu zone, possibly a thread, will call the callback for create / destroy the textures
	ImGuiFileDialog* vContext);								// ImGuiFileDialog context 
#endif // USE_THUMBNAILS

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // IMGUIFILEDIALOG_H



#ifdef __cplusplus

#include <cfloat>
#include <cstring> // stricmp / strcasecmp
#include <cstdarg> // variadic
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include <cstdio>
#include <cerrno>

// this option need c++17
#ifdef USE_STD_FILESYSTEM
#include <filesystem>
#include <exception>
#endif // USE_STD_FILESYSTEM

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || \
	defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#define _IGFD_WIN_
#define stat _stat
#define stricmp _stricmp
#include <cctype>
// this option need c++17
#ifdef USE_STD_FILESYSTEM
#include <Windows.h>
#else
#include <dirent.h>
#endif // USE_STD_FILESYSTEM
#define PATH_SEP '\\'
#ifndef PATH_MAX
#define PATH_MAX 260
#endif // PATH_MAX
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || \
	defined(__NetBSD__) || defined(__APPLE__) || defined (__EMSCRIPTEN__)
#define _IGFD_UNIX_
#define stricmp strcasecmp
#include <sys/types.h>
// this option need c++17
#ifndef USE_STD_FILESYSTEM
#include <dirent.h> 
#endif // USE_STD_FILESYSTEM
#define PATH_SEP '/'
#endif // _IGFD_UNIX_

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cstdlib>
#include <algorithm>
#include <iostream>

#ifdef USE_THUMBNAILS
#ifndef DONT_DEFINE_AGAIN__STB_IMAGE_IMPLEMENTATION
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // STB_IMAGE_IMPLEMENTATION
#endif // DONT_DEFINE_AGAIN__STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#ifndef DONT_DEFINE_AGAIN__STB_IMAGE_RESIZE_IMPLEMENTATION
#ifndef STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#endif // STB_IMAGE_RESIZE_IMPLEMENTATION
#endif // DONT_DEFINE_AGAIN__STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#endif // USE_THUMBNAILS

namespace IGFD
{
	// float comparisons
#ifndef IS_FLOAT_DIFFERENT
#define IS_FLOAT_DIFFERENT(a,b) (fabs((a) - (b)) > FLT_EPSILON)
#endif // IS_FLOAT_DIFFERENT
#ifndef IS_FLOAT_EQUAL
#define IS_FLOAT_EQUAL(a,b) (fabs((a) - (b)) < FLT_EPSILON)
#endif // IS_FLOAT_EQUAL
	// width of filter combobox
#ifndef FILTER_COMBO_WIDTH
#define FILTER_COMBO_WIDTH 150.0f
#endif // FILTER_COMBO_WIDTH
	// for lets you define your button widget
	// if you have like me a special bi-color button
#ifndef IMGUI_PATH_BUTTON
#define IMGUI_PATH_BUTTON ImGui::Button
#endif // IMGUI_PATH_BUTTON
#ifndef IMGUI_BUTTON
#define IMGUI_BUTTON ImGui::Button
#endif // IMGUI_BUTTON
	// locales
#ifndef createDirButtonString
#define createDirButtonString "+"
#endif // createDirButtonString
#ifndef okButtonString
#define okButtonString "OK"
#endif // okButtonString
#ifndef okButtonWidth
#define okButtonWidth 0.0f
#endif // okButtonWidth
#ifndef cancelButtonString
#define cancelButtonString "Cancel"
#endif // cancelButtonString
#ifndef cancelButtonWidth
#define cancelButtonWidth 0.0f
#endif // cancelButtonWidth
#ifndef okCancelButtonAlignement
#define okCancelButtonAlignement 0.0f
#endif // okCancelButtonAlignement
#ifndef invertOkAndCancelButtons
	// 0 => disabled, 1 => enabled
#define invertOkAndCancelButtons 0
#endif // invertOkAndCancelButtons
#ifndef resetButtonString
#define resetButtonString "R"
#endif // resetButtonString
#ifndef drivesButtonString
#define drivesButtonString "Drives"
#endif // drivesButtonString
#ifndef editPathButtonString
#define editPathButtonString "E"
#endif // editPathButtonString
#ifndef searchString
#define searchString "Search :"
#endif // searchString
#ifndef dirEntryString
#define dirEntryString "[Dir]"
#endif // dirEntryString
#ifndef linkEntryString
#define linkEntryString "[Link]"
#endif // linkEntryString
#ifndef fileEntryString
#define fileEntryString "[File]"
#endif // fileEntryString
#ifndef fileNameString
#define fileNameString "File Name :"
#endif // fileNameString
#ifndef dirNameString
#define dirNameString "Directory Path :"
#endif // dirNameString
#ifndef buttonResetSearchString
#define buttonResetSearchString "Reset search"
#endif // buttonResetSearchString
#ifndef buttonDriveString
#define buttonDriveString "Drives"
#endif // buttonDriveString
#ifndef buttonEditPathString
#define buttonEditPathString "Edit path\nYou can also right click on path buttons"
#endif // buttonEditPathString
#ifndef buttonResetPathString
#define buttonResetPathString "Reset to current directory"
#endif // buttonResetPathString
#ifndef buttonCreateDirString
#define buttonCreateDirString "Create Directory"
#endif // buttonCreateDirString
#ifndef tableHeaderAscendingIcon
#define tableHeaderAscendingIcon "A|"
#endif // tableHeaderAscendingIcon
#ifndef tableHeaderDescendingIcon
#define tableHeaderDescendingIcon "D|"
#endif // tableHeaderDescendingIcon
#ifndef tableHeaderFileNameString
#define tableHeaderFileNameString "File name"
#endif // tableHeaderFileNameString
#ifndef tableHeaderFileTypeString
#define tableHeaderFileTypeString "Type"
#endif // tableHeaderFileTypeString
#ifndef tableHeaderFileSizeString
#define tableHeaderFileSizeString "Size"
#endif // tableHeaderFileSizeString
#ifndef tableHeaderFileDateString
#define tableHeaderFileDateString "Date"
#endif // tableHeaderFileDateString
#ifndef fileSizeBytes
#define fileSizeBytes "o"
#endif // fileSizeBytes
#ifndef fileSizeKiloBytes
#define fileSizeKiloBytes "Ko"
#endif // fileSizeKiloBytes
#ifndef fileSizeMegaBytes
#define fileSizeMegaBytes "Mo"
#endif // fileSizeMegaBytes
#ifndef fileSizeGigaBytes
#define fileSizeGigaBytes "Go"
#endif // fileSizeGigaBytes
#ifndef OverWriteDialogTitleString
#define OverWriteDialogTitleString "The file Already Exist !"
#endif // OverWriteDialogTitleString
#ifndef OverWriteDialogMessageString
#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#endif // OverWriteDialogMessageString
#ifndef OverWriteDialogConfirmButtonString
#define OverWriteDialogConfirmButtonString "Confirm"
#endif // OverWriteDialogConfirmButtonString
#ifndef OverWriteDialogCancelButtonString
#define OverWriteDialogCancelButtonString "Cancel"
#endif // OverWriteDialogCancelButtonString
	// see strftime functionin <ctime> for customize
#ifndef DateTimeFormat
#define DateTimeFormat "%Y/%m/%d %H:%M"
#endif // DateTimeFormat
#ifdef USE_THUMBNAILS
#ifndef tableHeaderFileThumbnailsString
#define tableHeaderFileThumbnailsString "Thumbnails"
#endif // tableHeaderFileThumbnailsString
#ifndef DisplayMode_FilesList_ButtonString
#define DisplayMode_FilesList_ButtonString "FL"
#endif // DisplayMode_FilesList_ButtonString
#ifndef DisplayMode_FilesList_ButtonHelp
#define DisplayMode_FilesList_ButtonHelp "File List"
#endif // DisplayMode_FilesList_ButtonHelp
#ifndef DisplayMode_ThumbailsList_ButtonString
#define DisplayMode_ThumbailsList_ButtonString "TL"
#endif // DisplayMode_ThumbailsList_ButtonString
#ifndef DisplayMode_ThumbailsList_ButtonHelp
#define DisplayMode_ThumbailsList_ButtonHelp "Thumbnails List"
#endif // DisplayMode_ThumbailsList_ButtonHelp
#ifndef DisplayMode_ThumbailsGrid_ButtonString
#define DisplayMode_ThumbailsGrid_ButtonString "TG"
#endif // DisplayMode_ThumbailsGrid_ButtonString
#ifndef DisplayMode_ThumbailsGrid_ButtonHelp
#define DisplayMode_ThumbailsGrid_ButtonHelp "Thumbnails Grid"
#endif // DisplayMode_ThumbailsGrid_ButtonHelp
#ifndef DisplayMode_ThumbailsList_ImageHeight 
#define DisplayMode_ThumbailsList_ImageHeight 32.0f
#endif // DisplayMode_ThumbailsList_ImageHeight
#ifndef IMGUI_RADIO_BUTTON
	inline bool inRadioButton(const char* vLabel, bool vToggled)
	{
		bool pressed = false;

		if (vToggled)
		{
			ImVec4 bua = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			ImVec4 te = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			ImGui::PushStyleColor(ImGuiCol_Button, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, te);
			ImGui::PushStyleColor(ImGuiCol_Text, bua);
		}

		pressed = IMGUI_BUTTON(vLabel);

		if (vToggled)
		{
			ImGui::PopStyleColor(4); //-V112
		}

		return pressed;
	}
#define IMGUI_RADIO_BUTTON inRadioButton
#endif // IMGUI_RADIO_BUTTON
#endif  // USE_THUMBNAILS
#ifdef USE_BOOKMARK
#ifndef defaultBookmarkPaneWith
#define defaultBookmarkPaneWith 150.0f
#endif // defaultBookmarkPaneWith
#ifndef bookmarksButtonString
#define bookmarksButtonString "Bookmark"
#endif // bookmarksButtonString
#ifndef bookmarksButtonHelpString
#define bookmarksButtonHelpString "Bookmark"
#endif // bookmarksButtonHelpString
#ifndef addBookmarkButtonString
#define addBookmarkButtonString "+"
#endif // addBookmarkButtonString
#ifndef removeBookmarkButtonString
#define removeBookmarkButtonString "-"
#endif // removeBookmarkButtonString
#ifndef IMGUI_TOGGLE_BUTTON
	inline bool inToggleButton(const char* vLabel, bool* vToggled)
	{
		bool pressed = false;

		if (vToggled && *vToggled)
		{
			ImVec4 bua = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			//ImVec4 buh = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			//ImVec4 bu = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			ImVec4 te = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			ImGui::PushStyleColor(ImGuiCol_Button, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, te);
			ImGui::PushStyleColor(ImGuiCol_Text, bua);
		}

		pressed = IMGUI_BUTTON(vLabel);

		if (vToggled && *vToggled)
		{
			ImGui::PopStyleColor(4); //-V112
		}

		if (vToggled && pressed)
			*vToggled = !*vToggled;

		return pressed;
	}
#define IMGUI_TOGGLE_BUTTON inToggleButton
#endif // IMGUI_TOGGLE_BUTTON
#endif // USE_BOOKMARK

	/////////////////////////////////////////////////////////////////////////////////////
	//// INLINE FUNCTIONS ///////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

#ifndef USE_STD_FILESYSTEM
	inline int inAlphaSort(const struct dirent** a, const struct dirent** b)
	{
		return strcoll((*a)->d_name, (*b)->d_name);
	}
#endif

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILE EXTENTIONS INFOS //////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	IGFD::FileStyle::FileStyle() 
		: color(0, 0, 0, 0)
	{ 

	}

	IGFD::FileStyle::FileStyle(const FileStyle& vStyle)
	{
		color = vStyle.color;
		icon = vStyle.icon;
		font = vStyle.font;
		flags = vStyle.flags;
	}

	IGFD::FileStyle::FileStyle(const ImVec4& vColor, const std::string& vIcon, ImFont* vFont) 
		: color(vColor), icon(vIcon), font(vFont)
	{ 

	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILE INFOS /////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	// https://github.com/ocornut/imgui/issues/1720
	bool IGFD::Utils::Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 1.0f);
	}

	bool IGFD::Utils::WReplaceString(std::wstring& str, const std::wstring& oldStr, const std::wstring& newStr)
	{
		bool found = false;
#ifdef _IGFD_WIN_
		size_t pos = 0;
		while ((pos = str.find(oldStr, pos)) != std::wstring::npos)
		{
			found = true;
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
#endif // _IGFD_WIN_
		return found;
	}

	std::vector<std::wstring> IGFD::Utils::WSplitStringToVector(const std::wstring& text, char delimiter, bool pushEmpty)
	{
		std::vector<std::wstring> arr;
#ifdef _IGFD_WIN_
		if (!text.empty())
		{
			std::wstring::size_type start = 0;
			std::wstring::size_type end = text.find(delimiter, start);
			while (end != std::wstring::npos)
			{
				std::wstring token = text.substr(start, end - start);
				if (!token.empty() || (token.empty() && pushEmpty)) //-V728
					arr.push_back(token);
				start = end + 1;
				end = text.find(delimiter, start);
			}
			std::wstring token = text.substr(start);
			if (!token.empty() || (token.empty() && pushEmpty)) //-V728
				arr.push_back(token);
		}
#endif // _IGFD_WIN_
		return arr;
	}

	// Convert a wide Unicode string to an UTF8 string
	std::string IGFD::Utils::utf8_encode(const std::wstring &wstr)
	{
		std::string res;
#ifdef _IGFD_WIN_
		if(!wstr.empty())
		{
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0],
				(int)wstr.size(), NULL, 0, NULL, NULL);
			if (size_needed)
			{
				res = std::string(size_needed, 0);
				WideCharToMultiByte (CP_UTF8, 0, &wstr[0],
					(int)wstr.size(), &res[0], size_needed, NULL, NULL);
			}
		}
#endif // _IGFD_WIN_
		return res;
	}

	// Convert an UTF8 string to a wide Unicode String
	std::wstring IGFD::Utils::utf8_decode(const std::string &str)
	{
		std::wstring res;
#ifdef _IGFD_WIN_
		if( !str.empty())
		{
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0],
				(int)str.size(), NULL, 0);
			if (size_needed)
			{
				res = std::wstring(size_needed, 0);
				MultiByteToWideChar(CP_UTF8, 0, &str[0],
					(int)str.size(), &res[0], size_needed);
			}
		}
#endif // _IGFD_WIN_
		return res;
	}

	bool IGFD::Utils::ReplaceString(std::string& str, const std::string& oldStr, const std::string& newStr)
	{
		bool found = false;
		size_t pos = 0;
		while ((pos = str.find(oldStr, pos)) != std::string::npos)
		{
			found = true;
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
		return found;
	}

	std::vector<std::string> IGFD::Utils::SplitStringToVector(const std::string& text, char delimiter, bool pushEmpty)
	{
		std::vector<std::string> arr;
		if (!text.empty())
		{
			size_t start = 0;
			size_t end = text.find(delimiter, start);
			while (end != std::string::npos)
			{
				auto token = text.substr(start, end - start);
				if (!token.empty() || (token.empty() && pushEmpty)) //-V728
					arr.push_back(token);
				start = end + 1;
				end = text.find(delimiter, start);
			}
			auto token = text.substr(start);
			if (!token.empty() || (token.empty() && pushEmpty)) //-V728
				arr.push_back(token);
		}
		return arr;
	}

	std::vector<std::string> IGFD::Utils::GetDrivesList()
	{
		std::vector<std::string> res;

#ifdef _IGFD_WIN_
		const DWORD mydrives = 2048;
		char lpBuffer[2048];
#define mini(a,b) (((a) < (b)) ? (a) : (b))
		const DWORD countChars = mini(GetLogicalDriveStringsA(mydrives, lpBuffer), 2047);
#undef mini
		if (countChars > 0U && countChars < 2049U)
		{
			std::string var = std::string(lpBuffer, (size_t)countChars);
			IGFD::Utils::ReplaceString(var, "\\", "");
			res = IGFD::Utils::SplitStringToVector(var, '\0', false);
		}
#endif // _IGFD_WIN_

		return res;
	}

	bool IGFD::Utils::IsDirectoryCanBeOpened(const std::string& name)
	{
		bool bExists = false;

		if (!name.empty())
		{
#ifdef USE_STD_FILESYSTEM
			namespace fs = std::filesystem;
#ifdef _IGFD_WIN_
			std::wstring wname = IGFD::Utils::utf8_decode(name.c_str());
			fs::path pathName = fs::path(wname);
#else // _IGFD_WIN_
			fs::path pathName = fs::path(name);
#endif // _IGFD_WIN_
			try
			{
				// interesting, in the case of a protected dir or for any reason the dir cant be opened
				// this func will work but will say nothing more . not like the dirent version
				bExists = fs::is_directory(pathName);
				// test if can be opened, this function can thrown an exception if there is an issue with this dir
				// here, the dir_iter is need else not exception is thrown.. 
				const auto dir_iter = std::filesystem::directory_iterator(pathName);
				(void)dir_iter; // for avoid unused warnings
			}
			catch (std::exception /*ex*/)
			{
				// fail so this dir cant be opened
				bExists = false;
			}
#else
			DIR* pDir = nullptr;
			// interesting, in the case of a protected dir or for any reason the dir cant be opened
			// this func will fail
			pDir = opendir(name.c_str());
			if (pDir != nullptr)
			{
				bExists = true;
				(void)closedir(pDir);
			}
#endif // USE_STD_FILESYSTEM
		}

		return bExists;    // this is not a directory!
	}

	bool IGFD::Utils::IsDirectoryExist(const std::string& name)
	{
		bool bExists = false;

		if (!name.empty())
		{
#ifdef USE_STD_FILESYSTEM
			namespace fs = std::filesystem;
#ifdef _IGFD_WIN_
			std::wstring wname = IGFD::Utils::utf8_decode(name.c_str());
			fs::path pathName = fs::path(wname);
#else // _IGFD_WIN_
			fs::path pathName = fs::path(name);
#endif // _IGFD_WIN_
			bExists = fs::is_directory(pathName);
#else
			DIR* pDir = nullptr;
			pDir = opendir(name.c_str());
			if (pDir)
			{
				bExists = true; 
				closedir(pDir);
			}
			else if (ENOENT == errno) 
			{
				/* Directory does not exist. */
				//bExists = false;
			}
			else 
			{
				/* opendir() failed for some other reason. 
				like if a dir is protected, or not accessable with user right
				*/
				bExists = true;
			}
#endif // USE_STD_FILESYSTEM
		}

		return bExists;    // this is not a directory!
	}

	bool IGFD::Utils::CreateDirectoryIfNotExist(const std::string& name)
	{
		bool res = false;

		if (!name.empty())
		{
			if (!IsDirectoryExist(name))
			{
#ifdef _IGFD_WIN_
#ifdef USE_STD_FILESYSTEM
				namespace fs = std::filesystem;
				std::wstring wname = IGFD::Utils::utf8_decode(name.c_str());
				fs::path pathName = fs::path(wname);
				res = fs::create_directory(pathName);
#else // USE_STD_FILESYSTEM
				std::wstring wname = IGFD::Utils::utf8_decode(name);
				if (CreateDirectoryW(wname.c_str(), nullptr))
				{
					res = true;
				}
#endif // USE_STD_FILESYSTEM
#elif defined(__EMSCRIPTEN__) // _IGFD_WIN_
				std::string str = std::string("FS.mkdir('") + name + "');";
				emscripten_run_script(str.c_str());
				res = true;
#elif defined(_IGFD_UNIX_)
				char buffer[PATH_MAX] = {};
				snprintf(buffer, PATH_MAX, "mkdir -p \"%s\"", name.c_str()); 
				const int dir_err = std::system(buffer);
				if (dir_err != -1)
				{
					res = true;
				}
#endif // _IGFD_WIN_
				if (!res) {
					std::cout << "Error creating directory " << name << std::endl;
				}
			}
		}

		return res;
	}

#ifdef USE_STD_FILESYSTEM
	// https://github.com/aiekick/ImGuiFileDialog/issues/54
	IGFD::Utils::PathStruct IGFD::Utils::ParsePathFileName(const std::string& vPathFileName)
	{
		namespace fs = std::filesystem;
		PathStruct res;
		if (vPathFileName.empty())
			return res;

		auto fsPath = fs::path(vPathFileName);

		if (fs::is_directory(fsPath)) {
			res.name = "";
			res.path = fsPath.string();
			res.isOk = true;

		} else if (fs::is_regular_file(fsPath)) {
			res.name = fsPath.filename().string();
			res.path = fsPath.parent_path().string();
			res.isOk = true;
		}

		return res;
	}
#else
	IGFD::Utils::PathStruct IGFD::Utils::ParsePathFileName(const std::string& vPathFileName)
	{
		PathStruct res;

		if (!vPathFileName.empty())
		{
			std::string pfn = vPathFileName;
			std::string separator(1u, PATH_SEP);
			IGFD::Utils::ReplaceString(pfn, "\\", separator);
			IGFD::Utils::ReplaceString(pfn, "/", separator);

			size_t lastSlash = pfn.find_last_of(separator);
			if (lastSlash != std::string::npos)
			{
				res.name = pfn.substr(lastSlash + 1);
				res.path = pfn.substr(0, lastSlash);
				res.isOk = true;
			}

			size_t lastPoint = pfn.find_last_of('.');
			if (lastPoint != std::string::npos)
			{
				if (!res.isOk)
				{
					res.name = pfn;
					res.isOk = true;
				}
				res.ext = pfn.substr(lastPoint + 1);
				IGFD::Utils::ReplaceString(res.name, "." + res.ext, "");
			}

			if (!res.isOk)
			{
				res.name = std::move(pfn);
				res.isOk = true;
			}
		}

		return res;
	}
#endif // USE_STD_FILESYSTEM

	void IGFD::Utils::AppendToBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr)
	{
		std::string st = vStr;
		size_t len = vBufferLen - 1u;
		size_t slen = strlen(vBuffer);

		if (!st.empty() && st != "\n")
		{
			IGFD::Utils::ReplaceString(st, "\n", "");
			IGFD::Utils::ReplaceString(st, "\r", "");
		}
		vBuffer[slen] = '\0';
		std::string str = std::string(vBuffer);
		//if (!str.empty()) str += "\n";
		str += vStr;
		if (len > str.size()) len = str.size();
#ifdef _MSC_VER
		strncpy_s(vBuffer, vBufferLen, str.c_str(), len);
#else // _MSC_VER
		strncpy(vBuffer, str.c_str(), len);
#endif // _MSC_VER
		vBuffer[len] = '\0';
	}

	void IGFD::Utils::ResetBuffer(char* vBuffer)
	{
		vBuffer[0] = '\0';
	}

	void IGFD::Utils::SetBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr)
	{
		ResetBuffer(vBuffer);
		AppendToBuffer(vBuffer, vBufferLen, vStr);
	}

	std::string IGFD::Utils::LowerCaseString(const std::string& vString)
	{
		auto str = vString;

		// convert to lower case
		for (char& c : str)
			c = (char)std::tolower(c);

		return str;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILE INFOS /////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	bool IGFD::FileInfos::IsTagFound(const std::string& vTag) const
	{
		if (!vTag.empty())
		{
			if (fileNameExt_optimized == "..") return true;

			return
				fileNameExt_optimized.find(vTag) != std::string::npos ||	// first try wihtout case and accents
				fileNameExt.find(vTag) != std::string::npos;				// second if searched with case and accents
		}

		// if tag is empty => its a special case but all is found
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// SEARCH MANAGER /////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	void IGFD::SearchManager::Clear()
	{
		puSearchTag.clear();
		IGFD::Utils::ResetBuffer(puSearchBuffer);
	}

	void IGFD::SearchManager::DrawSearchBar(FileDialogInternal& vFileDialogInternal)
	{
		// search field
		if (IMGUI_BUTTON(resetButtonString "##BtnImGuiFileDialogSearchField"))
		{
			Clear();
			vFileDialogInternal.puFileManager.ApplyFilteringOnFileList(vFileDialogInternal);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonResetSearchString);
		ImGui::SameLine();
		ImGui::Text(searchString);
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		bool edited = ImGui::InputText("##InputImGuiFileDialogSearchField", puSearchBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
		if (ImGui::GetItemID() == ImGui::GetActiveID())
			puSearchInputIsActive = true;
		ImGui::PopItemWidth();
		if (edited)
		{
			puSearchTag = puSearchBuffer;
			vFileDialogInternal.puFileManager.ApplyFilteringOnFileList(vFileDialogInternal);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILTER INFOS ///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	void IGFD::FilterManager::FilterInfos::clear()
	{
		filter.clear();
		filter_regex = std::regex();
		collectionfilters.clear();
		filter_optimized.clear();
		collectionfilters_optimized.clear();
		collectionfilters_regex.clear();
	}

	bool IGFD::FilterManager::FilterInfos::empty() const
	{
		return filter.empty() && collectionfilters.empty();
	}

	bool IGFD::FilterManager::FilterInfos::exist(const std::string& vFilter, bool vIsCaseInsensitive) const
	{
		if (vIsCaseInsensitive)
		{
			auto _filter = Utils::LowerCaseString(vFilter);
			return
				filter_optimized == _filter ||
				(collectionfilters_optimized.find(_filter) != collectionfilters_optimized.end());
		}
		return 
			filter == vFilter || 
			(collectionfilters.find(vFilter) != collectionfilters.end());
	}

	bool IGFD::FilterManager::FilterInfos::regex_exist(const std::string& vFilter) const
	{
		if (std::regex_search(vFilter, filter_regex))
		{
			return true;
		}
		else
		{
			for (auto regex : collectionfilters_regex)
			{
				if (std::regex_search(vFilter, regex))
				{
					return true;
				}
			}
		}

		return false;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//// FILTER MANAGER /////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	void IGFD::FilterManager::ParseFilters(const char* vFilters)
	{
		prParsedFilters.clear();

		if (vFilters)
			puDLGFilters = vFilters;			// file mode
		else
			puDLGFilters.clear();				// directory mode

		if (!puDLGFilters.empty())
		{
			// ".*,.cpp,.h,.hpp" => simple filters
			// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters
			// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex
			// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex

			bool currentFilterFound = false;

			size_t nan = std::string::npos;
			size_t p = 0, lp = 0;
			while ((p = puDLGFilters.find_first_of("{,", p)) != nan)
			{
				FilterInfos infos;

				if (puDLGFilters[p] == '{') // {
				{
					infos.filter = puDLGFilters.substr(lp, p - lp);
					infos.filter_optimized = Utils::LowerCaseString(infos.filter);
					p++;
					lp = puDLGFilters.find('}', p);
					if (lp != nan)
					{
						std::string fs = puDLGFilters.substr(p, lp - p);
						auto arr = IGFD::Utils::SplitStringToVector(fs, ',', false);
						for (auto a : arr)
						{
							infos.collectionfilters.emplace(a);
							infos.collectionfilters_optimized.emplace(Utils::LowerCaseString(a));

							// a regex
							if (a.find('(') != std::string::npos) {
								if (a.find(')') != std::string::npos) {
									infos.collectionfilters_regex.push_back(std::regex(a));
								}
							}
						}
					}
					p = lp + 1;
				}
				else // ,
				{
					infos.filter = puDLGFilters.substr(lp, p - lp);
					infos.filter_optimized = Utils::LowerCaseString(infos.filter);

					// a regex
					if (infos.filter.find('(') != std::string::npos) {
						if (infos.filter.find(')') != std::string::npos) {
							infos.filter_regex = std::regex(infos.filter);
						}
					}

					p++;
				}

				if (!currentFilterFound && prSelectedFilter.filter == infos.filter)
				{
					currentFilterFound = true;
					prSelectedFilter = infos;
				}

				lp = p;
				if (!infos.empty())
					prParsedFilters.emplace_back(infos);
			}

			std::string token = puDLGFilters.substr(lp);
			if (!token.empty())
			{
				FilterInfos infos;
				infos.filter = std::move(token);
				prParsedFilters.emplace_back(infos);
			}

			if (!currentFilterFound)
				if (!prParsedFilters.empty())
					prSelectedFilter = *prParsedFilters.begin();
		}
	}

	void IGFD::FilterManager::SetSelectedFilterWithExt(const std::string& vFilter)
	{
		if (!prParsedFilters.empty())
		{
			if (!vFilter.empty())
			{
				// std::map<std::string, FilterInfos>
				for (const auto& infos : prParsedFilters)
				{
					if (vFilter == infos.filter)
					{
						prSelectedFilter = infos;
					}
					else
					{
						// maybe this ext is in an extention so we will 
						// explore the collections is they are existing
						for (const auto& filter : infos.collectionfilters)
						{
							if (vFilter == filter)
							{
								prSelectedFilter = infos;
							}
						}
					}
				}
			}

			if (prSelectedFilter.empty())
				prSelectedFilter = *prParsedFilters.begin();
		}
	}

	void IGFD::FilterManager::SetFileStyle(const IGFD_FileStyleFlags& vFlags, const char* vCriteria, const FileStyle& vInfos)
	{
		std::string _criteria;
		if (vCriteria)
			_criteria = std::string(vCriteria);
		prFilesStyle[vFlags][_criteria] = std::make_shared<FileStyle>(vInfos);
		prFilesStyle[vFlags][_criteria]->flags = vFlags;
	}

	// will be called internally 
	// will not been exposed to IGFD API
	bool IGFD::FilterManager::prFillFileStyle(std::shared_ptr<FileInfos> vFileInfos) const
	{
		// todo : better system to found regarding what style to priorize regarding other
		// maybe with a lambda fucntion for let the user use his style
		// according to his use case
		if (vFileInfos.use_count() && !prFilesStyle.empty())
		{
			for (const auto& _flag : prFilesStyle)
			{
				for (const auto& _file : _flag.second)
				{
					if ((_flag.first & IGFD_FileStyleByTypeDir && _flag.first & IGFD_FileStyleByTypeLink && vFileInfos->fileType.isDir() && vFileInfos->fileType.isSymLink()) ||
						(_flag.first & IGFD_FileStyleByTypeFile && _flag.first & IGFD_FileStyleByTypeLink && vFileInfos->fileType.isFile() && vFileInfos->fileType.isSymLink()) ||
						(_flag.first & IGFD_FileStyleByTypeLink && vFileInfos->fileType.isSymLink()) ||
						(_flag.first & IGFD_FileStyleByTypeDir && vFileInfos->fileType.isDir()) ||
						(_flag.first & IGFD_FileStyleByTypeFile && vFileInfos->fileType.isFile()))
					{
						if (_file.first.empty()) // for all links
						{
							vFileInfos->fileStyle = _file.second;
						}
						else if (_file.first.find('(') != std::string::npos &&
							std::regex_search(vFileInfos->fileNameExt, std::regex(_file.first)))  // for links who are equal to style criteria
						{
							vFileInfos->fileStyle = _file.second;
						}
						else if (_file.first == vFileInfos->fileNameExt)  // for links who are equal to style criteria
						{
							vFileInfos->fileStyle = _file.second;
						}
					}			

					if (_flag.first & IGFD_FileStyleByExtention)
					{
						if (_file.first.find('(') != std::string::npos &&
							std::regex_search(vFileInfos->fileExt, std::regex(_file.first)))
						{
							vFileInfos->fileStyle = _file.second;
						}
						else if (_file.first == vFileInfos->fileExt)
						{
							vFileInfos->fileStyle = _file.second;
						}
					}

					if (_flag.first & IGFD_FileStyleByFullName)
					{
						if (_file.first.find('(') != std::string::npos &&
							std::regex_search(vFileInfos->fileNameExt, std::regex(_file.first)))
						{
							vFileInfos->fileStyle = _file.second;
						}
						else if (_file.first == vFileInfos->fileNameExt)
						{
							vFileInfos->fileStyle = _file.second;
						}
					}

					if (_flag.first & IGFD_FileStyleByContainedInFullName)
					{
						if (_file.first.find('(') != std::string::npos &&
							std::regex_search(vFileInfos->fileNameExt, std::regex(_file.first)))
						{
							vFileInfos->fileStyle = _file.second;
						}
						else if (vFileInfos->fileNameExt.find(_file.first) != std::string::npos)
						{
							vFileInfos->fileStyle = _file.second;
						}
					}

					if (vFileInfos->fileStyle.use_count())
						return true;
				}
			}
		}

		return false;
	}

	void IGFD::FilterManager::SetFileStyle(const IGFD_FileStyleFlags& vFlags, const char* vCriteria, const ImVec4& vColor, const std::string& vIcon, ImFont* vFont)
	{
		std::string _criteria;
		if (vCriteria)
			_criteria = std::string(vCriteria);
		prFilesStyle[vFlags][_criteria] = std::make_shared<FileStyle>(vColor, vIcon, vFont);
		prFilesStyle[vFlags][_criteria]->flags = vFlags;
	}

	// todo : to refactor this fucking function
	bool IGFD::FilterManager::GetFileStyle(const IGFD_FileStyleFlags& vFlags, const std::string& vCriteria, ImVec4* vOutColor, std::string* vOutIcon, ImFont **vOutFont)
	{
		if (vOutColor)
		{
			if (!prFilesStyle.empty())
			{
				if (prFilesStyle.find(vFlags) != prFilesStyle.end()) // found
				{
					if (vFlags & IGFD_FileStyleByContainedInFullName)
					{
						// search for vCriteria who are containing the criteria
						for (const auto& _file : prFilesStyle.at(vFlags))
						{
							if (vCriteria.find(_file.first) != std::string::npos)
							{
								if (_file.second.use_count())
								{
									*vOutColor = _file.second->color;
									if (vOutIcon)
										*vOutIcon = _file.second->icon;
									if (vOutFont)
										*vOutFont = _file.second->font;
									return true;
								}
							}
						}
					}
					else
					{
						if (prFilesStyle.at(vFlags).find(vCriteria) != prFilesStyle.at(vFlags).end()) // found
						{
							*vOutColor = prFilesStyle[vFlags][vCriteria]->color;
							if (vOutIcon)
								*vOutIcon = prFilesStyle[vFlags][vCriteria]->icon;
							if (vOutFont)
								*vOutFont = prFilesStyle[vFlags][vCriteria]->font;
							return true;
						}
					}
				}
				else
				{
					// search for flag composition
					for (const auto& _flag : prFilesStyle)
					{
						if (_flag.first & vFlags)
						{
							if (_flag.first & IGFD_FileStyleByContainedInFullName)
							{
								// search for vCriteria who are containing the criteria
								for (const auto& _file : prFilesStyle.at(_flag.first))
								{
									if (vCriteria.find(_file.first) != std::string::npos)
									{
										if (_file.second.use_count())
										{
											*vOutColor = _file.second->color;
											if (vOutIcon)
												*vOutIcon = _file.second->icon;
											if (vOutFont)
												*vOutFont = _file.second->font;
											return true;
										}
									}
								}
							}
							else
							{
								if (prFilesStyle.at(_flag.first).find(vCriteria) != prFilesStyle.at(_flag.first).end()) // found
								{
									*vOutColor = prFilesStyle[_flag.first][vCriteria]->color;
									if (vOutIcon)
										*vOutIcon = prFilesStyle[_flag.first][vCriteria]->icon;
									if (vOutFont)
										*vOutFont = prFilesStyle[_flag.first][vCriteria]->font;
									return true;
								}
							}
						}
					}
				}
			}
		}
		return false;
	}

	void IGFD::FilterManager::ClearFilesStyle()
	{
		prFilesStyle.clear();
	}

	bool IGFD::FilterManager::IsCoveredByFilters(const std::string& vNameExt, const std::string& vExt, bool vIsCaseInsensitive) const
	{
		if (!puDLGFilters.empty() && !prSelectedFilter.empty())
		{
			// check if current file extention is covered by current filter
			// we do that here, for avoid doing that during filelist display
			// for better fps
			return (
				prSelectedFilter.exist(vExt, vIsCaseInsensitive) ||
				prSelectedFilter.exist(".*", vIsCaseInsensitive) ||
				prSelectedFilter.exist("*.*", vIsCaseInsensitive) ||
				prSelectedFilter.filter == ".*" ||
				prSelectedFilter.regex_exist(vNameExt));
		}

		return false;
	}

	bool IGFD::FilterManager::DrawFilterComboBox(FileDialogInternal& vFileDialogInternal)
	{
		// combobox of filters
		if (!puDLGFilters.empty())
		{
			ImGui::SameLine();

			bool needToApllyNewFilter = false;

			ImGui::PushItemWidth(FILTER_COMBO_WIDTH);
			if (ImGui::BeginCombo("##Filters", prSelectedFilter.filter.c_str(), ImGuiComboFlags_None))
			{
				intptr_t i = 0;
				for (const auto& filter : prParsedFilters)
				{
					const bool item_selected = (filter.filter == prSelectedFilter.filter);
					ImGui::PushID((void*)(intptr_t)i++);
					if (ImGui::Selectable(filter.filter.c_str(), item_selected))
					{
						prSelectedFilter = filter;
						needToApllyNewFilter = true;
					}
					ImGui::PopID();
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			if (needToApllyNewFilter)
			{
				vFileDialogInternal.puFileManager.OpenCurrentPath(vFileDialogInternal);
			}

			return needToApllyNewFilter;
		}

		return false;
	}

	IGFD::FilterManager::FilterInfos IGFD::FilterManager::GetSelectedFilter()
	{
		return prSelectedFilter;
	}

	std::string IGFD::FilterManager::ReplaceExtentionWithCurrentFilter(const std::string& vFile) const
	{
		auto result = vFile;

		if (!result.empty())
		{
			// if not a collection we can replace the filter by the extention we want
			if (prSelectedFilter.collectionfilters.empty() && 
				prSelectedFilter.filter != ".*" &&
				prSelectedFilter.filter != "*.*")
			{
				size_t lastPoint = vFile.find_last_of('.');
				if (lastPoint != std::string::npos)
				{
					result = result.substr(0, lastPoint);
				}

				result += prSelectedFilter.filter;
			}
		}

		return result;
	}

	void IGFD::FilterManager::SetDefaultFilterIfNotDefined()
	{
		if (prSelectedFilter.empty() && // no filter selected
			!prParsedFilters.empty()) // filter exist
			prSelectedFilter = *prParsedFilters.begin(); // we take the first filter
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILE MANAGER ///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	IGFD::FileManager::FileManager()
	{
		puFsRoot = std::string(1u, PATH_SEP);
	}

	void IGFD::FileManager::OpenCurrentPath(const FileDialogInternal& vFileDialogInternal)
	{
		puShowDrives = false;
		ClearComposer();
		ClearFileLists();
		if (puDLGDirectoryMode) // directory mode
			SetDefaultFileName(".");
		else
			SetDefaultFileName(puDLGDefaultFileName);
		ScanDir(vFileDialogInternal, GetCurrentPath());
	}

	void IGFD::FileManager::SortFields(const FileDialogInternal& vFileDialogInternal)
	{
		SortFields(vFileDialogInternal, prFileList, prFilteredFileList);
	}

	void IGFD::FileManager::SortFields(const FileDialogInternal& vFileDialogInternal, 
		std::vector<std::shared_ptr<FileInfos>>& vFileInfosList,
		std::vector<std::shared_ptr<FileInfos>>& vFileInfosFilteredList)
	{
		if (puSortingField != SortingFieldEnum::FIELD_NONE)
		{
			puHeaderFileName = tableHeaderFileNameString;
			puHeaderFileType = tableHeaderFileTypeString;
			puHeaderFileSize = tableHeaderFileSizeString;
			puHeaderFileDate = tableHeaderFileDateString;
#ifdef USE_THUMBNAILS
			puHeaderFileThumbnails = tableHeaderFileThumbnailsString;
#endif // #ifdef USE_THUMBNAILS
		}

		if (puSortingField == SortingFieldEnum::FIELD_FILENAME)
		{
			if (puSortingDirection[0])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileName = tableHeaderAscendingIcon + puHeaderFileName;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						// this code fail in c:\\Users with the link "All users". got a invalid comparator
						/*
						// use code from https://github.com/jackm97/ImGuiFileDialog/commit/bf40515f5a1de3043e60562dc1a494ee7ecd3571
						// strict ordering for file/directory types beginning in '.'
						// common on _IGFD_WIN_ platforms
						if (a->fileNameExt[0] == '.' && b->fileNameExt[0] != '.')
						return false;
						if (a->fileNameExt[0] != '.' && b->fileNameExt[0] == '.')
						return true;
						if (a->fileNameExt[0] == '.' && b->fileNameExt[0] == '.')
						{
						return (stricmp(a->fileNameExt.c_str(), b->fileNameExt.c_str()) < 0); // sort in insensitive case
						}
						*/
						if (a->fileType != b->fileType) return (a->fileType < b->fileType); // directories first
						return (stricmp(a->fileNameExt.c_str(), b->fileNameExt.c_str()) < 0); // sort in insensitive case
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileName = tableHeaderDescendingIcon + puHeaderFileName;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						// this code fail in c:\\Users with the link "All users". got a invalid comparator
						/*
						// use code from https://github.com/jackm97/ImGuiFileDialog/commit/bf40515f5a1de3043e60562dc1a494ee7ecd3571
						// strict ordering for file/directory types beginning in '.'
						// common on _IGFD_WIN_ platforms
						if (a->fileNameExt[0] == '.' && b->fileNameExt[0] != '.')
						return false;
						if (a->fileNameExt[0] != '.' && b->fileNameExt[0] == '.')
						return true;
						if (a->fileNameExt[0] == '.' && b->fileNameExt[0] == '.')
						{
						return (stricmp(a->fileNameExt.c_str(), b->fileNameExt.c_str()) > 0); // sort in insensitive case
						}
						*/
						if (a->fileType != b->fileType) return (a->fileType > b->fileType); // directories last
						return (stricmp(a->fileNameExt.c_str(), b->fileNameExt.c_str()) > 0); // sort in insensitive case
					});
			}
		}
		else if (puSortingField == SortingFieldEnum::FIELD_TYPE)
		{
			if (puSortingDirection[1])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileType = tableHeaderAscendingIcon + puHeaderFileType;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType < b->fileType); // directory in first
						return (a->fileExt < b->fileExt); // else
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileType = tableHeaderDescendingIcon + puHeaderFileType;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType > b->fileType); // directory in last
						return (a->fileExt > b->fileExt); // else
					});
			}
		}
		else if (puSortingField == SortingFieldEnum::FIELD_SIZE)
		{
			if (puSortingDirection[2])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileSize = tableHeaderAscendingIcon + puHeaderFileSize;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType < b->fileType); // directory in first
						return (a->fileSize < b->fileSize); // else
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileSize = tableHeaderDescendingIcon + puHeaderFileSize;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType > b->fileType); // directory in last
						return (a->fileSize > b->fileSize); // else
					});
			}
		}
		else if (puSortingField == SortingFieldEnum::FIELD_DATE)
		{
			if (puSortingDirection[3])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileDate = tableHeaderAscendingIcon + puHeaderFileDate;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType < b->fileType); // directory in first
						return (a->fileModifDate < b->fileModifDate); // else
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileDate = tableHeaderDescendingIcon + puHeaderFileDate;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType > b->fileType); // directory in last
						return (a->fileModifDate > b->fileModifDate); // else
					});
			}
		}
#ifdef USE_THUMBNAILS
		else if (puSortingField == SortingFieldEnum::FIELD_THUMBNAILS)
		{
			// we will compare thumbnails by :
			// 1) width 
			// 2) height

			if (puSortingDirection[4])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileThumbnails = tableHeaderAscendingIcon + puHeaderFileThumbnails;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (a->fileType.isDir()); // directory in first
						if (a->thumbnailInfo.textureWidth == b->thumbnailInfo.textureWidth)
							return (a->thumbnailInfo.textureHeight < b->thumbnailInfo.textureHeight);
						return (a->thumbnailInfo.textureWidth < b->thumbnailInfo.textureWidth);
					});
			}

			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				puHeaderFileThumbnails = tableHeaderDescendingIcon + puHeaderFileThumbnails;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(vFileInfosList.begin(), vFileInfosList.end(),
					[](const std::shared_ptr<FileInfos>& a, const std::shared_ptr<FileInfos>& b) -> bool
					{
						if (!a.use_count() || !b.use_count())
							return false;

						if (a->fileType != b->fileType) return (!a->fileType.isDir()); // directory in last
						if (a->thumbnailInfo.textureWidth == b->thumbnailInfo.textureWidth)
							return (a->thumbnailInfo.textureHeight > b->thumbnailInfo.textureHeight);
						return (a->thumbnailInfo.textureWidth > b->thumbnailInfo.textureWidth);
					});
			}
		}
#endif // USE_THUMBNAILS

		ApplyFilteringOnFileList(vFileDialogInternal, vFileInfosList, vFileInfosFilteredList);
	}

	void IGFD::FileManager::ClearFileLists()
	{
		prFilteredFileList.clear();
		prFileList.clear();
	}

	void IGFD::FileManager::ClearPathLists()
	{
		prFilteredPathList.clear();
		prPathList.clear();
	}

	void IGFD::FileManager::AddFile(const FileDialogInternal& vFileDialogInternal, const std::string& vPath, const std::string& vFileName, const FileType& vFileType)
	{
		auto infos = std::make_shared<FileInfos>();

		infos->filePath = vPath;
		infos->fileNameExt = vFileName;
		infos->fileNameExt_optimized = Utils::LowerCaseString(infos->fileNameExt);
		infos->fileType = vFileType;

		if (infos->fileNameExt.empty() || (infos->fileNameExt == "." && !vFileDialogInternal.puFilterManager.puDLGFilters.empty())) return; // filename empty or filename is the current dir '.' //-V807
		if (infos->fileNameExt != ".." && (vFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DontShowHiddenFiles) && infos->fileNameExt[0] == '.') // dont show hidden files
			if (!vFileDialogInternal.puFilterManager.puDLGFilters.empty() || (vFileDialogInternal.puFilterManager.puDLGFilters.empty() && infos->fileNameExt != ".")) // except "." if in directory mode //-V728
				return;

		if (infos->fileType.isFile()
			|| infos->fileType.isLinkToUnknown()) // link can have the same extention of a file
		{
			size_t lpt = infos->fileNameExt.find_last_of('.');
			if (lpt != std::string::npos)
			{
				infos->fileExt = infos->fileNameExt.substr(lpt);
			}

			if (!vFileDialogInternal.puFilterManager.IsCoveredByFilters(infos->fileNameExt, infos->fileExt,
				(vFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_CaseInsensitiveExtention) != 0))
			{
				return;
			}
		}

		vFileDialogInternal.puFilterManager.prFillFileStyle(infos);

		prCompleteFileInfos(infos);
		prFileList.push_back(infos);
	}

	void IGFD::FileManager::AddPath(const FileDialogInternal& vFileDialogInternal, const std::string& vPath, const std::string& vFileName, const FileType& vFileType)
	{
		if (!vFileType.isDir())
			return;

		auto infos = std::make_shared<FileInfos>();

		infos->filePath = vPath;
		infos->fileNameExt = vFileName;
		infos->fileNameExt_optimized = Utils::LowerCaseString(infos->fileNameExt);
		infos->fileType = vFileType;

		if (infos->fileNameExt.empty() || (infos->fileNameExt == "." && !vFileDialogInternal.puFilterManager.puDLGFilters.empty())) return; // filename empty or filename is the current dir '.' //-V807
		if (infos->fileNameExt != ".." && (vFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DontShowHiddenFiles) && infos->fileNameExt[0] == '.') // dont show hidden files
			if (!vFileDialogInternal.puFilterManager.puDLGFilters.empty() || (vFileDialogInternal.puFilterManager.puDLGFilters.empty() && infos->fileNameExt != ".")) // except "." if in directory mode //-V728
				return;

		if (infos->fileType.isFile() 
			|| infos->fileType.isLinkToUnknown()) // link can have the same extention of a file
		{
			size_t lpt = infos->fileNameExt.find_last_of('.');
			if (lpt != std::string::npos)
			{
				infos->fileExt = infos->fileNameExt.substr(lpt);
			}

			if (!vFileDialogInternal.puFilterManager.IsCoveredByFilters(infos->fileNameExt, infos->fileExt,
				(vFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_CaseInsensitiveExtention) != 0))
			{
				return;
			}
		}

		vFileDialogInternal.puFilterManager.prFillFileStyle(infos);

		prCompleteFileInfos(infos);
		prPathList.push_back(infos);
	}

	void IGFD::FileManager::ScanDir(const FileDialogInternal& vFileDialogInternal, const std::string& vPath)
	{
		std::string	path = vPath;

		if (prCurrentPathDecomposition.empty())
		{
			SetCurrentDir(path);
		}

		if (!prCurrentPathDecomposition.empty())
		{
#ifdef _IGFD_WIN_
			if (path == puFsRoot)
				path += std::string(1u, PATH_SEP);
#endif // _IGFD_WIN_

			ClearFileLists();

#ifdef USE_STD_FILESYSTEM
			const std::filesystem::path fspath(path);
			const auto dir_iter = std::filesystem::directory_iterator(fspath);
			FileType fstype = FileType(FileType::ContentType::Directory, std::filesystem::is_symlink(std::filesystem::status(fspath)));
			AddFile(vFileDialogInternal, path, "..", fstype);
			for (const auto& file : dir_iter)
			{
				FileType fileType;
				if (file.is_symlink())
				{
					fileType.SetSymLink(file.is_symlink());
					fileType.SetContent(FileType::ContentType::LinkToUnknown);
				}

				if (file.is_directory()) { fileType.SetContent(FileType::ContentType::Directory); } // directory or symlink to directory
				else if (file.is_regular_file()) { fileType.SetContent(FileType::ContentType::File); }

				if (fileType.isValid())
				{
					auto fileNameExt = file.path().filename().string();
					AddFile(vFileDialogInternal, path, fileNameExt, fileType);
				}
			}
#else // dirent
			struct dirent** files = nullptr;
			size_t n = scandir(path.c_str(), &files, nullptr, inAlphaSort);
			if (n && files)
			{
				size_t i;

				for (i = 0; i < n; i++)
				{
					struct dirent* ent = files[i];

					FileType fileType;
					switch (ent->d_type)
					{
					case DT_DIR:
						fileType.SetContent(FileType::ContentType::Directory); break;
					case DT_REG:
						fileType.SetContent(FileType::ContentType::File); break;
					case DT_LNK:
					{
						fileType.SetSymLink(true);
						fileType.SetContent(FileType::ContentType::LinkToUnknown); // by default if we can't figure out the target type.
						struct stat statInfos = {};
						int result = stat((path + PATH_SEP + ent->d_name).c_str(), &statInfos);
						if (result == 0)
						{
							if (statInfos.st_mode & S_IFREG)
							{
								fileType.SetContent(FileType::ContentType::File);
							}
							else if (statInfos.st_mode & S_IFDIR)
							{
								fileType.SetContent(FileType::ContentType::Directory);
							}
						}
						break;
					}
					default:
						break; // leave it invalid (devices, etc.)
					}

					if (fileType.isValid())
					{
						auto fileNameExt = ent->d_name;
						AddFile(vFileDialogInternal, path, fileNameExt, fileType);
					}
				}

				for (i = 0; i < n; i++)
				{
					free(files[i]);
				}

				free(files);
			}
#endif // USE_STD_FILESYSTEM

			SortFields(vFileDialogInternal, prFileList, prFilteredFileList);
		}
	}

#if defined(USE_QUICK_PATH_SELECT)
	void IGFD::FileManager::ScanDirForPathSelection(const FileDialogInternal& vFileDialogInternal, const std::string& vPath)
	{
		std::string	path = vPath;

		/*if (prCurrentPathDecomposition.empty())
		{
		SetCurrentDir(path);
		}*/

		if (!path.empty())
		{
#ifdef _IGFD_WIN_
			if (path == puFsRoot)
				path += std::string(1u, PATH_SEP);
#endif // _IGFD_WIN_

			ClearPathLists();

#ifdef USE_STD_FILESYSTEM
			const std::filesystem::path fspath(path);
			const auto dir_iter = std::filesystem::directory_iterator(fspath);
			FileType fstype = FileType(FileType::ContentType::Directory, std::filesystem::is_symlink(std::filesystem::status(fspath)));
			AddPath(vFileDialogInternal, path, "..", fstype);
			for (const auto& file : dir_iter)
			{
				FileType fileType; 
				if (file.is_symlink())
				{
					fileType.SetSymLink(file.is_symlink());
					fileType.SetContent(FileType::ContentType::LinkToUnknown);
				}
				if (file.is_directory())
				{
					fileType.SetContent(FileType::ContentType::Directory);
					auto fileNameExt = file.path().filename().string();
					AddPath(vFileDialogInternal, path, fileNameExt, fileType);
				}
			}
#else // dirent
			struct dirent** files = nullptr;
			size_t n = scandir(path.c_str(), &files, nullptr, inAlphaSort);
			if (n)
			{
				size_t i;

				for (i = 0; i < n; i++)
				{
					struct dirent* ent = files[i];

					if (ent->d_type == DT_DIR)
					{
						auto fileNameExt = ent->d_name;
						AddPath(vFileDialogInternal, path, fileNameExt, FileType(FileType::ContentType::Directory, false));
					}
				}

				for (i = 0; i < n; i++)
				{
					free(files[i]);
				}

				free(files);
			}
#endif // USE_STD_FILESYSTEM

			SortFields(vFileDialogInternal, prPathList, prFilteredPathList);
		}
	}
#endif // USE_QUICK_PATH_SELECT

#if defined(USE_QUICK_PATH_SELECT)
	void IGFD::FileManager::OpenPathPopup(const FileDialogInternal& vFileDialogInternal, std::vector<std::string>::iterator vPathIter)
	{
		const auto path = ComposeNewPath(vPathIter);
		ScanDirForPathSelection(vFileDialogInternal, path);
		prPopupComposedPath = vPathIter;
		ImGui::OpenPopup("IGFD_Path_Popup");
	}
#endif // USE_QUICK_PATH_SELECT

	bool IGFD::FileManager::GetDrives()
	{
		auto drives = IGFD::Utils::GetDrivesList();
		if (!drives.empty())
		{
			prCurrentPath.clear();
			prCurrentPathDecomposition.clear();
			ClearFileLists();
			for (auto& drive : drives)
			{
				auto info = std::make_shared<FileInfos>();
				info->fileNameExt = drive;
				info->fileNameExt_optimized = Utils::LowerCaseString(drive);
				info->fileType.SetContent(FileType::ContentType::Directory);

				if (!info->fileNameExt.empty())
				{
					prFileList.push_back(info);
				}
			}
			puShowDrives = true;
			return true;
		}
		return false;
	}

	bool IGFD::FileManager::IsComposerEmpty()
	{
		return prCurrentPathDecomposition.empty();
	}

	size_t IGFD::FileManager::GetComposerSize()
	{
		return prCurrentPathDecomposition.size();
	}

	bool IGFD::FileManager::IsFileListEmpty()
	{
		return prFileList.empty();
	}

	bool IGFD::FileManager::IsPathListEmpty()
	{
		return prPathList.empty();
	}

	size_t IGFD::FileManager::GetFullFileListSize()
	{
		return prFileList.size();
	}

	std::shared_ptr<FileInfos> IGFD::FileManager::GetFullFileAt(size_t vIdx)
	{
		if (vIdx < prFileList.size())
			return prFileList[vIdx];
		return nullptr;
	}

	bool IGFD::FileManager::IsFilteredListEmpty()
	{
		return prFilteredFileList.empty();
	}

	bool IGFD::FileManager::IsPathFilteredListEmpty()
	{
		return prFilteredPathList.empty();
	}

	size_t IGFD::FileManager::GetFilteredListSize()
	{
		return prFilteredFileList.size();
	}

	size_t IGFD::FileManager::GetPathFilteredListSize()
	{
		return prFilteredPathList.size();
	}

	std::shared_ptr<FileInfos> IGFD::FileManager::GetFilteredFileAt(size_t vIdx)
	{
		if (vIdx < prFilteredFileList.size())
			return prFilteredFileList[vIdx];
		return nullptr;
	}

	std::shared_ptr<FileInfos> IGFD::FileManager::GetFilteredPathAt(size_t vIdx)
	{
		if (vIdx < prFilteredPathList.size())
			return prFilteredPathList[vIdx];
		return nullptr;
	}

	std::vector<std::string>::iterator IGFD::FileManager::GetCurrentPopupComposedPath()
	{
		return prPopupComposedPath;
	}

	bool IGFD::FileManager::IsFileNameSelected(const std::string& vFileName)
	{
		return prSelectedFileNames.find(vFileName) != prSelectedFileNames.end();
	}

	std::string IGFD::FileManager::GetBack()
	{
		return prCurrentPathDecomposition.back();
	}

	void IGFD::FileManager::ClearComposer()
	{
		prCurrentPathDecomposition.clear();
	}

	void IGFD::FileManager::ClearAll()
	{
		ClearComposer();
		ClearFileLists();
		ClearPathLists();
	}
	void IGFD::FileManager::ApplyFilteringOnFileList(const FileDialogInternal& vFileDialogInternal)
	{
		ApplyFilteringOnFileList(vFileDialogInternal, prFileList, prFilteredFileList);
	}

	void IGFD::FileManager::ApplyFilteringOnFileList(
		const FileDialogInternal& vFileDialogInternal,
		std::vector<std::shared_ptr<FileInfos>>& vFileInfosList,
		std::vector<std::shared_ptr<FileInfos>>& vFileInfosFilteredList)
	{
		vFileInfosFilteredList.clear();
		for (const auto& file : vFileInfosList)
		{
			if (!file.use_count())
				continue;
			bool show = true;
			if (!file->IsTagFound(vFileDialogInternal.puSearchManager.puSearchTag))  // if search tag
				show = false;
			if (puDLGDirectoryMode && !file->fileType.isDir())
				show = false;
			if (show)
				vFileInfosFilteredList.push_back(file);
		}
	}

	std::string IGFD::FileManager::prRoundNumber(double vvalue, int n)
	{
		std::stringstream tmp;
		tmp << std::setprecision(n) << std::fixed << vvalue;
		return tmp.str();
	}

	std::string IGFD::FileManager::prFormatFileSize(size_t vByteSize)
	{
		if (vByteSize != 0)
		{
			static double lo = 1024.0;
			static double ko = 1024.0 * 1024.0;
			static double mo = 1024.0 * 1024.0 * 1024.0;

			auto v = (double)vByteSize;

			if (v < lo)
				return prRoundNumber(v, 0) + " " + fileSizeBytes; // octet
			else if (v < ko)
				return prRoundNumber(v / lo, 2) + " " + fileSizeKiloBytes; // ko
			else  if (v < mo)
				return prRoundNumber(v / ko, 2) + " " + fileSizeMegaBytes; // Mo
			else
				return prRoundNumber(v / mo, 2) + " " + fileSizeGigaBytes; // Go
		}

		return "";
	}

	void IGFD::FileManager::prCompleteFileInfos(const std::shared_ptr<FileInfos>& vInfos)
	{
		if (!vInfos.use_count())
			return;

		if (vInfos->fileNameExt != "." &&
			vInfos->fileNameExt != "..")
		{
			// _stat struct :
			//dev_t     st_dev;     /* ID of device containing file */
			//ino_t     st_ino;     /* inode number */
			//mode_t    st_mode;    /* protection */
			//nlink_t   st_nlink;   /* number of hard links */
			//uid_t     st_uid;     /* user ID of owner */
			//gid_t     st_gid;     /* group ID of owner */
			//dev_t     st_rdev;    /* device ID (if special file) */
			//off_t     st_size;    /* total size, in bytes */
			//blksize_t st_blksize; /* blocksize for file system I/O */
			//blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
			//time_t    st_atime;   /* time of last access - not sure out of ntfs */
			//time_t    st_mtime;   /* time of last modification - not sure out of ntfs */
			//time_t    st_ctime;   /* time of last status change - not sure out of ntfs */

			std::string fpn;

			// FIXME: so the condition is always true?
			if (vInfos->fileType.isFile() || vInfos->fileType.isLinkToUnknown() || vInfos->fileType.isDir())
				fpn = vInfos->filePath + std::string(1u, PATH_SEP) + vInfos->fileNameExt;

			struct stat statInfos = {};
			char timebuf[100];
			int result = stat(fpn.c_str(), &statInfos);
			if (!result)
			{
				if (!vInfos->fileType.isDir())
				{
					vInfos->fileSize = (size_t)statInfos.st_size;
					vInfos->formatedFileSize = prFormatFileSize(vInfos->fileSize);
				}

				size_t len = 0;
#ifdef _MSC_VER
				struct tm _tm;
				errno_t err = localtime_s(&_tm, &statInfos.st_mtime);
				if (!err) len = strftime(timebuf, 99, DateTimeFormat, &_tm);
#else // _MSC_VER
				struct tm* _tm = localtime(&statInfos.st_mtime);
				if (_tm) len = strftime(timebuf, 99, DateTimeFormat, _tm);
#endif // _MSC_VER
				if (len)
				{
					vInfos->fileModifDate = std::string(timebuf, len);
				}
			}
		}
	}

	void IGFD::FileManager::prRemoveFileNameInSelection(const std::string& vFileName)
	{
		prSelectedFileNames.erase(vFileName);

		if (prSelectedFileNames.size() == 1)
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vFileName.c_str());
		}
		else
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%zu files Selected", prSelectedFileNames.size());
		}
	}

	void IGFD::FileManager::prAddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName)
	{
		prSelectedFileNames.emplace(vFileName);

		if (prSelectedFileNames.size() == 1)
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vFileName.c_str());
		}
		else
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%zu files Selected", prSelectedFileNames.size());
		}

		if (vSetLastSelectionFileName)
			prLastSelectedFileName = vFileName;
	}

	void IGFD::FileManager::SetCurrentDir(const std::string& vPath)
	{
		std::string path = vPath;
#ifdef _IGFD_WIN_
		if (puFsRoot == path)
			path += std::string(1u, PATH_SEP);
#endif // _IGFD_WIN_

#ifdef USE_STD_FILESYSTEM
		namespace fs = std::filesystem;
		bool dir_opened = fs::is_directory(vPath);
		if (!dir_opened)
		{
			path = ".";
			dir_opened = fs::is_directory(vPath);
		}
		if (dir_opened)
#else
		DIR* dir = opendir(path.c_str());
		if (dir == nullptr)
		{
			path = ".";
			dir = opendir(path.c_str());
		}

		if (dir != nullptr)
#endif // USE_STD_FILESYSTEM
		{
#ifdef _IGFD_WIN_
			DWORD numchar = 0;
			std::wstring wpath = IGFD::Utils::utf8_decode(path);
			numchar = GetFullPathNameW(wpath.c_str(), 0, nullptr, nullptr);
			std::wstring fpath(numchar, 0);
			GetFullPathNameW(wpath.c_str(), numchar, (wchar_t*)fpath.data(), nullptr);
			std::string real_path = IGFD::Utils::utf8_encode(fpath);
			if (real_path.back() == '\0') // for fix issue we can have with std::string concatenation.. if there is a \0 at end
				real_path = real_path.substr(0, real_path.size() - 1U);
			if (!real_path.empty())
#elif defined(_IGFD_UNIX_) // _IGFD_UNIX_ is _IGFD_WIN_ or APPLE
			char real_path[PATH_MAX]; 
			char* numchar = realpath(path.c_str(), real_path);
			if (numchar != nullptr)
#endif // _IGFD_WIN_
			{
				prCurrentPath = std::move(real_path);
				if (prCurrentPath[prCurrentPath.size() - 1] == PATH_SEP)
				{
					prCurrentPath = prCurrentPath.substr(0, prCurrentPath.size() - 1);
				}
				IGFD::Utils::SetBuffer(puInputPathBuffer, MAX_PATH_BUFFER_SIZE, prCurrentPath);
				prCurrentPathDecomposition = IGFD::Utils::SplitStringToVector(prCurrentPath, PATH_SEP, false);
#ifdef _IGFD_UNIX_ // _IGFD_UNIX_ is _IGFD_WIN_ or APPLE
				prCurrentPathDecomposition.insert(prCurrentPathDecomposition.begin(), std::string(1u, PATH_SEP));
#endif // _IGFD_UNIX_
				if (!prCurrentPathDecomposition.empty())
				{
#ifdef _IGFD_WIN_
					puFsRoot = prCurrentPathDecomposition[0];
#endif // _IGFD_WIN_
				}
			}
#ifndef USE_STD_FILESYSTEM
			closedir(dir);
#endif
		}
	}

	bool IGFD::FileManager::CreateDir(const std::string& vPath)
	{
		bool res = false;

		if (!vPath.empty())
		{
			std::string path = prCurrentPath + std::string(1u, PATH_SEP) + vPath;

			res = IGFD::Utils::CreateDirectoryIfNotExist(path);
		}

		return res;
	}

	std::string IGFD::FileManager::ComposeNewPath(std::vector<std::string>::iterator vIter)
	{
		std::string res;

		while (true)
		{
			if (!res.empty())
			{
#ifdef _IGFD_WIN_
				res = *vIter + std::string(1u, PATH_SEP) + res;
#elif defined(_IGFD_UNIX_) // _IGFD_UNIX_ is _IGFD_WIN_ or APPLE
				if (*vIter == puFsRoot)
					res = *vIter + res;
				else
					res = *vIter + PATH_SEP + res;
#endif // _IGFD_WIN_
			}
			else
				res = *vIter;

			if (vIter == prCurrentPathDecomposition.begin())
			{
#ifdef _IGFD_UNIX_ // _IGFD_UNIX_ is _IGFD_WIN_ or APPLE
				if (res[0] != PATH_SEP)
					res = PATH_SEP + res;
#endif // defined(_IGFD_UNIX_)
				break;
			}

			--vIter;
		}

		return res;
	}

	bool IGFD::FileManager::SetPathOnParentDirectoryIfAny()
	{
		if (prCurrentPathDecomposition.size() > 1)
		{
			prCurrentPath = ComposeNewPath(prCurrentPathDecomposition.end() - 2);
			return true;
		}
		return false;
	}

	std::string IGFD::FileManager::GetCurrentPath()
	{
		if (prCurrentPath.empty())
			prCurrentPath = ".";
		return prCurrentPath;
	}

	void IGFD::FileManager::SetCurrentPath(const std::string& vCurrentPath)
	{
		if (vCurrentPath.empty())
			prCurrentPath = ".";
		else
			prCurrentPath = vCurrentPath;
	}

	bool IGFD::FileManager::IsFileExist(const std::string& vFile)
	{
		std::ifstream docFile(vFile, std::ios::in);
		if (docFile.is_open())
		{
			docFile.close();
			return true;
		}
		return false;
	}

	void IGFD::FileManager::SetDefaultFileName(const std::string& vFileName)
	{
		puDLGDefaultFileName = vFileName;
		IGFD::Utils::SetBuffer(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vFileName);
	}

	bool IGFD::FileManager::SelectDirectory(const std::shared_ptr<FileInfos>& vInfos)
	{
		if (!vInfos.use_count())
			return false;

		bool pathClick = false;

		if (vInfos->fileNameExt == "..")
		{
			pathClick = SetPathOnParentDirectoryIfAny();
		}
		else
		{
			std::string newPath;

			if (puShowDrives)
			{
				newPath = vInfos->fileNameExt + std::string(1u, PATH_SEP);
			}
			else
			{
#ifdef __linux__
				if (puFsRoot == prCurrentPath)
					newPath = prCurrentPath + vInfos->fileNameExt;
				else
#endif // __linux__
					newPath = prCurrentPath + std::string(1u, PATH_SEP) + vInfos->fileNameExt;
			}

			if (IGFD::Utils::IsDirectoryCanBeOpened(newPath))
			{

				if (puShowDrives)
				{
					prCurrentPath = vInfos->fileNameExt;
					puFsRoot = prCurrentPath;
				}
				else
				{
					prCurrentPath = newPath; //-V820
				}
				pathClick = true;
			}
		}

		return pathClick;
	}

	void IGFD::FileManager::SelectFileName(const FileDialogInternal& vFileDialogInternal, const std::shared_ptr<FileInfos>& vInfos)
	{
		if (!vInfos.use_count())
			return;

		if (ImGui::GetIO().KeyCtrl)
		{
			if (puDLGcountSelectionMax == 0) // infinite selection
			{
				if (prSelectedFileNames.find(vInfos->fileNameExt) == prSelectedFileNames.end()) // not found +> add
				{
					prAddFileNameInSelection(vInfos->fileNameExt, true);
				}
				else // found +> remove
				{
					prRemoveFileNameInSelection(vInfos->fileNameExt);
				}
			}
			else // selection limited by size
			{
				if (prSelectedFileNames.size() < puDLGcountSelectionMax)
				{
					if (prSelectedFileNames.find(vInfos->fileNameExt) == prSelectedFileNames.end()) // not found +> add
					{
						prAddFileNameInSelection(vInfos->fileNameExt, true);
					}
					else // found +> remove
					{
						prRemoveFileNameInSelection(vInfos->fileNameExt);
					}
				}
			}
		}
		else if (ImGui::GetIO().KeyShift)
		{
			if (puDLGcountSelectionMax != 1)
			{
				prSelectedFileNames.clear();
				// we will iterate filelist and get the last selection after the start selection
				bool startMultiSelection = false;
				std::string fileNameToSelect = vInfos->fileNameExt;
				std::string savedLastSelectedFileName; // for invert selection mode
				for (const auto& file : prFileList)
				{
					if (!file.use_count())
						continue;

					bool canTake = true;
					if (!file->IsTagFound(vFileDialogInternal.puSearchManager.puSearchTag)) canTake = false;
					if (canTake) // if not filtered, we will take files who are filtered by the dialog
					{
						if (file->fileNameExt == prLastSelectedFileName)
						{
							startMultiSelection = true;
							prAddFileNameInSelection(prLastSelectedFileName, false);
						}
						else if (startMultiSelection)
						{
							if (puDLGcountSelectionMax == 0) // infinite selection
							{
								prAddFileNameInSelection(file->fileNameExt, false);
							}
							else // selection limited by size
							{
								if (prSelectedFileNames.size() < puDLGcountSelectionMax)
								{
									prAddFileNameInSelection(file->fileNameExt, false);
								}
								else
								{
									startMultiSelection = false;
									if (!savedLastSelectedFileName.empty())
										prLastSelectedFileName = savedLastSelectedFileName;
									break;
								}
							}
						}

						if (file->fileNameExt == fileNameToSelect)
						{
							if (!startMultiSelection) // we are before the last Selected FileName, so we must inverse
							{
								savedLastSelectedFileName = prLastSelectedFileName;
								prLastSelectedFileName = fileNameToSelect;
								fileNameToSelect = savedLastSelectedFileName;
								startMultiSelection = true;
								prAddFileNameInSelection(prLastSelectedFileName, false);
							}
							else
							{
								startMultiSelection = false;
								if (!savedLastSelectedFileName.empty())
									prLastSelectedFileName = savedLastSelectedFileName;
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			prSelectedFileNames.clear();
			IGFD::Utils::ResetBuffer(puFileNameBuffer);
			prAddFileNameInSelection(vInfos->fileNameExt, true);
		}
	}

	void IGFD::FileManager::DrawDirectoryCreation(const FileDialogInternal& vFileDialogInternal)
	{
		if (vFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableCreateDirectoryButton)
			return;

		if (IMGUI_BUTTON(createDirButtonString))
		{
			if (!prCreateDirectoryMode)
			{
				prCreateDirectoryMode = true;
				IGFD::Utils::ResetBuffer(puDirectoryNameBuffer);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonCreateDirString);

		if (prCreateDirectoryMode)
		{
			ImGui::SameLine();

			ImGui::PushItemWidth(100.0f);
			ImGui::InputText("##DirectoryFileName", puDirectoryNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
			ImGui::PopItemWidth();

			ImGui::SameLine();

			if (IMGUI_BUTTON(okButtonString))
			{
				std::string newDir = std::string(puDirectoryNameBuffer);
				if (CreateDir(newDir))
				{
					SetCurrentPath(prCurrentPath + std::string(1u, PATH_SEP) + newDir);
					OpenCurrentPath(vFileDialogInternal);
				}

				prCreateDirectoryMode = false;
			}

			ImGui::SameLine();

			if (IMGUI_BUTTON(cancelButtonString))
			{
				prCreateDirectoryMode = false;
			}
		}

		ImGui::SameLine();
	}

	void IGFD::FileManager::DrawPathComposer(const FileDialogInternal& vFileDialogInternal)
	{
		if (IMGUI_BUTTON(resetButtonString))
		{
			SetCurrentPath(".");
			OpenCurrentPath(vFileDialogInternal);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonResetPathString);

#ifdef _IGFD_WIN_
		ImGui::SameLine();

		if (IMGUI_BUTTON(drivesButtonString))
		{
			puDrivesClicked = true;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonDriveString);
#endif // _IGFD_WIN_

		ImGui::SameLine();

		if (IMGUI_BUTTON(editPathButtonString))
		{
			puInputPathActivated = !puInputPathActivated;
			if (puInputPathActivated)
			{
				auto endIt = prCurrentPathDecomposition.end();
				prCurrentPath = ComposeNewPath(--endIt);
				IGFD::Utils::SetBuffer(puInputPathBuffer, MAX_PATH_BUFFER_SIZE, prCurrentPath);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonEditPathString);

		ImGui::SameLine();

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

		// show current path
		if (!prCurrentPathDecomposition.empty())
		{
			ImGui::SameLine();

			if (puInputPathActivated)
			{
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::InputText("##pathedition", puInputPathBuffer, MAX_PATH_BUFFER_SIZE);
				ImGui::PopItemWidth();
			}
			else
			{
				int _id = 0;
				for (auto itPathDecomp = prCurrentPathDecomposition.begin();
					itPathDecomp != prCurrentPathDecomposition.end(); ++itPathDecomp)
				{
					if (itPathDecomp != prCurrentPathDecomposition.begin())
					{
#if defined(CUSTOM_PATH_SPACING)
						ImGui::SameLine(0, CUSTOM_PATH_SPACING);
#else
						ImGui::SameLine();
#endif // USE_CUSTOM_PATH_SPACING
#if defined(USE_QUICK_PATH_SELECT)

#if defined(_IGFD_WIN_)
						const char* sep = "\\";
#elif defined(_IGFD_UNIX_)
						const char* sep = "/";
						if (itPathDecomp != prCurrentPathDecomposition.begin() + 1)
#endif
						{
							ImGui::PushID(_id++);
							bool click = IMGUI_PATH_BUTTON(sep);
							ImGui::PopID();

#if defined(CUSTOM_PATH_SPACING)
							ImGui::SameLine(0, CUSTOM_PATH_SPACING);
#else
							ImGui::SameLine();
#endif // USE_CUSTOM_PATH_SPACING

							if (click)
							{
								OpenPathPopup(vFileDialogInternal, itPathDecomp-1);
							}
							else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
							{
								SetCurrentPath(itPathDecomp-1);
								break;
							}
						}
#endif // USE_QUICK_PATH_SELECT
					}

					ImGui::PushID(_id++);
					bool click = IMGUI_PATH_BUTTON((*itPathDecomp).c_str());
					ImGui::PopID();
					if (click)
					{
						prCurrentPath = ComposeNewPath(itPathDecomp);
						puPathClicked = true;
						break;
					}
					else if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) // activate input for path
					{
						SetCurrentPath(itPathDecomp);
						break;
					}
				}
			}
		}
	}

	void IGFD::FileManager::SetCurrentPath(std::vector<std::string>::iterator vPathIter)
	{
		prCurrentPath = ComposeNewPath(vPathIter);
		IGFD::Utils::SetBuffer(puInputPathBuffer, MAX_PATH_BUFFER_SIZE, prCurrentPath);
		puInputPathActivated = true;
	}

	std::string IGFD::FileManager::GetResultingPath()
	{
		std::string path = prCurrentPath;

		if (puDLGDirectoryMode) // if directory mode
		{
			std::string selectedDirectory = puFileNameBuffer;
			if (!selectedDirectory.empty() &&
				selectedDirectory != ".")
				path += std::string(1u, PATH_SEP) + selectedDirectory;
		}

		return path;
	}

	std::string IGFD::FileManager::GetResultingFileName(FileDialogInternal& vFileDialogInternal)
	{
		if (!puDLGDirectoryMode) // if not directory mode
		{
			return vFileDialogInternal.puFilterManager.ReplaceExtentionWithCurrentFilter(std::string(puFileNameBuffer));
		}

		return ""; // directory mode
	}

	std::string IGFD::FileManager::GetResultingFilePathName(FileDialogInternal& vFileDialogInternal)
	{
		std::string result = GetResultingPath();

		std::string filename = GetResultingFileName(vFileDialogInternal);
		if (!filename.empty())
		{
#ifdef _IGFD_UNIX_
			if (puFsRoot != result)
#endif // _IGFD_UNIX_
				result += std::string(1u, PATH_SEP);

			result += filename;
		}

		return result;
	}

	std::map<std::string, std::string> IGFD::FileManager::GetResultingSelection()
	{
		std::map<std::string, std::string> res;

		for (auto& selectedFileName : prSelectedFileNames)
		{
			std::string result = GetResultingPath();

#ifdef _IGFD_UNIX_
			if (puFsRoot != result)
#endif // _IGFD_UNIX_
				result += std::string(1u, PATH_SEP);

			result += selectedFileName;

			res[selectedFileName] = result;
		}

		return res;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILE DIALOG INTERNAL ///////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	void IGFD::FileDialogInternal::NewFrame()
	{
		puCanWeContinue = true;	// reset flag for possibily validate the dialog
		puIsOk = false;				// reset dialog result
		puFileManager.puDrivesClicked = false;
		puFileManager.puPathClicked = false;

		puNeedToExitDialog = false;

#ifdef USE_DIALOG_EXIT_WITH_KEY
		if (ImGui::IsKeyPressed(IGFD_EXIT_KEY))
		{
			// we do that here with the data's defined at the last frame
			// because escape key can quit input activation and at the end of the frame all flag will be false
			// so we will detect nothing
			if (!(puFileManager.puInputPathActivated ||
				puSearchManager.puSearchInputIsActive ||
				puFileInputIsActive ||
				puFileListViewIsActive))
			{
				puNeedToExitDialog = true; // need to quit dialog
			}
		}
		else
#endif
		{
			puSearchManager.puSearchInputIsActive = false;
			puFileInputIsActive = false;
			puFileListViewIsActive = false;
		}
	}

	void IGFD::FileDialogInternal::EndFrame()
	{
		// directory change
		if (puFileManager.puPathClicked)
		{
			puFileManager.OpenCurrentPath(*this);
		}

		if (puFileManager.puDrivesClicked)
		{
			if (puFileManager.GetDrives())
			{
				puFileManager.ApplyFilteringOnFileList(*this);
			}
		}

		if (puFileManager.puInputPathActivated)
		{
			auto gio = ImGui::GetIO();
			if (ImGui::IsKeyReleased(ImGuiKey_Enter))
			{
				puFileManager.SetCurrentPath(std::string(puFileManager.puInputPathBuffer));
				puFileManager.OpenCurrentPath(*this);
				puFileManager.puInputPathActivated = false;
			}
			if (ImGui::IsKeyReleased(ImGuiKey_Escape))
			{
				puFileManager.puInputPathActivated = false;
			}
		}
	}

	void IGFD::FileDialogInternal::ResetForNewDialog()
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// THUMBNAIL FEATURE //////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	IGFD::ThumbnailFeature::ThumbnailFeature()
	{
#ifdef USE_THUMBNAILS
		prDisplayMode = DisplayModeEnum::FILE_LIST;
#endif
	}

	IGFD::ThumbnailFeature::~ThumbnailFeature()	= default;

	void IGFD::ThumbnailFeature::NewThumbnailFrame(FileDialogInternal& vFileDialogInternal)
	{
		(void)vFileDialogInternal;

#ifdef USE_THUMBNAILS
		prStartThumbnailFileDatasExtraction();
#endif
	}

	void IGFD::ThumbnailFeature::EndThumbnailFrame(FileDialogInternal& vFileDialogInternal)
	{
#ifdef USE_THUMBNAILS
		prClearThumbnails(vFileDialogInternal);
#endif
	}

	void IGFD::ThumbnailFeature::QuitThumbnailFrame(FileDialogInternal& vFileDialogInternal)
	{
#ifdef USE_THUMBNAILS
		prStopThumbnailFileDatasExtraction();
		prClearThumbnails(vFileDialogInternal);
#endif
	}

#ifdef USE_THUMBNAILS
	void IGFD::ThumbnailFeature::prStartThumbnailFileDatasExtraction()
	{
		const bool res = prThumbnailGenerationThread.use_count() && prThumbnailGenerationThread->joinable();
		if (!res)
		{
			prIsWorking = true;
			prCountFiles = 0U;
			prThumbnailGenerationThread = std::shared_ptr<std::thread>(
				new std::thread(&IGFD::ThumbnailFeature::prThreadThumbnailFileDatasExtractionFunc, this),
				[this](std::thread* obj)
				{
					prIsWorking = false;
					if (obj)
						obj->join();
				});
		}
	}

	bool IGFD::ThumbnailFeature::prStopThumbnailFileDatasExtraction()
	{
		const bool res = prThumbnailGenerationThread.use_count() && prThumbnailGenerationThread->joinable();
		if (res)
		{
			prThumbnailGenerationThread.reset();
		}

		return res;
	}

	void IGFD::ThumbnailFeature::prThreadThumbnailFileDatasExtractionFunc()
	{
		prCountFiles = 0U;
		prIsWorking = true;

		// infinite loop while is thread working
		while(prIsWorking)
		{
			if (!prThumbnailFileDatasToGet.empty())
			{
				std::shared_ptr<FileInfos> file = nullptr;
				prThumbnailFileDatasToGetMutex.lock();
				//get the first file in the list
				file = (*prThumbnailFileDatasToGet.begin());
				prThumbnailFileDatasToGetMutex.unlock();

				// retrieve datas of the texture file if its an image file
				if (file.use_count())
				{
					if (file->fileType.isFile()) //-V522
					{
						if (file->fileExt == ".png"
							|| file->fileExt == ".bmp"
							|| file->fileExt == ".tga"
							|| file->fileExt == ".jpg" || file->fileExt == ".jpeg"
							|| file->fileExt == ".gif"
							|| file->fileExt == ".psd"
							|| file->fileExt == ".pic"
							|| file->fileExt == ".ppm" || file->fileExt == ".pgm"
							//|| file->fileExt == ".hdr" => format float so in few times
							)
						{
							auto fpn = file->filePath + std::string(1u, PATH_SEP) + file->fileNameExt;

							int w = 0;
							int h = 0;
							int chans = 0;
							uint8_t *datas = stbi_load(fpn.c_str(), &w, &h, &chans, STBI_rgb_alpha);
							if (datas)
							{
								if (w && h)
								{
									// resize with respect to glyph ratio
									const float ratioX = (float)w / (float)h;
									const float newX = DisplayMode_ThumbailsList_ImageHeight * ratioX;
									float newY = w / ratioX;
									if (newX < w) 
										newY = DisplayMode_ThumbailsList_ImageHeight;

									const auto newWidth = (int)newX;
									const auto newHeight = (int)newY;
									const auto newBufSize = (size_t)(newWidth * newHeight * 4U); //-V112 //-V1028
									auto resizedData = new uint8_t[newBufSize];

									const int resizeSucceeded = stbir_resize_uint8(
										datas, w, h, 0,
										resizedData, newWidth, newHeight, 0,
										4); //-V112

									if (resizeSucceeded)
									{
										auto th = &file->thumbnailInfo;

										th->textureFileDatas = resizedData;
										th->textureWidth = newWidth;
										th->textureHeight = newHeight;
										th->textureChannels = 4; //-V112

																 // we set that at least, because will launch the gpu creation of the texture in the main thread
										th->isReadyToUpload = true;

										// need gpu loading
										prAddThumbnailToCreate(file);
									}
								}
								else
								{
									printf("image loading fail : w:%i h:%i c:%i\n", w, h, 4); //-V112
								}

								stbi_image_free(datas);
							}
						}
					}

					// peu importe le resultat on vire le fichicer
					// remove form this list
					// write => thread concurency issues
					prThumbnailFileDatasToGetMutex.lock();
					prThumbnailFileDatasToGet.pop_front();
					prThumbnailFileDatasToGetMutex.unlock();
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}
	}

	inline void inVariadicProgressBar(float fraction, const ImVec2& size_arg, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		char TempBuffer[512];
		const int w = vsnprintf(TempBuffer, 511, fmt, args);
		va_end(args);
		if (w)
		{
			ImGui::ProgressBar(fraction, size_arg, TempBuffer);
		}
	}

	void IGFD::ThumbnailFeature::prDrawThumbnailGenerationProgress()
	{
		if (prThumbnailGenerationThread.use_count() && prThumbnailGenerationThread->joinable())
		{
			if (!prThumbnailFileDatasToGet.empty())
			{
				const auto p = (float)((double)prCountFiles / (double)prThumbnailFileDatasToGet.size()); // read => no thread concurency issues
				inVariadicProgressBar(p, ImVec2(50, 0), "%u/%u", prCountFiles, (uint32_t)prThumbnailFileDatasToGet.size()); // read => no thread concurency issues
				ImGui::SameLine();
			}
		}
	}

	void IGFD::ThumbnailFeature::prAddThumbnailToLoad(const std::shared_ptr<FileInfos>& vFileInfos)
	{
		if (vFileInfos.use_count())
		{
			if (vFileInfos->fileType.isFile())
			{
				if (vFileInfos->fileExt == ".png"
					|| vFileInfos->fileExt == ".bmp"
					|| vFileInfos->fileExt == ".tga"
					|| vFileInfos->fileExt == ".jpg" || vFileInfos->fileExt == ".jpeg"
					|| vFileInfos->fileExt == ".gif"
					|| vFileInfos->fileExt == ".psd"
					|| vFileInfos->fileExt == ".pic"
					|| vFileInfos->fileExt == ".ppm" || vFileInfos->fileExt == ".pgm"
					//|| file->fileExt == ".hdr" => format float so in few times
					)
				{
					// write => thread concurency issues
					prThumbnailFileDatasToGetMutex.lock();
					prThumbnailFileDatasToGet.push_back(vFileInfos);
					vFileInfos->thumbnailInfo.isLoadingOrLoaded = true;
					prThumbnailFileDatasToGetMutex.unlock();
				}
			}
		}
	}

	void IGFD::ThumbnailFeature::prAddThumbnailToCreate(const std::shared_ptr<FileInfos>& vFileInfos)
	{
		if (vFileInfos.use_count())
		{
			// write => thread concurency issues
			prThumbnailToCreateMutex.lock();
			prThumbnailToCreate.push_back(vFileInfos);
			prThumbnailToCreateMutex.unlock();
		}
	}

	void IGFD::ThumbnailFeature::prAddThumbnailToDestroy(const IGFD_Thumbnail_Info& vIGFD_Thumbnail_Info)
	{
		// write => thread concurency issues
		prThumbnailToDestroyMutex.lock();
		prThumbnailToDestroy.push_back(vIGFD_Thumbnail_Info);
		prThumbnailToDestroyMutex.unlock();
	}

	void IGFD::ThumbnailFeature::prDrawDisplayModeToolBar()
	{
		if (IMGUI_RADIO_BUTTON(DisplayMode_FilesList_ButtonString,
			prDisplayMode == DisplayModeEnum::FILE_LIST))
			prDisplayMode = DisplayModeEnum::FILE_LIST;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_FilesList_ButtonHelp);
		ImGui::SameLine();
		if (IMGUI_RADIO_BUTTON(DisplayMode_ThumbailsList_ButtonString,
			prDisplayMode == DisplayModeEnum::THUMBNAILS_LIST))
			prDisplayMode = DisplayModeEnum::THUMBNAILS_LIST;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_ThumbailsList_ButtonHelp);
		ImGui::SameLine();
		/* todo
		if (IMGUI_RADIO_BUTTON(DisplayMode_ThumbailsGrid_ButtonString,
		prDisplayMode == DisplayModeEnum::THUMBNAILS_GRID))
		prDisplayMode = DisplayModeEnum::THUMBNAILS_GRID;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_ThumbailsGrid_ButtonHelp);
		ImGui::SameLine();
		*/
		prDrawThumbnailGenerationProgress();
	}

	void IGFD::ThumbnailFeature::prClearThumbnails(FileDialogInternal& vFileDialogInternal)
	{
		// directory wil be changed so the file list will be erased
		if (vFileDialogInternal.puFileManager.puPathClicked)
		{
			size_t count = vFileDialogInternal.puFileManager.GetFullFileListSize();
			for (size_t idx = 0U; idx < count; idx++)
			{
				auto file = vFileDialogInternal.puFileManager.GetFullFileAt(idx);
				if (file.use_count())
				{
					if (file->thumbnailInfo.isReadyToDisplay) //-V522
					{
						prAddThumbnailToDestroy(file->thumbnailInfo);
					}
				}
			}
		}
	}

	void IGFD::ThumbnailFeature::SetCreateThumbnailCallback(const CreateThumbnailFun& vCreateThumbnailFun)
	{
		prCreateThumbnailFun = vCreateThumbnailFun;
	}

	void IGFD::ThumbnailFeature::SetDestroyThumbnailCallback(const DestroyThumbnailFun& vCreateThumbnailFun)
	{
		prDestroyThumbnailFun = vCreateThumbnailFun;
	}

	void IGFD::ThumbnailFeature::ManageGPUThumbnails()
	{
		if (prCreateThumbnailFun)
		{
			if (!prThumbnailToCreate.empty())
			{
				for (const auto& file : prThumbnailToCreate)
				{
					if (file.use_count())
					{
						prCreateThumbnailFun(&file->thumbnailInfo);
					}
				}
				prThumbnailToCreateMutex.lock();
				prThumbnailToCreate.clear();
				prThumbnailToCreateMutex.unlock();
			}
		}
		else
		{
			printf("No Callback found for create texture\nYou need to define the callback with a call to SetCreateThumbnailCallback\n");
		}

		if (prDestroyThumbnailFun)
		{
			if (!prThumbnailToDestroy.empty())
			{
				for (auto thumbnail : prThumbnailToDestroy)
				{
					prDestroyThumbnailFun(&thumbnail);
				}
				prThumbnailToDestroyMutex.lock();
				prThumbnailToDestroy.clear();
				prThumbnailToDestroyMutex.unlock();
			}
		}
		else
		{
			printf("No Callback found for destroy texture\nYou need to define the callback with a call to SetCreateThumbnailCallback\n");
		}
	}

#endif // USE_THUMBNAILS

	/////////////////////////////////////////////////////////////////////////////////////
	//// BOOKMARK FEATURE ///////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	IGFD::BookMarkFeature::BookMarkFeature()
	{
#ifdef USE_BOOKMARK
		prBookmarkWidth = defaultBookmarkPaneWith;
#endif // USE_BOOKMARK
	}

#ifdef USE_BOOKMARK
	void IGFD::BookMarkFeature::prDrawBookmarkButton()
	{
		IMGUI_TOGGLE_BUTTON(bookmarksButtonString, &prBookmarkPaneShown);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(bookmarksButtonHelpString);
	}

	bool IGFD::BookMarkFeature::prDrawBookmarkPane(FileDialogInternal& vFileDialogInternal, const ImVec2& vSize)
	{
		bool res = false;

		ImGui::BeginChild("##bookmarkpane", vSize);

		static int selectedBookmarkForEdition = -1;

		if (IMGUI_BUTTON(addBookmarkButtonString "##ImGuiFileDialogAddBookmark"))
		{
			if (!vFileDialogInternal.puFileManager.IsComposerEmpty())
			{
				BookmarkStruct bookmark;
				bookmark.name = vFileDialogInternal.puFileManager.GetBack();
				bookmark.path = vFileDialogInternal.puFileManager.GetCurrentPath();
				prBookmarks.push_back(bookmark);
			}
		}
		if (selectedBookmarkForEdition >= 0 &&
			selectedBookmarkForEdition < (int)prBookmarks.size())
		{
			ImGui::SameLine();
			if (IMGUI_BUTTON(removeBookmarkButtonString "##ImGuiFileDialogAddBookmark"))
			{
				prBookmarks.erase(prBookmarks.begin() + selectedBookmarkForEdition);
				if (selectedBookmarkForEdition == (int)prBookmarks.size())
					selectedBookmarkForEdition--;
			}

			if (selectedBookmarkForEdition >= 0 &&
				selectedBookmarkForEdition < (int)prBookmarks.size())
			{
				ImGui::SameLine();

				ImGui::PushItemWidth(vSize.x - ImGui::GetCursorPosX());
				if (ImGui::InputText("##ImGuiFileDialogBookmarkEdit", prBookmarkEditBuffer, MAX_FILE_DIALOG_NAME_BUFFER))
				{
					prBookmarks[(size_t)selectedBookmarkForEdition].name = std::string(prBookmarkEditBuffer);
				}
				ImGui::PopItemWidth();
			}
		}

		ImGui::Separator();

		if (!prBookmarks.empty())
		{
			prBookmarkClipper.Begin((int)prBookmarks.size(), ImGui::GetTextLineHeightWithSpacing());
			while (prBookmarkClipper.Step())
			{
				for (int i = prBookmarkClipper.DisplayStart; i < prBookmarkClipper.DisplayEnd; i++)
				{
					if (i < 0) continue;
					const BookmarkStruct& bookmark = prBookmarks[(size_t)i];
					ImGui::PushID(i);
					if (ImGui::Selectable(bookmark.name.c_str(), selectedBookmarkForEdition == i,ImGuiSelectableFlags_AllowDoubleClick) ||
						(selectedBookmarkForEdition == -1 && bookmark.path == vFileDialogInternal.puFileManager.GetCurrentPath())) // select if path is current
					{
						selectedBookmarkForEdition = i;
						IGFD::Utils::ResetBuffer(prBookmarkEditBuffer);
						IGFD::Utils::AppendToBuffer(prBookmarkEditBuffer, MAX_FILE_DIALOG_NAME_BUFFER, bookmark.name);

						if (ImGui::IsMouseDoubleClicked(0)) // apply path
						{
							vFileDialogInternal.puFileManager.SetCurrentPath(bookmark.path);
							vFileDialogInternal.puFileManager.OpenCurrentPath(vFileDialogInternal);
							res = true;
						}
					}
					ImGui::PopID();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%s", bookmark.path.c_str()); //-V111
				}
			}
			prBookmarkClipper.End();
		}

		ImGui::EndChild();

		return res;
	}

	std::string IGFD::BookMarkFeature::SerializeBookmarks(const bool& vDontSerializeCodeBasedBookmarks)
	{
		std::string res;

		size_t idx = 0;
		for (auto& it : prBookmarks)
		{
			if (vDontSerializeCodeBasedBookmarks && it.defined_by_code)
				continue;

			if (idx++ != 0)
				res += "##"; // ## because reserved by imgui, so an input text cant have ##

			res += it.name + "##" + it.path;
		}

		return res;
	}

	void IGFD::BookMarkFeature::DeserializeBookmarks(const std::string& vBookmarks)
	{
		if (!vBookmarks.empty())
		{
			prBookmarks.clear();
			auto arr = IGFD::Utils::SplitStringToVector(vBookmarks, '#', false);
			for (size_t i = 0; i < arr.size(); i += 2)
			{
				if (i + 1 < arr.size()) // for avoid crash if arr size is impair due to user mistake after edition
				{
					BookmarkStruct bookmark;
					bookmark.name = arr[i];
					// if bad format we jump this bookmark
					bookmark.path = arr[i + 1];
					prBookmarks.push_back(bookmark);
				}
			}
		}
	}

	void IGFD::BookMarkFeature::AddBookmark(const std::string& vBookMarkName, const std::string& vBookMarkPath)
	{
		if (vBookMarkName.empty() || vBookMarkPath.empty())
			return;

		BookmarkStruct bookmark;
		bookmark.name = vBookMarkName;
		bookmark.path = vBookMarkPath;
		bookmark.defined_by_code = true;
		prBookmarks.push_back(bookmark);
	}

	bool IGFD::BookMarkFeature::RemoveBookmark(const std::string& vBookMarkName)
	{
		if (vBookMarkName.empty())
			return false;

		for (auto bookmark_it = prBookmarks.begin(); bookmark_it != prBookmarks.end(); ++bookmark_it)
		{
			if ((*bookmark_it).name == vBookMarkName)
			{
				prBookmarks.erase(bookmark_it);
				return true;
			}
		}

		return false;
	}
#endif // USE_BOOKMARK

	/////////////////////////////////////////////////////////////////////////////////////
	//// KEY EXPLORER FEATURE ///////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	KeyExplorerFeature::KeyExplorerFeature() = default;

#ifdef USE_EXPLORATION_BY_KEYS
	bool IGFD::KeyExplorerFeature::prLocateItem_Loop(FileDialogInternal& vFileDialogInternal, ImWchar vC)
	{
		bool found = false;

		auto& fdi = vFileDialogInternal.puFileManager;
		if (!fdi.IsFilteredListEmpty())
		{
			auto countFiles = fdi.GetFilteredListSize();
			for (size_t i = prLocateFileByInputChar_lastFileIdx; i < countFiles; i++)
			{
				auto nfo = fdi.GetFilteredFileAt(i);
				if (nfo.use_count())
				{
					if (nfo->fileNameExt_optimized[0] == vC || // lower case search //-V522
						nfo->fileNameExt[0] == vC) // maybe upper case search
					{
						//float p = ((float)i) * ImGui::GetTextLineHeightWithSpacing();
						float p = (float)((double)i / (double)countFiles) * ImGui::GetScrollMaxY();
						ImGui::SetScrollY(p);
						prLocateFileByInputChar_lastFound = true;
						prLocateFileByInputChar_lastFileIdx = i;
						prStartFlashItem(prLocateFileByInputChar_lastFileIdx);

						auto infos = fdi.GetFilteredFileAt(prLocateFileByInputChar_lastFileIdx);
						if (infos.use_count())
						{
							if (infos->fileType.isDir()) //-V522
							{
								if (fdi.puDLGDirectoryMode) // directory chooser
								{
									fdi.SelectFileName(vFileDialogInternal, infos);
								}
							}
							else
							{
								fdi.SelectFileName(vFileDialogInternal, infos);
							}

							found = true;
							break;
						}
					}
				}
			}
		}

		return found;
	}

	void IGFD::KeyExplorerFeature::prLocateByInputKey(FileDialogInternal& vFileDialogInternal)
	{
		ImGuiContext& g = *GImGui;
		auto& fdi = vFileDialogInternal.puFileManager;
		if (!g.ActiveId && !fdi.IsFilteredListEmpty())
		{
			auto& queueChar = ImGui::GetIO().InputQueueCharacters;
			auto countFiles = fdi.GetFilteredListSize();

			// point by char
			if (!queueChar.empty())
			{
				ImWchar c = queueChar.back();
				if (prLocateFileByInputChar_InputQueueCharactersSize != queueChar.size())
				{
					if (c == prLocateFileByInputChar_lastChar) // next file starting with same char until
					{
						if (prLocateFileByInputChar_lastFileIdx < countFiles - 1U)
							prLocateFileByInputChar_lastFileIdx++;
						else
							prLocateFileByInputChar_lastFileIdx = 0;
					}

					if (!prLocateItem_Loop(vFileDialogInternal, c))
					{
						// not found, loop again from 0 this time
						prLocateFileByInputChar_lastFileIdx = 0;
						prLocateItem_Loop(vFileDialogInternal, c);
					}

					prLocateFileByInputChar_lastChar = c;
				}
			}

			prLocateFileByInputChar_InputQueueCharactersSize = queueChar.size();
		}
	}

	void IGFD::KeyExplorerFeature::prExploreWithkeys(FileDialogInternal& vFileDialogInternal, ImGuiID vListViewID)
	{
		auto& fdi = vFileDialogInternal.puFileManager;
		if (!fdi.IsFilteredListEmpty())
		{
			bool canWeExplore = false;
			bool hasNav = (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard);

			ImGuiContext& g = *GImGui;
			if (!hasNav && !g.ActiveId) // no nav and no activated inputs
				canWeExplore = true;

			if (g.NavId && g.NavId == vListViewID)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_Enter) ||
					ImGui::IsKeyPressed(ImGuiKey_KeypadEnter) ||
					ImGui::IsKeyPressed(ImGuiKey_Space))
				{
					ImGui::ActivateItem(vListViewID);
					ImGui::SetActiveID(vListViewID, g.CurrentWindow);
				}
			}

			if (vListViewID == g.LastActiveId-1) // if listview id is the last acticated nav id (ImGui::ActivateItem(vListViewID);)
				canWeExplore = true;

			if (canWeExplore && ImGui::IsWindowFocused())
			{
				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				{
					ImGui::ClearActiveID();
					g.LastActiveId = 0;
				}

				auto countFiles = fdi.GetFilteredListSize();

				// explore
				bool exploreByKey = false;
				bool enterInDirectory = false;
				bool exitDirectory = false;

				if ((hasNav && ImGui::IsKeyPressed(ImGuiKey_UpArrow)) || (!hasNav && ImGui::IsKeyPressed(IGFD_KEY_UP)))
				{
					exploreByKey = true;
					if (prLocateFileByInputChar_lastFileIdx > 0)
						prLocateFileByInputChar_lastFileIdx--;
					else
						prLocateFileByInputChar_lastFileIdx = countFiles - 1U;
				}
				else if ((hasNav && ImGui::IsKeyPressed(ImGuiKey_DownArrow)) || (!hasNav && ImGui::IsKeyPressed(IGFD_KEY_DOWN)))
				{
					exploreByKey = true;
					if (prLocateFileByInputChar_lastFileIdx < countFiles - 1U)
						prLocateFileByInputChar_lastFileIdx++;
					else
						prLocateFileByInputChar_lastFileIdx = 0U;
				}
				else if (ImGui::IsKeyReleased(IGFD_KEY_ENTER))
				{
					exploreByKey = true;
					enterInDirectory = true;
				}
				else if (ImGui::IsKeyReleased(IGFD_KEY_BACKSPACE))
				{
					exploreByKey = true;
					exitDirectory = true;
				}

				if (exploreByKey)
				{
					//float totalHeight = prFilteredFileList.size() * ImGui::GetTextLineHeightWithSpacing();
					float p = (float)((double)prLocateFileByInputChar_lastFileIdx / (double)(countFiles - 1U)) * ImGui::GetScrollMaxY();// seems not udpated in tables version outside tables
																																		//float p = ((float)locateFileByInputChar_lastFileIdx) * ImGui::GetTextLineHeightWithSpacing();
					ImGui::SetScrollY(p);
					prStartFlashItem(prLocateFileByInputChar_lastFileIdx);

					auto infos = fdi.GetFilteredFileAt(prLocateFileByInputChar_lastFileIdx);
					if (infos.use_count())
					{
						if (infos->fileType.isDir()) //-V522
						{
							if (!fdi.puDLGDirectoryMode || enterInDirectory)
							{
								if (enterInDirectory)
								{
									if (fdi.SelectDirectory(infos))
									{
										// changement de repertoire
										vFileDialogInternal.puFileManager.OpenCurrentPath(vFileDialogInternal);
										if (prLocateFileByInputChar_lastFileIdx > countFiles - 1U)
										{
											prLocateFileByInputChar_lastFileIdx = 0;
										}
									}
								}
							}
							else // directory chooser
							{
								fdi.SelectFileName(vFileDialogInternal, infos);
							}
						}
						else
						{
							fdi.SelectFileName(vFileDialogInternal, infos);

							if (enterInDirectory)
							{
								vFileDialogInternal.puIsOk = true;
							}
						}

						if (exitDirectory)
						{
							auto nfo = std::make_shared<FileInfos>();
							nfo->fileNameExt = "..";

							if (fdi.SelectDirectory(nfo))
							{
								// changement de repertoire
								vFileDialogInternal.puFileManager.OpenCurrentPath(vFileDialogInternal);
								if (prLocateFileByInputChar_lastFileIdx > countFiles - 1U)
								{
									prLocateFileByInputChar_lastFileIdx = 0;
								}
							}
#ifdef _IGFD_WIN_
							else
							{
								if (fdi.GetComposerSize() == 1U)
								{
									if (fdi.GetDrives())
									{
										fdi.ApplyFilteringOnFileList(vFileDialogInternal);
									}
								}
							}
#endif // _IGFD_WIN_
						}
					}
				}
			}
		}
	}

	bool IGFD::KeyExplorerFeature::prFlashableSelectable(const char* label, bool selected,
		ImGuiSelectableFlags flags, bool vFlashing, const ImVec2& size_arg)
	{
		using namespace ImGui;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
		ImGuiID id = window->GetID(label);
		ImVec2 label_size = CalcTextSize(label, nullptr, true);
		ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y); //-V550
		ImVec2 pos = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;
		ItemSize(size, 0.0f);

		// Fill horizontal space
		// We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
		const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
		const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
		const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
		if (fabs(size_arg.x) < FLT_EPSILON || (flags & ImGuiSelectableFlags_SpanAvailWidth))
			size.x = ImMax(label_size.x, max_x - min_x);

		// Text stays at the submission position, but bounding box may be extended on both sides
		const ImVec2 text_min = pos;
		const ImVec2 text_max(min_x + size.x, pos.y + size.y);

		// Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
		ImRect bb(min_x, pos.y, text_max.x, text_max.y);
		if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
		{
			const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
			const float spacing_y = style.ItemSpacing.y;
			const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
			const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
			bb.Min.x -= spacing_L;
			bb.Min.y -= spacing_U;
			bb.Max.x += (spacing_x - spacing_L);
			bb.Max.y += (spacing_y - spacing_U);
		}
		//if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

		// Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
		const float backup_clip_rect_min_x = window->ClipRect.Min.x;
		const float backup_clip_rect_max_x = window->ClipRect.Max.x;
		if (span_all_columns)
		{
			window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
			window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
		}

		bool item_add;
		const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
		if (disabled_item)
		{
			ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
			g.CurrentItemFlags |= ImGuiItemFlags_Disabled;
			item_add = ItemAdd(bb, id);
			g.CurrentItemFlags = backup_item_flags;
		}
		else
		{
			item_add = ItemAdd(bb, id);
		}

		if (span_all_columns)
		{
			window->ClipRect.Min.x = backup_clip_rect_min_x;
			window->ClipRect.Max.x = backup_clip_rect_max_x;
		}

		if (!item_add)
			return false;

		const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
		if (disabled_item && !disabled_global) // Only testing this as an optimization
			BeginDisabled(true);

		// FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
		// which would be advantageous since most selectable are not selected.
		if (span_all_columns && window->DC.CurrentColumns)
			PushColumnsBackground();
		else if (span_all_columns && g.CurrentTable)
			TablePushBackgroundChannel();

		// We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
		ImGuiButtonFlags button_flags = 0;
		if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
		if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
		if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
		if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
		if (flags & ImGuiSelectableFlags_AllowItemOverlap) { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

		const bool was_selected = selected;
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

		// Auto-select when moved into
		// - This will be more fully fleshed in the range-select branch
		// - This is not exposed as it won't nicely work with some user side handling of shift/control
		// - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
		//   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
		//   - (2) usage will fail with clipped items
		//   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
		if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == window->DC.NavFocusScopeIdCurrent)
			if (g.NavJustMovedToId == id)
				selected = pressed = true;

		// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
		if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
		{
			if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
			{
				SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent, ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
				g.NavDisableHighlight = true;
			}
		}
		if (pressed)
			MarkItemEdited(id);

		if (flags & ImGuiSelectableFlags_AllowItemOverlap)
			SetItemAllowOverlap();

		// In this branch, Selectable() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
			g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		if ((held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld)) || vFlashing)
			hovered = true;
		if (hovered || selected)
		{
			const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
			RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
		}
		RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

		if (span_all_columns && window->DC.CurrentColumns)
			PopColumnsBackground();
		else if (span_all_columns && g.CurrentTable)
			TablePopBackgroundChannel();

		RenderTextClipped(text_min, text_max, label, nullptr, &label_size, style.SelectableTextAlign, &bb);

		// Automatically close popups
		if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
			CloseCurrentPopup();

		if (disabled_item && !disabled_global)
			EndDisabled();

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
		return pressed; //-V1020
	}

	void IGFD::KeyExplorerFeature::prStartFlashItem(size_t vIdx)
	{
		prFlashAlpha = 1.0f;
		prFlashedItem = vIdx;
	}

	bool IGFD::KeyExplorerFeature::prBeginFlashItem(size_t vIdx)
	{
		bool res = false;

		if (prFlashedItem == vIdx &&
			std::abs(prFlashAlpha - 0.0f) > 0.00001f)
		{
			prFlashAlpha -= prFlashAlphaAttenInSecs * ImGui::GetIO().DeltaTime;
			if (prFlashAlpha < 0.0f) prFlashAlpha = 0.0f;

			ImVec4 hov = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			hov.w = prFlashAlpha;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hov);
			res = true;
		}

		return res;
	}

	void IGFD::KeyExplorerFeature::prEndFlashItem()
	{
		ImGui::PopStyleColor();
	}

	void IGFD::KeyExplorerFeature::SetFlashingAttenuationInSeconds(float vAttenValue)
	{
		prFlashAlphaAttenInSecs = 1.0f / ImMax(vAttenValue, 0.01f);
	}
#endif // USE_EXPLORATION_BY_KEYS

	/////////////////////////////////////////////////////////////////////////////////////
	//// FILE DIALOG CONSTRUCTOR / DESTRUCTOR ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	IGFD::FileDialog::FileDialog() : BookMarkFeature(), KeyExplorerFeature(), ThumbnailFeature() {}
	IGFD::FileDialog::~FileDialog() = default;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// FILE DIALOG STANDARD DIALOG ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// path and fileNameExt can be specified
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vPath,
		const std::string& vFileName,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prFileDialogInternal.puShowDialog) // if already opened, quit
			return;

		prFileDialogInternal.ResetForNewDialog();

		prFileDialogInternal.puDLGkey = vKey;
		prFileDialogInternal.puDLGtitle = vTitle;
		prFileDialogInternal.puDLGuserDatas = vUserDatas;
		prFileDialogInternal.puDLGflags = vFlags;
		prFileDialogInternal.puDLGoptionsPane = nullptr;
		prFileDialogInternal.puDLGoptionsPaneWidth = 0.0f;

		prFileDialogInternal.puFilterManager.puDLGdefaultExt.clear();
		prFileDialogInternal.puFilterManager.ParseFilters(vFilters);

		prFileDialogInternal.puFileManager.puDLGDirectoryMode = (vFilters == nullptr);
		if (vPath.empty())
			prFileDialogInternal.puFileManager.puDLGpath = prFileDialogInternal.puFileManager.GetCurrentPath();
		else
			prFileDialogInternal.puFileManager.puDLGpath = vPath;
		prFileDialogInternal.puFileManager.SetCurrentPath(vPath);
		prFileDialogInternal.puFileManager.puDLGcountSelectionMax = (size_t)vCountSelectionMax;
		prFileDialogInternal.puFileManager.SetDefaultFileName(vFileName);

		prFileDialogInternal.puFileManager.ClearAll();

		prFileDialogInternal.puShowDialog = true;					// open dialog
	}

	// path and filename are obtained from filePathName
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vFilePathName,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prFileDialogInternal.puShowDialog) // if already opened, quit
			return;

		prFileDialogInternal.ResetForNewDialog();

		prFileDialogInternal.puDLGkey = vKey;
		prFileDialogInternal.puDLGtitle = vTitle;
		prFileDialogInternal.puDLGoptionsPane = nullptr;
		prFileDialogInternal.puDLGoptionsPaneWidth = 0.0f;
		prFileDialogInternal.puDLGuserDatas = vUserDatas;
		prFileDialogInternal.puDLGflags = vFlags;

		auto ps = IGFD::Utils::ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			prFileDialogInternal.puFileManager.puDLGpath = ps.path;
			prFileDialogInternal.puFileManager.SetDefaultFileName(ps.name);
			prFileDialogInternal.puFilterManager.puDLGdefaultExt = "." + ps.ext;
		}
		else
		{
			prFileDialogInternal.puFileManager.puDLGpath = prFileDialogInternal.puFileManager.GetCurrentPath();
			prFileDialogInternal.puFileManager.SetDefaultFileName("");
			prFileDialogInternal.puFilterManager.puDLGdefaultExt.clear();
		}

		prFileDialogInternal.puFilterManager.ParseFilters(vFilters);
		prFileDialogInternal.puFilterManager.SetSelectedFilterWithExt(
			prFileDialogInternal.puFilterManager.puDLGdefaultExt);

		prFileDialogInternal.puFileManager.SetCurrentPath(prFileDialogInternal.puFileManager.puDLGpath);

		prFileDialogInternal.puFileManager.puDLGDirectoryMode = (vFilters == nullptr);
		prFileDialogInternal.puFileManager.puDLGcountSelectionMax = vCountSelectionMax; //-V101

		prFileDialogInternal.puFileManager.ClearAll();

		prFileDialogInternal.puShowDialog = true;
	}

	// with pane
	// path and fileNameExt can be specified
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vPath,
		const std::string& vFileName,
		const PaneFun& vSidePane,
		const float& vSidePaneWidth,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prFileDialogInternal.puShowDialog) // if already opened, quit
			return;

		prFileDialogInternal.ResetForNewDialog();

		prFileDialogInternal.puDLGkey = vKey;
		prFileDialogInternal.puDLGtitle = vTitle;
		prFileDialogInternal.puDLGuserDatas = vUserDatas;
		prFileDialogInternal.puDLGflags = vFlags;
		prFileDialogInternal.puDLGoptionsPane = vSidePane;
		prFileDialogInternal.puDLGoptionsPaneWidth = vSidePaneWidth;

		prFileDialogInternal.puFilterManager.puDLGdefaultExt.clear();
		prFileDialogInternal.puFilterManager.ParseFilters(vFilters);

		prFileDialogInternal.puFileManager.puDLGcountSelectionMax = (size_t)vCountSelectionMax;
		prFileDialogInternal.puFileManager.puDLGDirectoryMode = (vFilters == nullptr);
		if (vPath.empty())
			prFileDialogInternal.puFileManager.puDLGpath = prFileDialogInternal.puFileManager.GetCurrentPath();
		else
			prFileDialogInternal.puFileManager.puDLGpath = vPath;

		prFileDialogInternal.puFileManager.SetCurrentPath(prFileDialogInternal.puFileManager.puDLGpath);

		prFileDialogInternal.puFileManager.SetDefaultFileName(vFileName);

		prFileDialogInternal.puFileManager.ClearAll();

		prFileDialogInternal.puShowDialog = true;					// open dialog
	}

	// with pane
	// path and filename are obtained from filePathName
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vFilePathName,
		const PaneFun& vSidePane,
		const float& vSidePaneWidth,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prFileDialogInternal.puShowDialog) // if already opened, quit
			return;

		prFileDialogInternal.ResetForNewDialog();

		prFileDialogInternal.puDLGkey = vKey;
		prFileDialogInternal.puDLGtitle = vTitle;
		prFileDialogInternal.puDLGoptionsPane = vSidePane;
		prFileDialogInternal.puDLGoptionsPaneWidth = vSidePaneWidth;
		prFileDialogInternal.puDLGuserDatas = vUserDatas;
		prFileDialogInternal.puDLGflags = vFlags;

		auto ps = IGFD::Utils::ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			prFileDialogInternal.puFileManager.puDLGpath = ps.path;
			prFileDialogInternal.puFileManager.SetDefaultFileName(vFilePathName);
			prFileDialogInternal.puFilterManager.puDLGdefaultExt = "." + ps.ext;
		}
		else
		{
			prFileDialogInternal.puFileManager.puDLGpath = prFileDialogInternal.puFileManager.GetCurrentPath();
			prFileDialogInternal.puFileManager.SetDefaultFileName("");
			prFileDialogInternal.puFilterManager.puDLGdefaultExt.clear();
		}

		prFileDialogInternal.puFileManager.SetCurrentPath(prFileDialogInternal.puFileManager.puDLGpath);

		prFileDialogInternal.puFileManager.puDLGcountSelectionMax = vCountSelectionMax; //-V101
		prFileDialogInternal.puFileManager.puDLGDirectoryMode = (vFilters == nullptr);
		prFileDialogInternal.puFilterManager.ParseFilters(vFilters);
		prFileDialogInternal.puFilterManager.SetSelectedFilterWithExt(
			prFileDialogInternal.puFilterManager.puDLGdefaultExt);

		prFileDialogInternal.puFileManager.ClearAll();

		prFileDialogInternal.puShowDialog = true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// FILE DIALOG DISPLAY FUNCTION ///////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	bool IGFD::FileDialog::Display(const std::string& vKey, ImGuiWindowFlags vFlags, ImVec2 vMinSize, ImVec2 vMaxSize)
	{
		bool res = false;

		if (prFileDialogInternal.puShowDialog && prFileDialogInternal.puDLGkey == vKey)
		{
			if (prFileDialogInternal.puUseCustomLocale)
				setlocale(prFileDialogInternal.puLocaleCategory, prFileDialogInternal.puLocaleBegin.c_str());

			auto& fdFile = prFileDialogInternal.puFileManager;
			auto& fdFilter = prFileDialogInternal.puFilterManager;

			static ImGuiWindowFlags flags; // todo: not a good solution for multi instance, to fix

										   // to be sure than only one dialog is displayed per frame
			ImGuiContext& g = *GImGui;
			if (g.FrameCount == prFileDialogInternal.puLastImGuiFrameCount) // one instance was displayed this frame before for this key +> quit
				return res;
			prFileDialogInternal.puLastImGuiFrameCount = g.FrameCount; // mark this instance as used this frame

			std::string name = prFileDialogInternal.puDLGtitle + "##" + prFileDialogInternal.puDLGkey;
			if (prFileDialogInternal.puName != name)
			{
				fdFile.ClearComposer();
				fdFile.ClearFileLists();
				flags = vFlags;
			}

			NewFrame();

#ifdef IMGUI_HAS_VIEWPORT
			if (!ImGui::GetIO().ConfigViewportsNoDecoration)
			{
				// https://github.com/ocornut/imgui/issues/4534
				ImGuiWindowClass window_class;
				window_class.ViewportFlagsOverrideClear = ImGuiViewportFlags_NoDecoration;
				ImGui::SetNextWindowClass(&window_class);
			}
#endif // IMGUI_HAS_VIEWPORT

			bool beg = false;
			if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_NoDialog) // disable our own dialog system (standard or modal)
			{
				beg = true;
			}
			else
			{
				ImGui::SetNextWindowSizeConstraints(vMinSize, vMaxSize);

				if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_Modal &&
					!prFileDialogInternal.puOkResultToConfirm) // disable modal because the confirm dialog for overwrite is a new modal
				{
					ImGui::OpenPopup(name.c_str());
					beg = ImGui::BeginPopupModal(name.c_str(), (bool*)nullptr,
						flags | ImGuiWindowFlags_NoScrollbar);
				}
				else
				{
					beg = ImGui::Begin(name.c_str(), (bool*)nullptr, flags | ImGuiWindowFlags_NoScrollbar);
				}
			}
			if (beg)
			{
#ifdef IMGUI_HAS_VIEWPORT
				// if decoration is enabled we disable the resizing feature of imgui for avoid crash with SDL2 and GLFW3
				if (ImGui::GetIO().ConfigViewportsNoDecoration)
				{
					flags = vFlags;
				}
				else
				{
					auto win = ImGui::GetCurrentWindowRead();
					if (win->Viewport->Idx != 0)
						flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
					else
						flags = vFlags;
				}
#endif // IMGUI_HAS_VIEWPORT

				ImGuiID _frameId = ImGui::GetID(name.c_str());
				ImVec2 frameSize = ImVec2(0, 0);
				if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_NoDialog)
					frameSize = vMaxSize;
				if (ImGui::BeginChild(_frameId, frameSize,
					false, flags | ImGuiWindowFlags_NoScrollbar))
				{
					prFileDialogInternal.puName = name; //-V820
					puAnyWindowsHovered |= ImGui::IsWindowHovered();

					if (fdFile.puDLGpath.empty())
						fdFile.puDLGpath = "."; // defaut path is '.'

					fdFilter.SetDefaultFilterIfNotDefined();

					// init list of files
					if (fdFile.IsFileListEmpty() && !fdFile.puShowDrives)
					{
						IGFD::Utils::ReplaceString(fdFile.puDLGDefaultFileName, fdFile.puDLGpath, ""); // local path
						if (!fdFile.puDLGDefaultFileName.empty())
						{
							fdFile.SetDefaultFileName(fdFile.puDLGDefaultFileName);
							fdFilter.SetSelectedFilterWithExt(fdFilter.puDLGdefaultExt);
						}
						else if (fdFile.puDLGDirectoryMode) // directory mode
							fdFile.SetDefaultFileName(".");
						fdFile.ScanDir(prFileDialogInternal, fdFile.puDLGpath);
					}

					// draw dialog parts
					prDrawHeader(); // bookmark, directory, path
					prDrawContent(); // bookmark, files view, side pane 
					res = prDrawFooter(); // file field, filter combobox, ok/cancel buttons

					EndFrame();


				}
				ImGui::EndChild();

				// for display in dialog center, the confirm to overwrite dlg
				prFileDialogInternal.puDialogCenterPos = ImGui::GetCurrentWindowRead()->ContentRegionRect.GetCenter();

				// when the confirm to overwrite dialog will appear we need to 
				// disable the modal mode of the main file dialog
				// see prOkResultToConfirm under
				if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_Modal &&
					!prFileDialogInternal.puOkResultToConfirm)
					ImGui::EndPopup();
			}

			if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_NoDialog) // disable our own dialog system (standard or modal)
			{

			}
			else
			{
				// same things here regarding prOkResultToConfirm
				if (!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_Modal) ||
					prFileDialogInternal.puOkResultToConfirm)
					ImGui::End();
			}
			// confirm the result and show the confirm to overwrite dialog if needed
			res = prConfirm_Or_OpenOverWriteFileDialog_IfNeeded(res, vFlags);

			if (prFileDialogInternal.puUseCustomLocale)
				setlocale(prFileDialogInternal.puLocaleCategory, prFileDialogInternal.puLocaleEnd.c_str());
		}

		return res;
	}

	void IGFD::FileDialog::NewFrame()
	{
		prFileDialogInternal.NewFrame();
		NewThumbnailFrame(prFileDialogInternal);
	}

	void IGFD::FileDialog::EndFrame()
	{
		EndThumbnailFrame(prFileDialogInternal);
		prFileDialogInternal.EndFrame();

	}
	void IGFD::FileDialog::QuitFrame()
	{
		QuitThumbnailFrame(prFileDialogInternal);
	}

	void IGFD::FileDialog::prDrawHeader()
	{
#ifdef USE_BOOKMARK
		if (!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableBookmarkMode))
		{
			prDrawBookmarkButton();
			ImGui::SameLine();
		}

#endif // USE_BOOKMARK

		prFileDialogInternal.puFileManager.DrawDirectoryCreation(prFileDialogInternal);

		if (
#ifdef USE_BOOKMARK
			!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableBookmarkMode) ||
#endif // USE_BOOKMARK
			!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableCreateDirectoryButton))
		{
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
		}
		prFileDialogInternal.puFileManager.DrawPathComposer(prFileDialogInternal);

#ifdef USE_THUMBNAILS
		if (!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableThumbnailMode))
		{
			prDrawDisplayModeToolBar();
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
		}
#endif // USE_THUMBNAILS

		prFileDialogInternal.puSearchManager.DrawSearchBar(prFileDialogInternal);
	}

	void IGFD::FileDialog::prDrawContent()
	{
		ImVec2 size = ImGui::GetContentRegionAvail() - ImVec2(0.0f, prFileDialogInternal.puFooterHeight);

#ifdef USE_BOOKMARK
		if (!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableBookmarkMode))
		{
			if (prBookmarkPaneShown)
			{
				//size.x -= prBookmarkWidth;
				float otherWidth = size.x - prBookmarkWidth;
				ImGui::PushID("##splitterbookmark");
				IGFD::Utils::Splitter(true, 4.0f,
					&prBookmarkWidth, &otherWidth, 10.0f,
					10.0f + prFileDialogInternal.puDLGoptionsPaneWidth, size.y);
				ImGui::PopID();
				size.x -= otherWidth;
				prDrawBookmarkPane(prFileDialogInternal, size);
				ImGui::SameLine();
			}
		}
#endif // USE_BOOKMARK

		size.x = ImGui::GetContentRegionAvail().x - prFileDialogInternal.puDLGoptionsPaneWidth;

		if (prFileDialogInternal.puDLGoptionsPane)
		{
			ImGui::PushID("##splittersidepane");
			IGFD::Utils::Splitter(true, 4.0f, &size.x, &prFileDialogInternal.puDLGoptionsPaneWidth, 10.0f, 10.0f, size.y);
			ImGui::PopID();
		}

#ifdef USE_THUMBNAILS
		if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableThumbnailMode)
		{
			prDrawFileListView(size);
		}
		else
		{
			switch (prDisplayMode)
			{
			case DisplayModeEnum::FILE_LIST:
				prDrawFileListView(size);
				break;
			case DisplayModeEnum::THUMBNAILS_LIST:
				prDrawThumbnailsListView(size);
				break;
			case DisplayModeEnum::THUMBNAILS_GRID:
				prDrawThumbnailsGridView(size);
			}
		}
#else	// USE_THUMBNAILS
		prDrawFileListView(size);
#endif	// USE_THUMBNAILS

		if (prFileDialogInternal.puDLGoptionsPane)
		{
			prDrawSidePane(size.y);
		}

#if defined(USE_QUICK_PATH_SELECT)
		DisplayPathPopup(size);
#endif // USE_QUICK_PATH_SELECT
	}

#if defined(USE_QUICK_PATH_SELECT)
	void IGFD::FileDialog::DisplayPathPopup(ImVec2 vSize)
	{
		ImVec2 size = ImVec2(vSize.x * 0.5f, vSize.y * 0.5f);
		if (ImGui::BeginPopup("IGFD_Path_Popup"))
		{
			auto& fdi = prFileDialogInternal.puFileManager;

			ImGui::PushID(this);

			static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoHostExtendY;
			auto listViewID = ImGui::GetID("##FileDialog_pathTable");
			if (ImGui::BeginTableEx("##FileDialog_pathTable", listViewID, 1, flags, size, 0.0f)) //-V112
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
				ImGui::TableSetupColumn(tableHeaderFileNameString, ImGuiTableColumnFlags_WidthStretch |
					(defaultSortOrderFilename ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending), -1, 0);

				ImGui::TableHeadersRow();

				if (!fdi.IsPathFilteredListEmpty())
				{
					std::string _str;
					ImFont* _font = nullptr;
					bool _showColor = false;

					prPathListClipper.Begin((int)fdi.GetPathFilteredListSize(), ImGui::GetTextLineHeightWithSpacing());
					while (prPathListClipper.Step())
					{
						for (int i = prPathListClipper.DisplayStart; i < prPathListClipper.DisplayEnd; i++)
						{
							if (i < 0) continue;

							auto infos = fdi.GetFilteredPathAt((size_t)i);
							if (!infos.use_count())
								continue;

							prBeginFileColorIconStyle(infos, _showColor, _str, &_font);

							bool selected = fdi.IsFileNameSelected(infos->fileNameExt); // found

							ImGui::TableNextRow();

							if (ImGui::TableNextColumn()) // file name
							{
								if (ImGui::Selectable(infos->fileNameExt.c_str(), &selected,
									ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SpanAvailWidth))
								{
									fdi.SetCurrentPath(fdi.ComposeNewPath(fdi.GetCurrentPopupComposedPath()));
									fdi.puPathClicked = fdi.SelectDirectory(infos);
									ImGui::CloseCurrentPopup();
								}
							}

							prEndFileColorIconStyle(_showColor, _font);
						}
					}
					prPathListClipper.End();
				}

				ImGui::EndTable();
			}

			ImGui::PopID();

			ImGui::EndPopup();
		}
	}
#endif

	bool IGFD::FileDialog::prDrawOkButton()
	{
		auto& fdFile = prFileDialogInternal.puFileManager;
		if (prFileDialogInternal.puCanWeContinue && strlen(fdFile.puFileNameBuffer))
		{
			if (IMGUI_BUTTON(okButtonString "##validationdialog", ImVec2(okButtonWidth, 0.0f)) || prFileDialogInternal.puIsOk)
			{
				prFileDialogInternal.puIsOk = true;
				return true;
			}

#if !invertOkAndCancelButtons
			ImGui::SameLine();
#endif

		}

		return false;
	}

	bool IGFD::FileDialog::prDrawCancelButton()
	{
		if (IMGUI_BUTTON(cancelButtonString "##validationdialog", ImVec2(cancelButtonWidth, 0.0f)) ||
			prFileDialogInternal.puNeedToExitDialog) // dialog exit asked
		{
			prFileDialogInternal.puIsOk = false;
			return true;
		}

#if invertOkAndCancelButtons
		ImGui::SameLine();
#endif

		return false;
	}

	bool IGFD::FileDialog::prDrawValidationButtons()
	{
		bool res = false;

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - prOkCancelButtonWidth) * okCancelButtonAlignement);

		ImGui::BeginGroup();

		if (invertOkAndCancelButtons)
		{
			res |= prDrawCancelButton();
			res |= prDrawOkButton();
		}
		else
		{
			res |= prDrawOkButton();
			res |= prDrawCancelButton();
		}

		ImGui::EndGroup();

		prOkCancelButtonWidth = ImGui::GetItemRectSize().x;

		return res;
	}

	bool IGFD::FileDialog::prDrawFooter()
	{
		auto& fdFile = prFileDialogInternal.puFileManager;

		float posY = ImGui::GetCursorPos().y; // height of last bar calc

		ImGui::AlignTextToFramePadding();

		if (!fdFile.puDLGDirectoryMode)
			ImGui::Text(fileNameString);
		else // directory chooser
			ImGui::Text(dirNameString);

		ImGui::SameLine();

		// Input file fields
		float width = ImGui::GetContentRegionAvail().x;
		if (!fdFile.puDLGDirectoryMode)
		{
			ImGuiContext& g = *GImGui;
			width -= FILTER_COMBO_WIDTH + g.Style.ItemSpacing.x;
		}

		ImGui::PushItemWidth(width);

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

		if (prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_ReadOnlyFileNameField)
		{
			flags |= ImGuiInputTextFlags_ReadOnly;
		}

		if (ImGui::InputText("##FileName", fdFile.puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, flags))
		{
			prFileDialogInternal.puIsOk = true;
		}

		if (ImGui::GetItemID() == ImGui::GetActiveID())
			prFileDialogInternal.puFileInputIsActive = true;
		ImGui::PopItemWidth();

		// combobox of filters
		prFileDialogInternal.puFilterManager.DrawFilterComboBox(prFileDialogInternal);

		bool res = false;

		res = prDrawValidationButtons();

		prFileDialogInternal.puFooterHeight = ImGui::GetCursorPosY() - posY;

		return res;
	}

	void IGFD::FileDialog::prSelectableItem(int vidx, std::shared_ptr<FileInfos> vInfos, bool vSelected, const char* vFmt, ...)
	{
		if (!vInfos.use_count())
			return;

		auto& fdi = prFileDialogInternal.puFileManager;

		static ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick |
			ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SpanAvailWidth;

		va_list args;
		va_start(args, vFmt);
		vsnprintf(fdi.puVariadicBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vFmt, args);
		va_end(args);

		float h = 0.0f;
#ifdef USE_THUMBNAILS
		if (prDisplayMode == DisplayModeEnum::THUMBNAILS_LIST &&
			!(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_DisableThumbnailMode))
		{
			h = DisplayMode_ThumbailsList_ImageHeight;
		}
#endif // USE_THUMBNAILS
#ifdef USE_EXPLORATION_BY_KEYS
		bool flashed = prBeginFlashItem((size_t)vidx);
		bool res = prFlashableSelectable(fdi.puVariadicBuffer, vSelected, selectableFlags,
			flashed, ImVec2(-1.0f, h));
		if (flashed)
			prEndFlashItem();
#else // USE_EXPLORATION_BY_KEYS
		(void)vidx; // remove a warnings ofr unused var

		bool res = ImGui::Selectable(fdi.puVariadicBuffer, vSelected, selectableFlags, ImVec2(-1.0f, h));
#endif // USE_EXPLORATION_BY_KEYS
		if (res)
		{
			if (vInfos->fileType.isDir())
			{
				// nav system, selectable cause open directory or select directory
				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)
				{
					// little fix for get back the mouse behavior in nav system
					if (ImGui::IsMouseDoubleClicked(0)) // 0 -> left mouse button double click
					{
						fdi.puPathClicked = fdi.SelectDirectory(vInfos);
					}
					else if (fdi.puDLGDirectoryMode) // directory chooser
					{
						fdi.SelectFileName(prFileDialogInternal, vInfos);
					}
					else
					{
						fdi.puPathClicked = fdi.SelectDirectory(vInfos);
					}
				}
				else // no nav system => classic behavior
				{
					if (ImGui::IsMouseDoubleClicked(0)) // 0 -> left mouse button double click
					{
						fdi.puPathClicked = fdi.SelectDirectory(vInfos);
					}
					else if (fdi.puDLGDirectoryMode) // directory chooser
					{
						fdi.SelectFileName(prFileDialogInternal, vInfos);
					}
				}
			}
			else
			{
				fdi.SelectFileName(prFileDialogInternal, vInfos);

				if (ImGui::IsMouseDoubleClicked(0))
				{
					prFileDialogInternal.puIsOk = true;
				}
			}
		}
	}

	void IGFD::FileDialog::prBeginFileColorIconStyle(std::shared_ptr<FileInfos> vFileInfos, bool& vOutShowColor, std::string& vOutStr, ImFont** vOutFont)
	{
		vOutStr.clear();
		vOutShowColor = false;

		if (vFileInfos->fileStyle.use_count()) //-V807 //-V522
		{
			vOutShowColor = true;

			*vOutFont = vFileInfos->fileStyle->font;
		}

		if (vOutShowColor && !vFileInfos->fileStyle->icon.empty()) vOutStr = vFileInfos->fileStyle->icon;
		else if (vFileInfos->fileType.isDir()) vOutStr = dirEntryString;
		else if (vFileInfos->fileType.isLinkToUnknown()) vOutStr = linkEntryString;
		else if (vFileInfos->fileType.isFile()) vOutStr = fileEntryString;

		vOutStr += " " + vFileInfos->fileNameExt;

		if (vOutShowColor)
			ImGui::PushStyleColor(ImGuiCol_Text, vFileInfos->fileStyle->color);
		if (*vOutFont)
			ImGui::PushFont(*vOutFont);
	}

	void IGFD::FileDialog::prEndFileColorIconStyle(const bool& vShowColor, ImFont* vFont)
	{
		if (vFont)
			ImGui::PopFont();
		if (vShowColor)
			ImGui::PopStyleColor();
	}

	void IGFD::FileDialog::prDrawFileListView(ImVec2 vSize)
	{
		auto& fdi = prFileDialogInternal.puFileManager;

		ImGui::PushID(this);

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY |
			ImGuiTableFlags_NoHostExtendY
#ifndef USE_CUSTOM_SORTING_ICON
			| ImGuiTableFlags_Sortable
#endif // USE_CUSTOM_SORTING_ICON
			;
		auto listViewID = ImGui::GetID("##FileDialog_fileTable");
		if (ImGui::BeginTableEx("##FileDialog_fileTable", listViewID, 4, flags, vSize, 0.0f)) //-V112
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
			ImGui::TableSetupColumn(fdi.puHeaderFileName.c_str(), ImGuiTableColumnFlags_WidthStretch |
				(defaultSortOrderFilename ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending), -1, 0);
			ImGui::TableSetupColumn(fdi.puHeaderFileType.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderType ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending) |
				((prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_HideColumnType) ? ImGuiTableColumnFlags_DefaultHide : 0), -1, 1);
			ImGui::TableSetupColumn(fdi.puHeaderFileSize.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderSize ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending) |
				((prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_HideColumnSize) ? ImGuiTableColumnFlags_DefaultHide : 0), -1, 2);
			ImGui::TableSetupColumn(fdi.puHeaderFileDate.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderDate ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending) |
				((prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_HideColumnDate) ? ImGuiTableColumnFlags_DefaultHide : 0), -1, 3);

#ifndef USE_CUSTOM_SORTING_ICON
			// Sort our data if sort specs have been changed!
			if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
			{
				if (sorts_specs->SpecsDirty && !fdi.IsFileListEmpty())
				{
					bool direction = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending;

					if (sorts_specs->Specs->ColumnUserID == 0)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_FILENAME;
						fdi.puSortingDirection[0] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else if (sorts_specs->Specs->ColumnUserID == 1)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_TYPE;
						fdi.puSortingDirection[1] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else if (sorts_specs->Specs->ColumnUserID == 2)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_SIZE;
						fdi.puSortingDirection[2] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else //if (sorts_specs->Specs->ColumnUserID == 3) => alwayd true for the moment, to uncomment if we add a fourth column
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_DATE;
						fdi.puSortingDirection[3] = direction;
						fdi.SortFields(prFileDialogInternal);
					}

					sorts_specs->SpecsDirty = false;
				}
			}

			ImGui::TableHeadersRow();
#else // USE_CUSTOM_SORTING_ICON
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			for (int column = 0; column < 4; column++) //-V112
			{
				ImGui::TableSetColumnIndex(column);
				const char* column_name = ImGui::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
				ImGui::PushID(column);
				ImGui::TableHeader(column_name);
				ImGui::PopID();
				if (ImGui::IsItemClicked())
				{
					if (column == 0)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_FILENAME)
							fdi.puSortingDirection[0] = !fdi.puSortingDirection[0];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_FILENAME;

						fdi.SortFields(prFileDialogInternal);
					}
					else if (column == 1)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_TYPE)
							fdi.puSortingDirection[1] = !fdi.puSortingDirection[1];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_TYPE;

						fdi.SortFields(prFileDialogInternal);
					}
					else if (column == 2)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_SIZE)
							fdi.puSortingDirection[2] = !fdi.puSortingDirection[2];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_SIZE;

						fdi.SortFields(prFileDialogInternal);
					}
					else //if (column == 3) => alwayd true for the moment, to uncomment if we add a fourth column
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_DATE)
							fdi.puSortingDirection[3] = !fdi.puSortingDirection[3];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_DATE;

						fdi.SortFields(prFileDialogInternal);
					}
				}
			}
#endif // USE_CUSTOM_SORTING_ICON
			if (!fdi.IsFilteredListEmpty())
			{
				std::string _str;
				ImFont* _font = nullptr;
				bool _showColor = false;

				prFileListClipper.Begin((int)fdi.GetFilteredListSize(), ImGui::GetTextLineHeightWithSpacing());
				while (prFileListClipper.Step())
				{
					for (int i = prFileListClipper.DisplayStart; i < prFileListClipper.DisplayEnd; i++)
					{
						if (i < 0) continue;

						auto infos = fdi.GetFilteredFileAt((size_t)i);
						if (!infos.use_count())
							continue;

						prBeginFileColorIconStyle(infos, _showColor, _str, &_font);

						bool selected = fdi.IsFileNameSelected(infos->fileNameExt); // found

						ImGui::TableNextRow();

						if (ImGui::TableNextColumn()) // file name
						{
							prSelectableItem(i, infos, selected, _str.c_str());
						}
						if (ImGui::TableNextColumn()) // file type
						{
							ImGui::Text("%s", infos->fileExt.c_str());
						}
						if (ImGui::TableNextColumn()) // file size
						{
							if (!infos->fileType.isDir())
							{
								ImGui::Text("%s ", infos->formatedFileSize.c_str());
							}
							else
							{
								ImGui::TextUnformatted("");
							}
						}
						if (ImGui::TableNextColumn()) // file date + time
						{
							ImGui::Text("%s", infos->fileModifDate.c_str());
						}

						prEndFileColorIconStyle(_showColor, _font);
					}
				}
				prFileListClipper.End();
			}

#ifdef USE_EXPLORATION_BY_KEYS
			if (!fdi.puInputPathActivated)
			{
				prLocateByInputKey(prFileDialogInternal);
				prExploreWithkeys(prFileDialogInternal, listViewID);
			}
#endif // USE_EXPLORATION_BY_KEYS

			ImGuiContext& g = *GImGui;
			if (g.LastActiveId - 1 == listViewID || g.LastActiveId == listViewID)
			{
				prFileDialogInternal.puFileListViewIsActive = true;
			}

			ImGui::EndTable();
		}

		ImGui::PopID();
	}

#ifdef USE_THUMBNAILS
	void IGFD::FileDialog::prDrawThumbnailsListView(ImVec2 vSize)
	{
		auto& fdi = prFileDialogInternal.puFileManager;

		ImGui::PushID(this);

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY |
			ImGuiTableFlags_NoHostExtendY
#ifndef USE_CUSTOM_SORTING_ICON
			| ImGuiTableFlags_Sortable
#endif // USE_CUSTOM_SORTING_ICON
			;
		auto listViewID = ImGui::GetID("##FileDialog_fileTable");
		if (ImGui::BeginTableEx("##FileDialog_fileTable", listViewID, 5, flags, vSize, 0.0f))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
			ImGui::TableSetupColumn(fdi.puHeaderFileName.c_str(), ImGuiTableColumnFlags_WidthStretch |
				(defaultSortOrderFilename ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending), -1, 0);
			ImGui::TableSetupColumn(fdi.puHeaderFileType.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderType ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending) |
				((prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_HideColumnType) ? ImGuiTableColumnFlags_DefaultHide : 0), -1, 1);
			ImGui::TableSetupColumn(fdi.puHeaderFileSize.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderSize ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending) |
				((prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_HideColumnSize) ? ImGuiTableColumnFlags_DefaultHide : 0), -1, 2);
			ImGui::TableSetupColumn(fdi.puHeaderFileDate.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderDate ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending) |
				((prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_HideColumnDate) ? ImGuiTableColumnFlags_DefaultHide : 0), -1, 3);
			// not needed to have an option for hide the thumbnails since this is why this view is used
			ImGui::TableSetupColumn(fdi.puHeaderFileThumbnails.c_str(), ImGuiTableColumnFlags_WidthFixed |
				(defaultSortOrderThumbnails ? ImGuiTableColumnFlags_PreferSortAscending : ImGuiTableColumnFlags_PreferSortDescending), -1, 4); //-V112

#ifndef USE_CUSTOM_SORTING_ICON
																																			   // Sort our data if sort specs have been changed!
			if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
			{
				if (sorts_specs->SpecsDirty && !fdi.IsFileListEmpty())
				{
					bool direction = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending;

					if (sorts_specs->Specs->ColumnUserID == 0)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_FILENAME;
						fdi.puSortingDirection[0] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else if (sorts_specs->Specs->ColumnUserID == 1)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_TYPE;
						fdi.puSortingDirection[1] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else if (sorts_specs->Specs->ColumnUserID == 2)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_SIZE;
						fdi.puSortingDirection[2] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else if (sorts_specs->Specs->ColumnUserID == 3)
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_DATE;
						fdi.puSortingDirection[3] = direction;
						fdi.SortFields(prFileDialogInternal);
					}
					else // if (sorts_specs->Specs->ColumnUserID == 4) = > always true for the moment, to uncomment if we add another column
					{
						fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_THUMBNAILS;
						fdi.puSortingDirection[4] = direction;
						fdi.SortFields(prFileDialogInternal);
					}

					sorts_specs->SpecsDirty = false;
				}
			}

			ImGui::TableHeadersRow();
#else // USE_CUSTOM_SORTING_ICON
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			for (int column = 0; column < 5; column++)
			{
				ImGui::TableSetColumnIndex(column);
				const char* column_name = ImGui::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
				ImGui::PushID(column);
				ImGui::TableHeader(column_name);
				ImGui::PopID();
				if (ImGui::IsItemClicked())
				{
					if (column == 0)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_FILENAME)
							fdi.puSortingDirection[0] = !fdi.puSortingDirection[0];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_FILENAME;

						fdi.SortFields(prFileDialogInternal);
					}
					else if (column == 1)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_TYPE)
							fdi.puSortingDirection[1] = !fdi.puSortingDirection[1];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_TYPE;

						fdi.SortFields(prFileDialogInternal);
					}
					else if (column == 2)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_SIZE)
							fdi.puSortingDirection[2] = !fdi.puSortingDirection[2];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_SIZE;

						fdi.SortFields(prFileDialogInternal);
					}
					else if (column == 3)
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_DATE)
							fdi.puSortingDirection[3] = !fdi.puSortingDirection[3];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_DATE;

						fdi.SortFields(prFileDialogInternal);
					}
					else // if (sorts_specs->Specs->ColumnUserID == 4) = > always true for the moment, to uncomment if we add another column
					{
						if (fdi.puSortingField == IGFD::FileManager::SortingFieldEnum::FIELD_THUMBNAILS)
							fdi.puSortingDirection[4] = !fdi.puSortingDirection[4];
						else
							fdi.puSortingField = IGFD::FileManager::SortingFieldEnum::FIELD_THUMBNAILS;

						fdi.SortFields(prFileDialogInternal);
					}
				}
			}
#endif // USE_CUSTOM_SORTING_ICON
			if (!fdi.IsFilteredListEmpty())
			{
				std::string _str;
				ImFont* _font = nullptr;
				bool _showColor = false;

				ImGuiContext& g = *GImGui;
				const float itemHeight = ImMax(g.FontSize, DisplayMode_ThumbailsList_ImageHeight) + g.Style.ItemSpacing.y;

				prFileListClipper.Begin((int)fdi.GetFilteredListSize(), itemHeight);
				while (prFileListClipper.Step())
				{
					for (int i = prFileListClipper.DisplayStart; i < prFileListClipper.DisplayEnd; i++)
					{
						if (i < 0) continue;

						auto infos = fdi.GetFilteredFileAt((size_t)i);
						if (!infos.use_count())
							continue;

						prBeginFileColorIconStyle(infos, _showColor, _str, &_font);

						bool selected = fdi.IsFileNameSelected(infos->fileNameExt); // found

						ImGui::TableNextRow();

						if (ImGui::TableNextColumn()) // file name
						{
							prSelectableItem(i, infos, selected, _str.c_str());
						}
						if (ImGui::TableNextColumn()) // file type
						{
							ImGui::Text("%s", infos->fileExt.c_str());
						}
						if (ImGui::TableNextColumn()) // file size
						{
							if (!infos->fileType.isDir())
							{
								ImGui::Text("%s ", infos->formatedFileSize.c_str());
							}
							else
							{
								ImGui::TextUnformatted("");
							}
						}
						if (ImGui::TableNextColumn()) // file date + time
						{
							ImGui::Text("%s", infos->fileModifDate.c_str());
						}
						if (ImGui::TableNextColumn()) // file thumbnails
						{
							auto th = &infos->thumbnailInfo;

							if (!th->isLoadingOrLoaded)
							{
								prAddThumbnailToLoad(infos);
							}
							if (th->isReadyToDisplay &&
								th->textureID)
							{
								ImGui::Image((ImTextureID)th->textureID,
									ImVec2((float)th->textureWidth,
										(float)th->textureHeight));
							}
						}

						prEndFileColorIconStyle(_showColor, _font);
					}
				}
				prFileListClipper.End();
			}

#ifdef USE_EXPLORATION_BY_KEYS
			if (!fdi.puInputPathActivated)
			{
				prLocateByInputKey(prFileDialogInternal);
				prExploreWithkeys(prFileDialogInternal, listViewID);
			}
#endif // USE_EXPLORATION_BY_KEYS

			ImGuiContext& g = *GImGui;
			if (g.LastActiveId - 1 == listViewID || g.LastActiveId == listViewID)
			{
				prFileDialogInternal.puFileListViewIsActive = true;
			}

			ImGui::EndTable();
		}

		ImGui::PopID();
	}

	void IGFD::FileDialog::prDrawThumbnailsGridView(ImVec2 vSize)
	{
		if (ImGui::BeginChild("##thumbnailsGridsFiles", vSize))
		{
			// todo
		}

		ImGui::EndChild();
	}

#endif

	void IGFD::FileDialog::prDrawSidePane(float vHeight)
	{
		ImGui::SameLine();

		ImGui::BeginChild("##FileTypes", ImVec2(0, vHeight));

		prFileDialogInternal.puDLGoptionsPane(
			prFileDialogInternal.puFilterManager.GetSelectedFilter().filter.c_str(), 
			prFileDialogInternal.puDLGuserDatas, &prFileDialogInternal.puCanWeContinue);

		ImGui::EndChild();
	}

	void IGFD::FileDialog::Close()
	{
		prFileDialogInternal.puDLGkey.clear();
		prFileDialogInternal.puShowDialog = false;
	}

	bool IGFD::FileDialog::WasOpenedThisFrame(const std::string& vKey) const
	{
		bool res = prFileDialogInternal.puShowDialog && prFileDialogInternal.puDLGkey == vKey;
		if (res)
		{
			ImGuiContext& g = *GImGui;
			res &= prFileDialogInternal.puLastImGuiFrameCount == g.FrameCount; // return true if a dialog was displayed in this frame
		}
		return res;
	}

	bool IGFD::FileDialog::WasOpenedThisFrame() const
	{
		bool res = prFileDialogInternal.puShowDialog;
		if (res)
		{
			ImGuiContext& g = *GImGui;
			res &= prFileDialogInternal.puLastImGuiFrameCount == g.FrameCount; // return true if a dialog was displayed in this frame
		}
		return res;
	}

	bool IGFD::FileDialog::IsOpened(const std::string& vKey) const
	{
		return (prFileDialogInternal.puShowDialog && prFileDialogInternal.puDLGkey == vKey);
	}

	bool IGFD::FileDialog::IsOpened() const
	{
		return prFileDialogInternal.puShowDialog;
	}

	std::string IGFD::FileDialog::GetOpenedKey() const
	{
		if (prFileDialogInternal.puShowDialog)
			return prFileDialogInternal.puDLGkey;
		return "";
	}

	std::string IGFD::FileDialog::GetFilePathName()
	{
		return prFileDialogInternal.puFileManager.GetResultingFilePathName(prFileDialogInternal);
	}

	std::string IGFD::FileDialog::GetCurrentPath()
	{
		return prFileDialogInternal.puFileManager.GetResultingPath();
	}

	std::string IGFD::FileDialog::GetCurrentFileName()
	{
		return prFileDialogInternal.puFileManager.GetResultingFileName(prFileDialogInternal);
	}

	std::string IGFD::FileDialog::GetCurrentFilter()
	{
		return prFileDialogInternal.puFilterManager.GetSelectedFilter().filter;
	}

	std::map<std::string, std::string> IGFD::FileDialog::GetSelection()
	{
		return prFileDialogInternal.puFileManager.GetResultingSelection();
	}

	UserDatas IGFD::FileDialog::GetUserDatas() const
	{
		return prFileDialogInternal.puDLGuserDatas;
	}

	bool IGFD::FileDialog::IsOk() const
	{
		return prFileDialogInternal.puIsOk;
	}

	void IGFD::FileDialog::SetFileStyle(const IGFD_FileStyleFlags& vFlags, const char* vCriteria, const FileStyle& vInfos)
	{
		prFileDialogInternal.puFilterManager.SetFileStyle(vFlags, vCriteria, vInfos);
	}

	void IGFD::FileDialog::SetFileStyle(const IGFD_FileStyleFlags& vFlags, const char* vCriteria, const ImVec4& vColor, const std::string& vIcon, ImFont* vFont)
	{
		prFileDialogInternal.puFilterManager.SetFileStyle(vFlags, vCriteria, vColor, vIcon, vFont);
	}

	bool IGFD::FileDialog::GetFileStyle(const IGFD_FileStyleFlags& vFlags, const std::string& vCriteria, ImVec4* vOutColor, std::string* vOutIcon, ImFont **vOutFont)
	{
		return prFileDialogInternal.puFilterManager.GetFileStyle(vFlags, vCriteria, vOutColor, vOutIcon, vOutFont);
	}

	void IGFD::FileDialog::ClearFilesStyle()
	{
		prFileDialogInternal.puFilterManager.ClearFilesStyle();
	}

	void IGFD::FileDialog::SetLocales(const int& /*vLocaleCategory*/, const std::string& vLocaleBegin, const std::string& vLocaleEnd)
	{
		prFileDialogInternal.puUseCustomLocale = true;
		prFileDialogInternal.puLocaleBegin = vLocaleBegin;
		prFileDialogInternal.puLocaleEnd = vLocaleEnd;
	}

	//////////////////////////////////////////////////////////////////////////////
	//// OVERWRITE DIALOG ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	bool IGFD::FileDialog::prConfirm_Or_OpenOverWriteFileDialog_IfNeeded(bool vLastAction, ImGuiWindowFlags vFlags)
	{
		// if confirmation => return true for confirm the overwrite et quit the dialog
		// if cancel => return false && set IsOk to false for keep inside the dialog

		// if IsOk == false => return false for quit the dialog
		if (!prFileDialogInternal.puIsOk && vLastAction)
		{
			QuitFrame();
			return true;
		}

		// if IsOk == true && no check of overwrite => return true for confirm the dialog
		if (prFileDialogInternal.puIsOk && vLastAction && !(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_ConfirmOverwrite))
		{
			QuitFrame();
			return true;
		}

		// if IsOk == true && check of overwrite => return false and show confirm to overwrite dialog
		if ((prFileDialogInternal.puOkResultToConfirm || (prFileDialogInternal.puIsOk && vLastAction)) && 
			(prFileDialogInternal.puDLGflags & ImGuiFileDialogFlags_ConfirmOverwrite))
		{
			if (prFileDialogInternal.puIsOk) // catched only one time
			{
				if (!prFileDialogInternal.puFileManager.IsFileExist(GetFilePathName())) // not existing => quit dialog
				{
					QuitFrame();
					return true;
				}
				else // existing => confirm dialog to open
				{
					prFileDialogInternal.puIsOk = false;
					prFileDialogInternal.puOkResultToConfirm = true;
				}
			}

			std::string name = OverWriteDialogTitleString "##" + prFileDialogInternal.puDLGtitle + prFileDialogInternal.puDLGkey + "OverWriteDialog";

			bool res = false;

			ImGui::OpenPopup(name.c_str());
			if (ImGui::BeginPopupModal(name.c_str(), (bool*)0,
				vFlags | ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				ImGui::SetWindowPos(prFileDialogInternal.puDialogCenterPos - ImGui::GetWindowSize() * 0.5f); // next frame needed for GetWindowSize to work

				ImGui::Text("%s", OverWriteDialogMessageString);

				if (IMGUI_BUTTON(OverWriteDialogConfirmButtonString))
				{
					prFileDialogInternal.puOkResultToConfirm = false;
					prFileDialogInternal.puIsOk = true;
					res = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (IMGUI_BUTTON(OverWriteDialogCancelButtonString))
				{
					prFileDialogInternal.puOkResultToConfirm = false;
					prFileDialogInternal.puIsOk = false;
					res = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (res)
			{
				QuitFrame();
			}
			return res;
		}

		return false;
	}
}

#endif // __cplusplus

/////////////////////////////////////////////////////////////////
///// C Interface ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Return an initialized IGFD_Selection_Pair
IMGUIFILEDIALOG_API IGFD_Selection_Pair IGFD_Selection_Pair_Get(void)
{
	IGFD_Selection_Pair res = {};
	res.fileName = nullptr;
	res.filePathName = nullptr;
	return res;
}

// destroy only the content of vSelection_Pair
IMGUIFILEDIALOG_API void IGFD_Selection_Pair_DestroyContent(IGFD_Selection_Pair* vSelection_Pair)
{
	if (vSelection_Pair)
	{
		delete[] vSelection_Pair->fileName;
		delete[] vSelection_Pair->filePathName;
	}
}

// Return an initialized IGFD_Selection
IMGUIFILEDIALOG_API IGFD_Selection IGFD_Selection_Get(void)
{
	return { nullptr, 0U };
}

// destroy only the content of vSelection
IMGUIFILEDIALOG_API void IGFD_Selection_DestroyContent(IGFD_Selection* vSelection)
{
	if (vSelection)
	{
		if (vSelection->table)
		{
			for (size_t i = 0U; i < vSelection->count; i++)
			{
				IGFD_Selection_Pair_DestroyContent(&vSelection->table[i]);
			}
			delete[] vSelection->table;
		}
		vSelection->count = 0U;
	}
}

// create an instance of ImGuiFileDialog
IMGUIFILEDIALOG_API ImGuiFileDialog* IGFD_Create(void)
{
	return new ImGuiFileDialog();
}

// destroy the instance of ImGuiFileDialog
IMGUIFILEDIALOG_API void IGFD_Destroy(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		delete vContext;
		vContext = nullptr;
	}
}

// standard dialog
IMGUIFILEDIALOG_API void IGFD_OpenDialog(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vPath,
	const char* vFileName,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters, vPath, vFileName,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenDialog2(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vFilePathName,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters, vFilePathName,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenPaneDialog(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vPath,
	const char* vFileName,
	IGFD_PaneFun vSidePane,
	const float vSidePaneWidth,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters,
			vPath, vFileName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenPaneDialog2(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vFilePathName,
	IGFD_PaneFun vSidePane,
	const float vSidePaneWidth,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters,
			vFilePathName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API bool IGFD_DisplayDialog(ImGuiFileDialog* vContext,
	const char* vKey, ImGuiWindowFlags vFlags, ImVec2 vMinSize, ImVec2 vMaxSize)
{
	if (vContext)
	{
		return vContext->Display(vKey, vFlags, vMinSize, vMaxSize);
	}

	return false;
}

IMGUIFILEDIALOG_API void IGFD_CloseDialog(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->Close();
	}
}

IMGUIFILEDIALOG_API bool IGFD_IsOk(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		return vContext->IsOk();
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_WasKeyOpenedThisFrame(ImGuiFileDialog* vContext,
	const char* vKey)
{
	if (vContext)
	{
		return vContext->WasOpenedThisFrame(vKey);
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_WasOpenedThisFrame(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		return vContext->WasOpenedThisFrame();
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_IsKeyOpened(ImGuiFileDialog* vContext,
	const char* vCurrentOpenedKey)
{
	if (vContext)
	{
		return vContext->IsOpened(vCurrentOpenedKey);
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_IsOpened(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		return vContext->IsOpened();
	}

	return false;
}

IMGUIFILEDIALOG_API IGFD_Selection IGFD_GetSelection(ImGuiFileDialog* vContext)
{
	IGFD_Selection res = IGFD_Selection_Get();

	if (vContext)
	{
		auto sel = vContext->GetSelection();
		if (!sel.empty())
		{
			res.count = sel.size();
			res.table = new IGFD_Selection_Pair[res.count];

			size_t idx = 0U;
			for (const auto& s : sel)
			{
				IGFD_Selection_Pair* pair = res.table + idx++;

				// fileNameExt
				if (!s.first.empty())
				{
					size_t siz = s.first.size() + 1U;
					pair->fileName = new char[siz];
#ifndef _MSC_VER
					strncpy(pair->fileName, s.first.c_str(), siz);
#else // _MSC_VER
					strncpy_s(pair->fileName, siz, s.first.c_str(), siz);
#endif // _MSC_VER
					pair->fileName[siz - 1U] = '\0';
				}

				// filePathName
				if (!s.second.empty())
				{
					size_t siz = s.first.size() + 1U;
					pair->filePathName = new char[siz];
#ifndef _MSC_VER
					strncpy(pair->filePathName, s.first.c_str(), siz);
#else // _MSC_VER
					strncpy_s(pair->filePathName, siz, s.first.c_str(), siz);
#endif // _MSC_VER
					pair->filePathName[siz - 1U] = '\0';
				}
			}

			return res;
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetFilePathName(ImGuiFileDialog* vContext)
{
	char* res = nullptr;

	if (vContext)
	{
		auto s = vContext->GetFilePathName();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = (char*)malloc(siz);
			if (res)
			{
#ifndef _MSC_VER
				strncpy(res, s.c_str(), siz);
#else // _MSC_VER
				strncpy_s(res, siz, s.c_str(), siz);
#endif // _MSC_VER
				res[siz - 1U] = '\0';
			}
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetCurrentFileName(ImGuiFileDialog* vContext)
{
	char* res = nullptr;

	if (vContext)
	{
		auto s = vContext->GetCurrentFileName();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = (char*)malloc(siz);
			if (res)
			{
#ifndef _MSC_VER
				strncpy(res, s.c_str(), siz);
#else // _MSC_VER
				strncpy_s(res, siz, s.c_str(), siz);
#endif // _MSC_VER
				res[siz - 1U] = '\0';
			}
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetCurrentPath(ImGuiFileDialog* vContext)
{
	char* res = nullptr;

	if (vContext)
	{
		auto s = vContext->GetCurrentPath();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = (char*)malloc(siz);
			if (res)
			{
#ifndef _MSC_VER
				strncpy(res, s.c_str(), siz);
#else // _MSC_VER
				strncpy_s(res, siz, s.c_str(), siz);
#endif // _MSC_VER
				res[siz - 1U] = '\0';
			}
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetCurrentFilter(ImGuiFileDialog* vContext)
{
	char* res = nullptr;

	if (vContext)
	{
		auto s = vContext->GetCurrentFilter();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = (char*)malloc(siz);
			if (res)
			{
#ifndef _MSC_VER
				strncpy(res, s.c_str(), siz);
#else // _MSC_VER
				strncpy_s(res, siz, s.c_str(), siz);
#endif // _MSC_VER
				res[siz - 1U] = '\0';
			}
		}
	}

	return res;
}

IMGUIFILEDIALOG_API void* IGFD_GetUserDatas(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		return vContext->GetUserDatas();
	}

	return nullptr;
}

IMGUIFILEDIALOG_API void IGFD_SetFileStyle(ImGuiFileDialog* vContext,
	IGFD_FileStyleFlags vFlags, const char* vCriteria, ImVec4 vColor, const char* vIcon, ImFont* vFont) //-V813
{
	if (vContext)
	{
		vContext->SetFileStyle(vFlags, vCriteria, vColor, vIcon, vFont);
	}
}

IMGUIFILEDIALOG_API void IGFD_SetFileStyle2(ImGuiFileDialog* vContext,
	IGFD_FileStyleFlags vFlags, const char* vCriteria, float vR, float vG, float vB, float vA, const char* vIcon, ImFont* vFont)
{
	if (vContext)
	{
		vContext->SetFileStyle(vFlags, vCriteria, ImVec4(vR, vG, vB, vA), vIcon, vFont);
	}
}

IMGUIFILEDIALOG_API bool IGFD_GetFileStyle(ImGuiFileDialog* vContext,
	IGFD_FileStyleFlags vFlags, const char* vCriteria, ImVec4* vOutColor, char** vOutIconText, ImFont** vOutFont)
{
	if (vContext)
	{
		std::string icon;
		bool res = vContext->GetFileStyle(vFlags, vCriteria, vOutColor, &icon, vOutFont);
		if (!icon.empty() && vOutIconText)
		{
			size_t siz = icon.size() + 1U;
			*vOutIconText = (char*)malloc(siz);
			if (*vOutIconText)
			{
#ifndef _MSC_VER
				strncpy(*vOutIconText, icon.c_str(), siz);
#else // _MSC_VER
				strncpy_s(*vOutIconText, siz, icon.c_str(), siz);
#endif // _MSC_VER
				(*vOutIconText)[siz - 1U] = '\0';
			}
		}
		return res;
	}

	return false;
}

IMGUIFILEDIALOG_API void IGFD_ClearFilesStyle(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->ClearFilesStyle();
	}
}

IMGUIFILEDIALOG_API void SetLocales(ImGuiFileDialog* vContext, const int vCategory, const char* vBeginLocale, const char* vEndLocale)
{
	if (vContext)
	{
		vContext->SetLocales(vCategory, (vBeginLocale ? vBeginLocale : ""), (vEndLocale ? vEndLocale : ""));
	}
}

#ifdef USE_EXPLORATION_BY_KEYS
IMGUIFILEDIALOG_API void IGFD_SetFlashingAttenuationInSeconds(ImGuiFileDialog* vContext, float vAttenValue)
{
	if (vContext)
	{
		vContext->SetFlashingAttenuationInSeconds(vAttenValue);
	}
}
#endif

#ifdef USE_BOOKMARK
IMGUIFILEDIALOG_API char* IGFD_SerializeBookmarks(ImGuiFileDialog* vContext, bool vDontSerializeCodeBasedBookmarks)
{
	char* res = nullptr;

	if (vContext)
	{
		auto s = vContext->SerializeBookmarks(vDontSerializeCodeBasedBookmarks);
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = (char*)malloc(siz);
			if (res)
			{
#ifndef _MSC_VER
				strncpy(res, s.c_str(), siz);
#else // _MSC_VER
				strncpy_s(res, siz, s.c_str(), siz);
#endif // _MSC_VER
				res[siz - 1U] = '\0';
			}
		}
	}

	return res;
}

IMGUIFILEDIALOG_API void IGFD_DeserializeBookmarks(ImGuiFileDialog* vContext, const char* vBookmarks)
{
	if (vContext)
	{
		vContext->DeserializeBookmarks(vBookmarks);
	}
}

IMGUIFILEDIALOG_API void IGFD_AddBookmark(ImGuiFileDialog* vContext, const char* vBookMarkName, const char* vBookMarkPath)
{
	if (vContext)
	{
		vContext->AddBookmark(vBookMarkName, vBookMarkPath);
	}
}

IMGUIFILEDIALOG_API void IGFD_RemoveBookmark(ImGuiFileDialog* vContext, const char* vBookMarkName)
{
	if (vContext)
	{
		vContext->RemoveBookmark(vBookMarkName);
	}
}

#endif

#ifdef USE_THUMBNAILS
IMGUIFILEDIALOG_API void SetCreateThumbnailCallback(ImGuiFileDialog* vContext, const IGFD_CreateThumbnailFun vCreateThumbnailFun)
{
	if (vContext)
	{
		vContext->SetCreateThumbnailCallback(vCreateThumbnailFun);
	}
}

IMGUIFILEDIALOG_API void SetDestroyThumbnailCallback(ImGuiFileDialog* vContext, const IGFD_DestroyThumbnailFun vDestroyThumbnailFun)
{
	if (vContext)
	{
		vContext->SetDestroyThumbnailCallback(vDestroyThumbnailFun);
	}
}

IMGUIFILEDIALOG_API void ManageGPUThumbnails(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->ManageGPUThumbnails();
	}
}
#endif // USE_THUMBNAILS
