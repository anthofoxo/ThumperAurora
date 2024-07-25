#ifdef _WIN32
#	include <Windows.h> // Suppress: warning C4005: 'APIENTRY': macro redefinition
#endif

#include <GLFW/glfw3.h>

#include <tinyfiledialogs.h>
#include <imgui_impl_glfw.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3_loader.h>
#include <imgui.h>
#include <lua.hpp>

#include "hashtable.hpp"

#include <any>
#include <algorithm>
#include <array>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <unordered_map>
#include <cstdint>

#include <TextEditor.h>

#include <imgui_memory_editor.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define SystemOpenURL(url) system("start " url);
#elif __APPLE__
#define SystemOpenURL(url) system("open " url);
#elif __linux__
#define SystemOpenURL(url) system("xdg-open" url);
#else
#error "Unknown compiler"
#endif


uint32_t hash32(unsigned char const* array, unsigned int size) {
	uint32_t h = 0x811c9dc5;

	while (size > 0) {
		size--;
		h = (h ^ *array) * 0x1000193;
		array++;
	}

	h *= 0x2001;
	h = (h ^ (h >> 0x7)) * 0x9;
	h = (h ^ (h >> 0x11)) * 0x21;

	return h;
}

std::optional<std::vector<char>> readFile(char const* path) {
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (!file) return std::nullopt;

	std::vector<char> content;
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	content.resize(size);
	file.seekg(0, std::ios::beg);
	file.read(content.data(), content.size());
	file.close();
	return content;
}

uint32_t readUint32(char** ptr) {
	uint32_t value;
	memcpy(&value, *ptr, sizeof(uint32_t));
	*ptr += sizeof(uint32_t);
	return value;
}

char readByte(char** ptr) {
	char value;
	memcpy(&value, *ptr, sizeof(char));
	*ptr += sizeof(char);
	return value;
}

std::string readString(char** ptr) {
	uint32_t length = readUint32(ptr);
	std::string string;
	string.resize(length);
	memcpy(string.data(), *ptr, length);
	*ptr += length;
	return string;
}

enum struct FileType : uint32_t {
	kObjlib     = 0x00000008,
	kDdsTexture = 0x0000000e
};

enum struct ObjType : uint32_t {
	kAnim = 0x5232f8f9,
	kBend = 0x7dd6b7d8,
	kBind = 0x570e17fa,
	kCam = 0x8f86650f,
	kCh = 0xadb02913,
	kCond = 0x4945e860,
	kDch = 0xac1abb2c,
	kDec = 0x9ce604da,
	kDsp = 0xacc2033e,
	kEnt = 0xeae6beee,
	kEnv = 0x3bbcc4ec,
	kFlow = 0x86621b1e,
	kFlt_0 = 0x6222e06f,
	kFlt_1 = 0x993811f5,
	kGameplay = 0xc2fd0a11,
	kGate = 0xaa63a508,
	kGrp = 0xc2aaec43,
	kLeaf = 0xce7e85f6,
	kLight = 0x711a2715,
	kLvl = 0xbcd17473,
	kMaster = 0x490780b9,
	kMastering = 0x1a5812f6,
	kMat = 0x7ba5c8e0,
	kMesh = 0xbf69f115,
	kObjlibGfx = 0x1ba51443,
	kObjlibSequin = 0xb0954548,
	kObjlibObj = 0x9d1c6219,
	kObjlibLevel = 0x0b374d9e,
	kObjlibAvatar = 0xe674624f,
	kPath = 0x4890a3f6,
	kPlayspace = 0x745dd78b,
	kPulse = 0x230da622,
	kSamp = 0x7aa8f390,
	kSDraw_Drawer = 0xd3058b5d,
	kSh = 0xcac934cf,
	kSpn = 0xd897d5db,
	kSt = 0xd955fdc6,
	kSteer = 0xe7b3aadb,
	kTex = 0x96ba8a70,
	kVib = 0x799c45a7,
	kVrSettings = 0x4f37349d,
	kXfm_Xfmer = 0x7d9db5ef
};

struct ObjlibHeader {
	FileType fileType;
	ObjType objType;
	uint32_t unknown0;
	uint32_t unknown1;
	uint32_t unknown2;
};

struct LibraryImport {
	uint32_t unknown0;
	std::string string;
};

struct ObjectImport {
	ObjType type;
	std::string objName;
	uint32_t unknown0;
	std::string libraryName;
};

struct Object {
	ObjType type;
	std::string name;
};

struct Objlib {
	std::vector<char> raw;
	size_t headerDefOffset; // Offset into raw data the object definitions start

	ObjlibHeader header;
	std::string originalName;
	std::vector<LibraryImport> libraryImports;
	std::vector<ObjectImport> objectImports;
	std::vector<Object> objects;
};

enum struct TraitType : uint32_t {
	kTraitInt = 0,
	kTraitBool,
	kTraitFloat,
	kTraitColor,
	kTraitObj,
	kTraitVec3,
	kTraitPath,
	kTraitEnum,
	kTraitAction,
	kTraitObjVec,
	kTraitString,
	kTraitCue,
	kTraitEvent,
	kTraitSym,
	kTraitList,
	kTraitTraitPath,
	kTraitQuat,
	kTraitChildLib,
	kTraitComponent,

	kNumTraitTypes,
};

std::array<std::string_view, 20> kTraitTypes = {
	
};

int failedCount = 0;

std::optional<Objlib> readObjlib(char const* file) {
	Objlib lib;

	lib.raw = readFile(file).value();
	char* const baseaddr = lib.raw.data();
	char* ptr = lib.raw.data();


	memcpy(&lib.header, ptr, sizeof(ObjlibHeader));
	ptr += sizeof(ObjlibHeader);

	if (lib.header.fileType != FileType::kObjlib)
		return std::nullopt;

	if (lib.header.objType == ObjType::kObjlibObj) {
		++failedCount;
		return std::nullopt;
	}

	if (lib.header.objType == ObjType::kObjlibGfx) {
		// nothing here?
	}

	// Not very sure what this value is
	if (lib.header.objType == ObjType::kObjlibLevel) {
		uint32_t unknown;
		memcpy(&unknown, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
	}

	// Not very sure what this value is
	if (lib.header.objType == ObjType::kObjlibAvatar) {
		uint32_t unknown;
		memcpy(&unknown, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
	}

	// Not very sure what this value is
	if (lib.header.objType == ObjType::kObjlibSequin) {
		uint32_t unknown;
		memcpy(&unknown, ptr, sizeof(uint32_t));
		ptr += sizeof(uint32_t);
	}

	uint32_t libraryImportCount = readUint32(&ptr);
	lib.libraryImports.reserve(libraryImportCount);

	for (uint32_t i = 0; i < libraryImportCount; ++i) {
		LibraryImport o;
		o.unknown0 = readUint32(&ptr);
		o.string = readString(&ptr);
		lib.libraryImports.emplace_back(o);
	}

	lib.originalName = readString(&ptr);

	uint32_t objectImportCount = readUint32(&ptr);
	lib.objectImports.reserve(objectImportCount);

	for (uint32_t i = 0; i < objectImportCount; ++i) {
		ObjectImport o;
		o.type = static_cast<ObjType>(readUint32(&ptr));
		o.objName = readString(&ptr);
		o.unknown0 = readUint32(&ptr);
		o.libraryName = readString(&ptr);
		lib.objectImports.push_back(o);
	}

	uint32_t objectCount = readUint32(&ptr);
	lib.objects.reserve(objectCount);

	for (uint32_t i = 0; i < objectCount; ++i) {
		Object o;
		o.type = static_cast<ObjType>(readUint32(&ptr));
		o.name = readString(&ptr);
		lib.objects.push_back(o);
	}

	lib.headerDefOffset = (uintptr_t)(ptr - baseaddr);

	return lib;
}

std::unordered_map<std::string, Objlib> kMap;

std::string kCacheDir;


std::string filter;

void loadObjLibs() {
	for (auto const& entry : std::filesystem::directory_iterator(kCacheDir)) {
		std::string file = entry.path().filename().string();

		if (entry.path().extension() != ".pc") continue;

		std::optional<Objlib> lib = readObjlib(entry.path().string().c_str());
		if (lib.has_value())
			kMap[entry.path().stem().string()] = lib.value();
	}
}

void dumpHashes() {
	std::ofstream file;
	file.open("hash_dump.lua");
	file << "local table = {}\n";

	for (auto const& [k, v] : kMap) {
		file << "table[0x" << k << "] = \"A";
		file << v.originalName;
		file << "\"\n";
	}

	file << "return table";
	file.close();
}

void loadConfig() {
	bool hasStoredCachePath = false;

	if (std::filesystem::exists("config.lua")) {
		lua_State* L = luaL_newstate();
		luaL_dofile(L, "config.lua");
		lua_getglobal(L, "cachePath");
		if (lua_isstring(L, -1))
			kCacheDir = lua_tostring(L, -1);
		lua_pop(L, 1);
		lua_close(L);
		hasStoredCachePath = true;
	}

	if(!hasStoredCachePath) {
		char const* selection = tinyfd_openFileDialog("Select Thumper executable", nullptr, 0, nullptr, nullptr, 0);
		if (selection == nullptr) return std::exit(0);
		kCacheDir = (std::filesystem::path(selection).parent_path() / "cache").string();
		std::replace(kCacheDir.begin(), kCacheDir.end(), '\\', '/');

		std::ofstream file;
		file.open("config.lua");
		file << "cachePath = \"" << kCacheDir << "\"";
		file.close();
	}
}

size_t split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
	size_t pos = txt.find(ch);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}

TextEditor editor;
lua_State* L = nullptr;
lua_State* T = nullptr;

void hookfunc(lua_State* L, lua_Debug* ar)
{
	lua_getinfo(L, "l", ar); // Get ar->currentline
	//printf("Executing line: %d\n", ar->currentline);

	TextEditor::ErrorMarkers marker;
	marker.insert(std::make_pair(ar->currentline, "Current line"));
	editor.SetErrorMarkers(marker);

	lua_yield(L, 0); // Yield, only works when using `lua_resume` and with 0 return values
}

static void dumpstack(lua_State* L) {
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++) {
		printf("%d\t%s\t", i, luaL_typename(L, i));
		switch (lua_type(L, i)) {
		case LUA_TNUMBER:
			printf("%g\n", lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			printf("%s\n", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			printf("%s\n", "nil");
			break;
		default:
			printf("%p\n", lua_topointer(L, i));
			break;
		}
	}
}

static Objlib* selection = nullptr;

char leafHeader[16] = { 0x22, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,  0x04,0x00, 0x00, 0x00,  0x02, 0x00, 0x00, 0x00, };

int main(int, char* []) {


	loadConfig();

	loadObjLibs();

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Aurora", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	bool viewHasher = false;
	bool viewBpms = false;

	bool showImguiDemo = false;

	MemoryEditor memedit;
	
	editor.SetText("function doPrint()\n\tprint(\"Aurora\")\nend\n\nfor i = 0, 10, 1 do\n\tdoPrint()\nend");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Quit", "Alt+F4", nullptr))
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools")) {
				ImGui::MenuItem("Hasher", nullptr, &viewHasher);
				ImGui::MenuItem("Level BPMs", nullptr, &viewBpms);
				if (ImGui::MenuItem("Dump hashes", nullptr, nullptr))
					dumpHashes();

				if (ImGui::MenuItem("Thumper File and Object Format Specification", nullptr, nullptr)) {
					SystemOpenURL("https://thumper.tiddlyhost.com/");
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem("ImGuiDemo", nullptr, &showImguiDemo);
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		{
			editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
			if (ImGui::Begin("Script")) {
		
		
				if (ImGui::Button("Run")) {
					L = luaL_newstate();
					luaL_openlibs(L);
		
					T = lua_newthread(L);
					lua_sethook(T, hookfunc, LUA_MASKCOUNT, 1);
		
					if (luaL_loadstring(T, editor.GetText().c_str()) != LUA_OK) {
						std::cerr << lua_tostring(L, -1) << '\n';
						__debugbreak();
					}
		
				}
		
				if (L != nullptr) {
		
					static int ct = 0;
					ct++;
		
					if (ct == 1) {
						ct -= 1;
		
		
						int status;
		
						//do
						//{
						int nres; // Number of return values, should be 0 when using inside hooks
						status = lua_resume(T, 0, 0, &nres); // Start/resume thread
						// The thread may be yielded, completed or have an error, but you get full control back in c, after every line of lua execution
		
						// Done!
						if (status != LUA_YIELD) {
							lua_close(L);
							T = nullptr;
							L = nullptr;
						}
		
					//} while (status == LUA_YIELD);
		
					}
		
				}
		
				editor.Render("Script");
			}
			
			ImGui::End();
		}

		static char* objlibOrigin = nullptr;
		static char* leafOffset = nullptr;
		
		{
			if (ImGui::Begin("Memory Viewer") && selection) {
				if (ImGui::SmallButton("Parse first leaf (May take a second)")) {
					auto occurance = std::search(selection->raw.data(), selection->raw.data() + selection->raw.size(), leafHeader, leafHeader + sizeof(leafHeader));
					
					if (occurance == selection->raw.data() + selection->raw.size()) {
						tinyfd_messageBox("Search failed", "Failed to find matching bytes", "ok", "error", 1);

					}
					else {
						memedit.GotoAddrAndHighlight(occurance - selection->raw.data(), occurance - selection->raw.data() + 16);
						objlibOrigin = selection->raw.data();
						leafOffset = occurance;
					}
				}

				memedit.DrawContents(selection->raw.data(), selection->raw.size(), (size_t)0);
			}

			ImGui::End();
		}

		if (leafOffset) {
			auto displayHash = [](char const* label, uint32_t hash) {

				char const* match = aurora::lookupHash(hash);

				if(match) ImGui::LabelText(label, "%s", match);
				else ImGui::LabelText(label, "%08X", hash);

			};

			if (ImGui::Begin("Leaf Dump")) {
				char* iterator = leafOffset;
				iterator += 16; // Skip header

				ImGui::LabelText("Offset", "%p", (void*)(uintptr_t)(leafOffset - objlibOrigin));
				ImGui::Separator();

				displayHash("Hash", readUint32(&iterator));
				ImGui::LabelText("Unknown", "%08X", readUint32(&iterator));
				ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
				ImGui::LabelText("Timeunit", "%s", readString(&iterator).c_str());
				displayHash("Hash", readUint32(&iterator));
				uint32_t numTraits = readUint32(&iterator);


				

				ImGui::LabelText("Num traits", "%d", numTraits);

				for (int i = 0; i < numTraits; ++i) {

					ImGui::PushID(i);

					std::string traitName = readString(&iterator);

					ImGui::LabelText("Trait name", "%s", traitName.c_str());
					ImGui::LabelText("Unknown", "%08X", readUint32(&iterator));

					displayHash("Parameter", readUint32(&iterator));

					ImGui::LabelText("Subobject Identifier", "%08X", readUint32(&iterator));
					TraitType traitType = (TraitType)readUint32(&iterator);
					ImGui::LabelText("Trait type", "%d", (uint32_t)traitType);
					uint32_t numDatapoints = readUint32(&iterator);
					ImGui::LabelText("Num datapoints", "%d", numDatapoints);

					for (uint32_t j = 0; j < numDatapoints; ++j) {
						uint32_t datapoint = readUint32(&iterator);

						std::any value;
						if (traitType == TraitType::kTraitFloat) value = std::bit_cast<float>(readUint32(&iterator));
						else if (traitType == TraitType::kTraitAction) value = std::bit_cast<char>(readByte(&iterator));
						else if (traitType == TraitType::kTraitBool) value = std::bit_cast<char>(readByte(&iterator));
						else __debugbreak();

						std::string interpolation = readString(&iterator);
						std::string easingMode = readString(&iterator);

						std::string str = std::format("[{}]", j);
						
						if (ImGui::TreeNode(str.c_str())) {
							ImGui::LabelText("Time", "%f", std::bit_cast<float>(datapoint));

							if (traitType == TraitType::kTraitFloat) ImGui::LabelText("Value", "%f", std::any_cast<float>(value));
							else if (traitType == TraitType::kTraitAction) ImGui::LabelText("Value", "%d", std::any_cast<char>(value));
							else if (traitType == TraitType::kTraitBool) ImGui::LabelText("Value", "%d", std::any_cast<char>(value));
							else __debugbreak();

							ImGui::LabelText("Interpolation", "%s", interpolation.c_str());
							ImGui::LabelText("Easing", "%s", easingMode.c_str());

							ImGui::TreePop();
						}
						
					}

					ImGui::Separator();
					ImGui::TextUnformatted("Total number of displayed UI elements for data points on Drool's Editor.");
					uint32_t additional_unknown = readUint32(&iterator);
					for (int j = 0; j < additional_unknown; ++j) {
						ImGui::LabelText("Time", "%f", std::bit_cast<float>(readUint32(&iterator)));

						char const* label = (j == additional_unknown - 1) ? "Value (unused)" : "Value";
						ImGui::LabelText(label, "%f", std::bit_cast<float>(readUint32(&iterator)));
						


						ImGui::LabelText("Interpolation", readString(&iterator).c_str());
						ImGui::LabelText("Easing", readString(&iterator).c_str());
					}
					ImGui::Separator();


					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Intensity type (A)", "%s", readString(&iterator).c_str());
					ImGui::LabelText("Intensity type (B)", "%s", readString(&iterator).c_str());
					ImGui::LabelText("Unknown", "%d", readByte(&iterator));
					ImGui::LabelText("Unknown", "%d", readByte(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Unknown", "%d", readByte(&iterator));
					ImGui::LabelText("Unknown", "%d", readByte(&iterator));
					ImGui::LabelText("Unknown", "%d", readByte(&iterator));

					ImGui::PopID();
				}
			}
			ImGui::End();
		}

		if (showImguiDemo) {
			ImGui::ShowDemoWindow(&showImguiDemo);
		}

		if (viewHasher) {
			if (ImGui::Begin("Hasher", &viewHasher)) {
				static std::string input;
				static uint32_t hash = hash32(nullptr, 0);
				if (ImGui::InputText("Input", &input))
					hash = hash32((unsigned char const*)input.data(), static_cast<unsigned int>(input.size()));

				ImGui::Text("0x%02X", hash);
			}
			ImGui::End();
		}

		if (viewBpms) {
			if (ImGui::Begin("Level Bpms", &viewBpms)) {
				ImGui::LabelText("Level 1", "%s", "320");
				ImGui::LabelText("Level 2", "%s", "340");
				ImGui::LabelText("Level 3", "%s", "360");
				ImGui::LabelText("Level 4", "%s", "380");
				ImGui::LabelText("Level 5", "%s", "400");
				ImGui::LabelText("Level 6", "%s", "420");
				ImGui::LabelText("Level 7", "%s", "440");
				ImGui::LabelText("Level 8", "%s", "460");
				ImGui::LabelText("Level 9", "%s", "480");
				ImGui::LabelText("Level 10", "%s", "270-550~");
			}
			ImGui::End();
		}

		if (ImGui::Begin("Obj Libs")) {
			ImGui::Text("Loaded %d libs", kMap.size());
			ImGui::Text("Failed to load %d libs", failedCount);

			ImGui::InputText("Filter", &filter);

			for (auto& [k, v] : kMap) {

				bool matches = true;

				// Apply filters
				if (!filter.empty()) {
					std::vector<std::string> tokens;
					split(filter, tokens, ' ');


					for (auto const& token : tokens)
						if (!v.originalName.contains(token)) {
							matches = false;
							break;
						}
				}

				if (!matches) continue;

				bool node_open = ImGui::TreeNodeEx(v.originalName.c_str(), selection == &v ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None);
				if (ImGui::IsItemClicked()) {
					selection = &v;
				}

				if (node_open) {
					ImGui::LabelText("Origin", "%s", k.c_str());

					ImGui::PushID(&v);
					
					if (ImGui::SmallButton("Jump to object definitions")) {
						selection = &v;
						memedit.GotoAddrAndHighlight(v.headerDefOffset, v.headerDefOffset + 4);
					}

					ImGui::PopID();

					if (v.libraryImports.size() > 0 && ImGui::TreeNode("Library Imports")) {
						for (auto const& import : v.libraryImports)
							ImGui::TextUnformatted(import.string.c_str());

						ImGui::TreePop();
					}

					if (v.objectImports.size() > 0 && ImGui::TreeNode("Object Imports")) {
						for (auto const& import : v.objectImports)
							ImGui::Text("%s from %s", import.objName.c_str(), import.libraryName.c_str());

						ImGui::TreePop();
					}

					if (v.objects.size() > 0 && ImGui::TreeNode("Objects")) {
						for (auto const& object : v.objects)
							ImGui::TextUnformatted(object.name.c_str());

						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
			}
		}
		ImGui::End();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}

#ifdef HE_ENTRY_WINMAIN
#include <Windows.h> // WINAPI, WinMain, _In_, _In_opt_, HINSTANCE
#include <stdlib.h> // __argc, __argv
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	return main(__argc, __argv);
}
#endif