#ifdef _WIN32
#	include <Windows.h> // Suppress: warning C4005: 'APIENTRY': macro redefinition
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <tinyfiledialogs.h>
#include <imgui_impl_glfw.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3_loader.h>
#include <imgui.h>
#include <lua.hpp>

#include "hashtable.hpp"

#include <vulpengine/vp_transform.hpp>

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

#include <vulpengine/vp_entry.hpp>

#include <cstdint>
#include <span>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/extended_min_max.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_memory_editor.h>
#include "thumper_structs.hpp"

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vulpengine/experimental/vp_shader_program.hpp"

#define AURORA_WIKI "http://thumper.anthofoxo.xyz/"

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

glm::vec3 readVec3(char** ptr) {
	glm::vec3 data;
	memcpy(&data, *ptr, sizeof(glm::vec3));
	*ptr += sizeof(glm::vec3);
	return data;
}

enum struct FileType : uint32_t {
	kMeshX      =  6,
	kObjlib     =  8,
	kFsbTexture = 13,
	kDdsTexture = 14,
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
	std::string originFile;


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

int failedCount = 0;

#include <vulpengine/vp_util.hpp>

std::optional<Objlib> readObjlib(char const* file) {
	Objlib lib;

	lib.originFile = file;

	lib.raw = vulpengine::read_file(file).value();
	char* const baseaddr = lib.raw.data();
	char* ptr = lib.raw.data();


	memcpy(&lib.header, ptr, sizeof(ObjlibHeader));
	ptr += sizeof(ObjlibHeader);

	if (lib.header.fileType == FileType::kMeshX) {
		return std::nullopt;
	}

	if (lib.header.fileType != FileType::kObjlib) {
		++failedCount;
		return std::nullopt;
	}

	if (lib.header.objType == ObjType::kObjlibObj) {
		++failedCount;
		std::cout << "unsupported type " << file << '\n';

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

void displayHash(char const* label, uint32_t hash) {
	char const* match = aurora::lookupHash(hash);

	if (match) ImGui::LabelText(label, "%s", match);
	else ImGui::LabelText(label, "%08X", hash);
}

static void writeU8(std::vector<uint8_t>& buffer, uint8_t data) {
	buffer.push_back(data);
}

static void writeU32(std::vector<uint8_t>& buffer, uint32_t data) {
	for (int i = 0; i < sizeof(uint32_t); ++i)
		buffer.push_back(0);

	memcpy(buffer.data() + buffer.size() - sizeof(uint32_t), &data, sizeof(uint32_t));
}

static void writeStr(std::vector<uint8_t>& buffer, std::string_view data) {
	writeU32(buffer, static_cast<uint32_t>(data.size()));
	for (auto c : data)
		buffer.push_back(c);
}

static void writeF32(std::vector<uint8_t>& buffer, float data) {
	for (int i = 0; i < sizeof(float); ++i)
		buffer.push_back(0);

	memcpy(buffer.data() + buffer.size() - sizeof(float), &data, sizeof(float));
}

static void writeVec3(std::vector<uint8_t>& buffer, glm::vec3 const& data) {
	writeF32(buffer, data.x);
	writeF32(buffer, data.y);
	writeF32(buffer, data.z);
}

void InjectIntoPc(std::vector<uint8_t>& raw, std::string originFile, size_t originSize, size_t originOffset) {
	std::string backup = originFile + std::string(".bak");
	if (!std::filesystem::exists(backup))
		std::filesystem::copy(originFile, backup);

	std::vector<char> original = vulpengine::read_file(originFile.c_str()).value();
	std::vector<char> final;
	final.resize(original.size() - originSize + raw.size());

	memcpy(final.data(), original.data(), originOffset);
	memcpy(final.data() + originOffset, raw.data(), raw.size());
	memcpy(final.data() + originOffset + raw.size(), original.data() + originOffset + originSize + 1, original.size() - originOffset - originSize);

	std::ofstream stream(originFile, std::ofstream::binary);
	stream.write(final.data(), final.size());
	stream.close();

	tinyfd_messageBox("Injected", "Changed injected, Application will exit", "ok", "info", 1);
	std::exit(0);
}

struct Samp final {
	std::string originFile;
	size_t originSize = 0;
	size_t originOffset = 0;

	uint32_t header[3];
	uint32_t hash;
	std::string playMode;
	uint32_t unknown0;
	std::string filepath;
	uint8_t unknown1[5];
	float volume;
	float pitch;
	float pan;
	float offset; // Likely in seconds
	std::string channel;

	void draw() {
		ImGui::TextUnformatted("Origin");
		ImGui::Separator();
		ImGui::LabelText("File", "%s", originFile.c_str());
		ImGui::LabelText("Offset", "%d", originOffset);
		ImGui::LabelText("Size", "%d", originSize);

		ImGui::TextUnformatted("Data");
		ImGui::Separator();

		displayHash("Hash", hash);
		ImGui::InputText("Play mode", &playMode);
		ImGui::InputScalarN("Unknown 0", ImGuiDataType_U32, &unknown0, 1, nullptr, nullptr, "%d", 0);
		ImGui::InputText("Filepath", &filepath);
		ImGui::InputScalarN("Unknown 1", ImGuiDataType_U8, &unknown1, 5, nullptr, nullptr, "%d", 0);
		ImGui::DragFloat("Volume", &volume);
		ImGui::DragFloat("Pitch", &pitch);
		ImGui::DragFloat("Pan", &pan);
		ImGui::DragFloat("Offset", &offset);
		ImGui::InputText("Channel", &channel);

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_Button, { 1,0,0,1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1, .3f, .3f,1 });
		if (ImGui::Button("Inject")) {
			std::vector<uint8_t> raw;
			serialize(raw);
			InjectIntoPc(raw, originFile, originSize, originOffset);
		}
		ImGui::PopStyleColor(2);
	}

	void origin(std::string const& file, size_t offset, size_t size) {
		originFile = file;
		originOffset = offset;
		originSize = size;
	}

	char* deserialize(char* ptr) {
		memcpy(header, ptr, sizeof(header)); ptr += sizeof(header);
		hash = readUint32(&ptr);
		playMode = readString(&ptr);
		unknown0 = readUint32(&ptr);
		filepath = readString(&ptr);
		memcpy(unknown1, ptr, sizeof(unknown1)); ptr += sizeof(unknown1);
		volume = std::bit_cast<float>(readUint32(&ptr));
		pitch = std::bit_cast<float>(readUint32(&ptr));
		pan = std::bit_cast<float>(readUint32(&ptr));
		offset = std::bit_cast<float>(readUint32(&ptr));
		channel = readString(&ptr);
		return ptr;
	}

	void serialize(std::vector<uint8_t>& data) {
		writeU32(data, header[0]);
		writeU32(data, header[1]);
		writeU32(data, header[2]);
		writeU32(data, hash);
		writeStr(data, playMode);
		writeU32(data, unknown0);
		writeStr(data, filepath);
		writeU8(data, unknown1[0]);
		writeU8(data, unknown1[1]);
		writeU8(data, unknown1[2]);
		writeU8(data, unknown1[3]);
		writeU8(data, unknown1[4]);
		writeF32(data, volume);
		writeF32(data, pitch);
		writeF32(data, pan);
		writeF32(data, offset);
		writeStr(data, channel);
	}

};

struct Spn final {
	std::string originFile;
	size_t originSize = 0;
	size_t originOffset = 0;

	uint32_t header[3];
	uint32_t hash0;
	uint32_t hash1;
	uint32_t unknown0;
	std::string name;
	std::string constraint;
	glm::vec3 translation;
	glm::vec3 rotationx;
	glm::vec3 rotationy;
	glm::vec3 rotationz;
	glm::vec3 scale;
	uint32_t unknown1;
	std::string objlibpath;
	std::string bucketType;

	void origin(std::string const& file, size_t offset, size_t size) {
		originFile = file;
		originOffset = offset;
		originSize = size;
	}

	void draw() {
		ImGui::TextUnformatted("Origin");
		ImGui::Separator();
		ImGui::LabelText("File", "%s", originFile.c_str());
		ImGui::LabelText("Offset", "%d", originOffset);
		ImGui::LabelText("Size", "%d", originSize);

		ImGui::TextUnformatted("Data");
		ImGui::Separator();

		displayHash("Hash", hash0);
		displayHash("Hash", hash1);
		ImGui::InputScalarN("Unknown 0", ImGuiDataType_U32, &unknown0, 1, nullptr, nullptr, "%d", 0);
		ImGui::InputText("Name", &name);
		ImGui::InputText("Constraint", &constraint);
		ImGui::DragFloat3("Position", (float*)&translation);
		ImGui::DragFloat3("Rotation x", (float*)&rotationx);
		ImGui::DragFloat3("Rotation y", (float*)&rotationy);
		ImGui::DragFloat3("Rotation z", (float*)&rotationz);
		ImGui::DragFloat3("Scale", (float*)&scale);
		ImGui::InputScalarN("Unknown 1", ImGuiDataType_U32, &unknown1, 1, nullptr, nullptr, "%d", 0);
		ImGui::InputText("Objlibpath", &objlibpath);
		ImGui::InputText("Bucket type", &bucketType);

		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_Button, { 1,0,0,1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1, .3f, .3f,1 });
		if (ImGui::Button("Inject")) {
			std::vector<uint8_t> raw;
			serialize(raw);
			InjectIntoPc(raw, originFile, originSize, originOffset);

		}
		ImGui::PopStyleColor(2);
	}

	char* deserialize(char* ptr) {
		memcpy(header, ptr, sizeof(header)); ptr += sizeof(header);
		hash0 = readUint32(&ptr);
		hash1 = readUint32(&ptr);
		unknown0 = readUint32(&ptr);
		name = readString(&ptr);
		constraint = readString(&ptr);
		translation = readVec3(&ptr);
		rotationx = readVec3(&ptr);
		rotationy = readVec3(&ptr);
		rotationz = readVec3(&ptr);
		scale = readVec3(&ptr);
		unknown1 = readUint32(&ptr);
		objlibpath = readString(&ptr);
		bucketType = readString(&ptr);
		return ptr;
	}

	void serialize(std::vector<uint8_t>& data) {
		writeU32(data, header[0]);
		writeU32(data, header[1]);
		writeU32(data, header[2]);
		writeU32(data, hash0);
		writeU32(data, hash1);
		writeU32(data, unknown0);
		writeStr(data, name);
		writeStr(data, constraint);
		writeVec3(data, translation);
		writeVec3(data, rotationx);
		writeVec3(data, rotationy);
		writeVec3(data, rotationz);
		writeVec3(data, scale);
		writeU32(data, unknown1);
		writeStr(data, objlibpath);
		writeStr(data, bucketType);
	}
};

static std::optional<Spn> spnParsed = std::nullopt;
static std::optional<Samp> sampParsed = std::nullopt;

void loadObjLibs() {
	for (auto const& entry : std::filesystem::directory_iterator(kCacheDir)) {
		std::string file = entry.path().filename().string();

		if (entry.path().extension() != ".pc") continue;

		std::optional<Objlib> lib = readObjlib(entry.path().string().c_str());
		if (lib.has_value()) {
			kMap[entry.path().stem().string()] = lib.value();

		
		}
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

size_t split(const std::string& txt, std::vector<std::string>& strs, char ch) {
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

void hookfunc(lua_State* L, lua_Debug* ar) {
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

bool attemptObjPcReplace(std::string obj, std::string pc) {
	if (!std::string_view(obj).ends_with(".obj")) return false;
	if (!std::string_view(pc).ends_with(".pc")) return false;

	// Read pc mesh for _unknownField4
	thumper::MeshFile pcMeshOriginal = thumper::MeshFile::from_file(pc).value();

	std::string const backupPath = pc + ".bak";

	// Import obj
	Assimp::Importer importer;
	aiScene const* scene = importer.ReadFile(obj.c_str(), aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_GenNormals | aiProcess_ImproveCacheLocality | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) return false;
	if (scene->mNumMeshes != 1) return false;

	aiMesh* mesh = scene->mMeshes[0];

	thumper::MeshFile pcMesh;
	pcMesh.meshes.resize(1);
	pcMesh.meshes[0].vertices.reserve(mesh->mNumVertices);

	// Store vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		thumper::Vertex v;
		v.texcoord = { 0.0f, 0.0f };
		v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		if (mesh->GetNumUVChannels() > 0) v.texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][1].y };
		v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		v.color = { 0, 0, 0, 0 };
		pcMesh.meshes[0].vertices.push_back(v);
	}

	// Store triangles
	pcMesh.meshes[0].triangles.reserve(mesh->mNumFaces);

	for (unsigned int iFace = 0; iFace < mesh->mNumFaces; iFace++) {
		aiFace face = mesh->mFaces[iFace];
		if (face.mNumIndices != 3) continue;

		thumper::Triangle t;
		t.elements[0] = face.mIndices[0];
		t.elements[1] = face.mIndices[1];
		t.elements[2] = face.mIndices[2];

		pcMesh.meshes[0].triangles.push_back(t);
	}

	// Make backup if needed
	if (!std::filesystem::exists(backupPath))
		std::filesystem::copy(pc, backupPath);

	pcMesh.to_file(pc);

	return true;
}

struct MeshWorkspace {
	void init() {
		for (auto const& entry : std::filesystem::directory_iterator(kCacheDir)) {
			if (entry.path().extension() != ".pc") continue;

			auto content = thumper::MeshFile::from_file(entry.path());
			if (!content) continue;

			mFiles.emplace_back(entry.path().filename().generic_string());
		}

		mShaderProgramSolid = {{ .file = "solid.glsl" }};
		mShaderProgramGrid = {{ .file = "grid.glsl" }};
	}

	void gui() {
		if (ImGui::Begin("Mesh Workspace - Mesh Info")) {
			ImGui::Checkbox("Flip Y Axis", &mFlipAxis);
			ImGui::Checkbox("Flip Winding", &mFlipWinding);
			if (ImGui::Button("Move Camera")) {
				mTransform.set(glm::inverse(glm::lookAt(glm::vec3(mLargestVertexDistanceTarget), mGeometryCenterTarget, { 0, 1, 0 })));
			}

			if (ImGui::Button("Export Mesh")) {
				auto pcMesh = thumper::MeshFile::from_file(kCacheDir + "/" + mSelected);
				
				if (pcMesh) {
					// Assimp scene setup
					aiScene scene;
					scene.mRootNode = new aiNode();
					scene.mMaterials = new aiMaterial * [1];
					scene.mMaterials[0] = nullptr;
					scene.mNumMaterials = 1;
					scene.mMaterials[0] = new aiMaterial();
					scene.mMeshes = new aiMesh * [1];
					scene.mMeshes[0] = nullptr;
					scene.mNumMeshes = 1;
					scene.mMeshes[0] = new aiMesh();
					scene.mMeshes[0]->mMaterialIndex = 0;
					scene.mRootNode->mMeshes = new unsigned int[1];
					scene.mRootNode->mMeshes[0] = 0;
					scene.mRootNode->mNumMeshes = 1;
					auto pMesh = scene.mMeshes[0];

					// Allocate containers
					thumper::Mesh& pcMeshOut = pcMesh->meshes[mMeshIndex];
					pMesh->mVertices = new aiVector3D[pcMeshOut.vertices.size()];
					pMesh->mNumVertices = pcMeshOut.vertices.size();
					pMesh->mTextureCoords[0] = new aiVector3D[pcMeshOut.vertices.size()];
					pMesh->mNumUVComponents[0] = pcMeshOut.vertices.size();

					// Copy vertex data
					for (auto itr = pcMeshOut.vertices.begin(); itr != pcMeshOut.vertices.end(); ++itr) {
						const auto& v = itr->position;
						const auto& t = itr->texcoord;
						pMesh->mVertices[itr - pcMeshOut.vertices.begin()] = aiVector3D(v[0], v[1], v[2]);
						pMesh->mTextureCoords[0][itr - pcMeshOut.vertices.begin()] = aiVector3D(t[0], t[2], 0);
					}

					// Allocate containers
					pMesh->mFaces = new aiFace[pcMeshOut.triangles.size()];
					pMesh->mNumFaces = pcMeshOut.triangles.size();

					// Copy triangle data
					for (int i = 0; i < pcMeshOut.triangles.size(); ++i) {
						aiFace& face = pMesh->mFaces[i];
						face.mIndices = new unsigned int[3];
						face.mNumIndices = 3;

						face.mIndices[0] = pcMeshOut.triangles[i].elements[0];
						face.mIndices[1] = pcMeshOut.triangles[i].elements[1];
						face.mIndices[2] = pcMeshOut.triangles[i].elements[2];
					}

					std::string exportPath = kCacheDir + "/" + mSelected + "." + std::to_string(mMeshIndex) + ".obj";

					Assimp::Exporter exporter;
					aiReturn result = exporter.Export(&scene, "obj", exportPath.c_str());
					if (result != aiReturn_SUCCESS) {
						throw std::runtime_error(exporter.GetErrorString());
					}
				}
			}

			ImGui::BeginDisabled(!mHasBackup);

			if (ImGui::Button("Restore Backup")) {
				std::filesystem::path current = kCacheDir + "/" + mSelected;
				std::filesystem::path backup = kCacheDir + "/" + mSelected + ".bak";

				std::filesystem::copy_file(backup, current, std::filesystem::copy_options::overwrite_existing);
				std::filesystem::remove(backup);
				mHasBackup = false;

				try {
					update_preview(mSelected);
					mValid = true;
				}
				catch (std::runtime_error const&) {
					mValid = false;
				}
			}

			ImGui::EndDisabled();

			if (ImGui::Button("Replace Mesh LODs")) {
				char const* filter = "*.obj";
				char const* objPath = tinyfd_openFileDialog("Select mesh", nullptr, 1, &filter, nullptr, false);
	
				std::string pcPath = kCacheDir + "/" + mSelected;

				if (objPath) {
					bool success = attemptObjPcReplace(objPath, pcPath);

					if (!success) ImGui::OpenPopup("InvalidMeshInput");
					else {
						try {
							update_preview(mSelected);
							mValid = true;
						}
						catch (std::runtime_error const&) {
							mValid = false;
						}
					}
				}
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("InvalidMeshInput", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Failed to replace mesh lods");
				if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

				ImGui::EndPopup();
			}

			ImGui::LabelText("Meshes in file", "%d", mMeshInfos.size());

			if (ImGui::SliderInt("Mesh Index", &mMeshIndex, 0, mMeshInfos.size() - 1)) {
				try {
					update_preview(mSelected);
					mValid = true;
				}
				catch (std::runtime_error const&) {
					mValid = false;
				}
			}

			for (auto const& info : mMeshInfos) {
				ImGui::Separator();
				ImGui::LabelText("Vertex Count", "%d", info.vertexCount);
				ImGui::LabelText("Triangle Count", "%d", info.triangleCount);
				ImGui::LabelText("Unknown", "%hu", info.unknownValue);
			}
		}
		ImGui::End();

		if (ImGui::Begin("Mesh Workspace - Meshes")) {
			ImGui::LabelText("Mesh Count", "%d", mFiles.size());

			for (auto const& string : mFiles) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
				if (mSelected == string) flags |= ImGuiTreeNodeFlags_Selected;

				ImGui::TreeNodeEx(string.c_str(), flags);

				if (ImGui::IsItemActivated()) {
					mSelected = string;

					// Check for a backup file
					mHasBackup = std::filesystem::exists(kCacheDir + "/" + mSelected + ".bak");

					try {
						update_preview(mSelected);
						mValid = true;
					}
					catch (std::runtime_error const&) {
						mValid = false;
					}
				}

				ImGui::TreePop();
			}

		}
		ImGui::End();

		if (ImGui::Begin("Mesh Workspace - Preview")) {
			if (mSelected.empty() || !mValid) {
				ImGui::TextUnformatted("Invalid Mesh");
			}
			else {
				ImVec2 avail = ImGui::GetContentRegionAvail();
				draw_preview(static_cast<int>(avail.x), static_cast<int>(avail.y));
				ImGui::Image((void*)(uintptr_t)mFramebufferColor, avail, { 0,1 }, { 1, 0 });
			}
		}
		ImGui::End();
	}

	void update_preview(std::string source) {
		if (source.empty()) return;

		if (mVertexArray != 0) {
			glDeleteVertexArrays(1, &mVertexArray);
			glDeleteBuffers(1, &mVertexBuffer);
			glDeleteBuffers(1, &mIndexBuffer);
		}

		std::string sourceDir = kCacheDir + "/" + source;
		auto thumpermesh = thumper::MeshFile::from_file(sourceDir);
		if (!thumpermesh) return;

		// Make sure we don't try to load meshes past the count
		if (mMeshIndex >= thumpermesh->meshes.size()) mMeshIndex = thumpermesh->meshes.size() - 1;

		// Present this mesh
		{
			thumper::Mesh mesh = thumpermesh->meshes[mMeshIndex];

			mElementCount = mesh.triangles.size() * 3;
			mLargestVertexDistanceTarget = 0;
			mGeometryCenterTarget = {};
			for (thumper::Vertex const& v : mesh.vertices) {
				float maxCoord = glm::max(glm::max(glm::abs(v.position[0]), glm::abs(v.position[1])), glm::abs(v.position[2]));
				if (maxCoord > mLargestVertexDistanceTarget) mLargestVertexDistanceTarget = maxCoord;
				mGeometryCenterTarget += v.position;
			}

			mGeometryCenterTarget /= static_cast<float>(mesh.vertices.size());

			glGenVertexArrays(1, &mVertexArray);
			glBindVertexArray(mVertexArray);

			glGenBuffers(1, &mVertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(decltype(mesh.vertices)::value_type), mesh.vertices.data(), GL_STATIC_DRAW);

			glGenBuffers(1, &mIndexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.triangles.size() * sizeof(decltype(mesh.triangles)::value_type), mesh.triangles.data(), GL_STATIC_DRAW);

			auto const stride = sizeof(thumper::Vertex);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(uintptr_t)offsetof(thumper::Vertex, position));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(uintptr_t)offsetof(thumper::Vertex, normal));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(uintptr_t)offsetof(thumper::Vertex, texcoord));
			glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, stride, (void*)(uintptr_t)offsetof(thumper::Vertex, color));

			glBindVertexArray(0);
		}
		
	}

	void draw_preview(int width, int height) {
		if (mVertexArray == 0) return;
		if (mElementCount == 0) return;

		if (width != mFramebufferSizeX || height != mFramebufferSizeY) {
			mFramebufferSizeX = width;
			mFramebufferSizeY = height;

			if (mFramebufferColor != 0) glDeleteTextures(1, &mFramebufferColor);
			glGenTextures(1, &mFramebufferColor);
			glBindTexture(GL_TEXTURE_2D, mFramebufferColor);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mFramebufferSizeX, mFramebufferSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			if (mFramebufferDepth != 0) glDeleteTextures(1, &mFramebufferDepth);
			glGenTextures(1, &mFramebufferDepth);
			glBindTexture(GL_TEXTURE_2D, mFramebufferDepth);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mFramebufferSizeX, mFramebufferSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

			if (mFramebuffer != 0) glDeleteFramebuffers(1, &mFramebuffer);
			glGenFramebuffers(1, &mFramebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFramebufferColor, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mFramebufferDepth, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
		glViewport(0, 0, width, height);
		glClearColor(0.7f, 0.8f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		mShaderProgramSolid.bind();

		glm::mat4 projection = glm::perspectiveFov(glm::radians(90.0f), (float)width, (float)height, 0.1f, 2048.0f);

		if (ImGui::IsWindowHovered()) {
			glm::vec3 move{};

			if (ImGui::IsKeyDown(ImGuiKey_W)) --move.z;
			if (ImGui::IsKeyDown(ImGuiKey_S)) ++move.z;
			if (ImGui::IsKeyDown(ImGuiKey_A)) --move.x;
			if (ImGui::IsKeyDown(ImGuiKey_D)) ++move.x;

			mTransform.translate(move * ImGui::GetIO().DeltaTime * 15.0f);
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
			glm::vec3 const up = glm::vec3(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * mTransform.get());

			mTransform.orientation = glm::rotate(mTransform.orientation, glm::radians(-ImGui::GetIO().MouseDelta.x * 0.3f), up);
			mTransform.orientation = glm::rotate(mTransform.orientation, glm::radians(-ImGui::GetIO().MouseDelta.y * 0.3f), { 1.0f, 0.0f, 0.0f });
		}

		mShaderProgramSolid.push_mat4f("uProjection", projection);
		mShaderProgramSolid.push_mat4f("uView", glm::inverse(mTransform.get()));
		mShaderProgramSolid.push_3f("uColor", 1.0f, 0.5f, 0.2f);
		mShaderProgramSolid.push_1f("uFlip", mFlipAxis);

		glBindVertexArray(mVertexArray);

		bool cwmode = true;
		cwmode ^= mFlipAxis;
		cwmode ^= mFlipWinding;
		if(cwmode) glFrontFace(GL_CW);

		glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, nullptr);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mShaderProgramSolid.push_3f("uColor", 1.0f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, mElementCount, GL_UNSIGNED_SHORT, nullptr);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glFrontFace(GL_CCW);

		mShaderProgramGrid.bind();
		mShaderProgramGrid.push_mat4f("uProjection", projection);
		mShaderProgramGrid.push_mat4f("uView", glm::inverse(mTransform.get()));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisable(GL_BLEND);


		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	struct SelectedMeshInfo {
		uint32_t vertexCount;
		uint32_t triangleCount;
		uint16_t unknownValue;
	};

	bool mHasBackup = false;
	bool mFlipWinding = false;
	bool mFlipAxis = false;
	bool mValid = false;
	int mMeshIndex = 0;
	std::vector<SelectedMeshInfo> mMeshInfos;
	std::string mSelected;

	GLuint mVertexArray = 0;
	GLuint mVertexBuffer = 0;
	GLuint mIndexBuffer = 0;
	GLsizei mElementCount = 0;
	vulpengine::Transform mTransform;
	float mLargestVertexDistanceTarget = 0;
	glm::vec3 mGeometryCenterTarget = {};

	GLuint mFramebuffer = 0;
	GLuint mFramebufferColor = 0;
	GLuint mFramebufferDepth = 0;
	int mFramebufferSizeX = 0;
	int mFramebufferSizeY = 0;

	std::vector<std::string> mFiles;
	vulpengine::experimental::ShaderProgram mShaderProgramSolid;
	vulpengine::experimental::ShaderProgram mShaderProgramGrid;
};

std::optional<MeshWorkspace> mWorkspaceMesh;

int main(int argc, char* argv[]) {
	loadConfig();

	loadObjLibs();

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Aurora 0.0.3-a.7: Thumper Cache Decompilation and Replacement", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	gladLoadGL(&glfwGetProcAddress);

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
	bool workspaceMesh = false;

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

				if (ImGui::MenuItem("Thumper Wiki", nullptr, nullptr)) {
					SystemOpenURL(AURORA_WIKI);
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem("ImGuiDemo", nullptr, &showImguiDemo);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Workspaces")) {
				ImGui::MenuItem("Meshes", nullptr, &workspaceMesh);
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
		static char* parseOffset = nullptr;
		
		const char* items[] = { "NOP", "Leaf", "Master", "Spn", "Samp"};
		static int parseModeIdx = 0;

		{
			if (ImGui::Begin("Search for offset by bytes") && selection) {
				static std::string input;
				static int offsetbegin = 0;

				if (ImGui::SmallButton("Samp header")) { input = "0C 00 00 00 04 00 00 00 01 00 00 00"; parseModeIdx = 4; };
				if (ImGui::SmallButton("Spn header")) { input = "01 00 00 00 04 00 00 00 02 00 00 00"; parseModeIdx = 3; };
				if (ImGui::SmallButton("Master header")) { input = "21 00 00 00 21 00 00 00 04 00 00 00 02 00 00 00"; parseModeIdx = 2; };
				if (ImGui::SmallButton("Leaf header")) { input = "22 00 00 00 21 00 00 00 04 00 00 00 02 00 00 00"; parseModeIdx = 1; }

				ImGui::InputInt("Offset start", &offsetbegin);
				ImGui::InputText("Byte search", &input);
		
				const char* combo_preview_value = items[parseModeIdx];
				if (ImGui::BeginCombo("Parse mode", combo_preview_value, 0)) {
					parseOffset = nullptr;

					for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
						const bool is_selected = (parseModeIdx == n);
						if (ImGui::Selectable(items[n], is_selected)) parseModeIdx = n;
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
		

				if (ImGui::Button("Search")) { 
					std::vector<std::string> tokens;
					std::vector<uint8_t> byteTokens;
					split(input, tokens, ' ');
					byteTokens.reserve(tokens.size());

					for (auto const& token : tokens) {
						std::istringstream ss(token);
						unsigned int byte;
						ss >> std::hex >> byte;
						byteTokens.push_back(static_cast<uint8_t>(byte));
					}

					auto occurance = std::search(selection->raw.data() + offsetbegin, selection->raw.data() + selection->raw.size(), byteTokens.data(), byteTokens.data() + byteTokens.size());

					if (occurance == selection->raw.data() + selection->raw.size()) {
						tinyfd_messageBox("Search failed", "Failed to find matching bytes", "ok", "error", 1);

					}
					else {
						offsetbegin = occurance - selection->raw.data();
						memedit.GotoAddrAndHighlight(occurance - selection->raw.data(), occurance - selection->raw.data() + 16);
						objlibOrigin = selection->raw.data();
						parseOffset = occurance;

						if (parseModeIdx == 4) {
							sampParsed = Samp();
							char* a = parseOffset;
							char* b = sampParsed->deserialize(parseOffset);
							sampParsed->origin(selection->originFile, a - selection->raw.data(), b - a - 1);
						}
						else if (parseModeIdx == 3) {
							spnParsed = Spn();
							char* a = parseOffset;
							char* b = spnParsed->deserialize(parseOffset);
							spnParsed->origin(selection->originFile, a - selection->raw.data(), b - a - 1);
						}
					}
				}
			}
			ImGui::End();

			if (ImGui::Begin("Memory Viewer") && selection) {
				memedit.DrawContents(selection->raw.data(), selection->raw.size(), (size_t)0);
			}
			ImGui::End();

		}

		if (parseOffset && parseModeIdx != 0) {
			if (parseModeIdx == 1) {
				if (ImGui::Begin("Parser")) {

					struct DataPoint {
						float datapoint;
						std::any value;
						std::string interpolation;
						std::string easing;
					};

					struct Trait {
						std::string name;
						uint32_t unknown0;
						uint32_t param;
						uint32_t subobjectIdentifier;
						TraitType traitType;
						std::vector<DataPoint> datapoints;
					};

					struct Leaf {
						uint32_t header[4];
						uint32_t hash0;
						uint32_t unknown0;
						float unknown1;
						std::string timeUnit;
						uint32_t hash1;
						std::vector<Trait> traits;
					};

					char* iterator = parseOffset;
					iterator += 16; // Skip header

					ImGui::LabelText("Offset", "%p", (void*)(uintptr_t)(parseOffset - objlibOrigin));
					ImGui::Separator();

					displayHash("Hash", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%08X", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Timeunit", "%s", readString(&iterator).c_str());
					displayHash("Hash", readUint32(&iterator));
					uint32_t numTraits = readUint32(&iterator);


					ImGui::LabelText("Num traits", "%d", numTraits);

					for (uint32_t i = 0; i < numTraits; ++i) {

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
						for (uint32_t j = 0; j < additional_unknown; ++j) {
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

					// We need to figure out how many 0 bytes pad the end of the leaf
				}
				ImGui::End();

			}

			if (parseModeIdx == 2) {
				if (ImGui::Begin("Master dump")) {
					char* iterator = parseOffset;
					iterator += 16; // Skip header

					ImGui::LabelText("Offset", "%p", (void*)(uintptr_t)(parseOffset - objlibOrigin));
					ImGui::Separator();

					displayHash("Hash", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					displayHash("Hash", readUint32(&iterator));
					ImGui::LabelText("Timeunit", "%s", readString(&iterator).c_str());
					displayHash("Hash", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%d", readUint32(&iterator));
					ImGui::LabelText("Unknown", "%f", std::bit_cast<float>(readUint32(&iterator)));
					ImGui::LabelText("Skybox name", "%s", readString(&iterator).c_str());
					ImGui::LabelText("Intro level", "%s", readString(&iterator).c_str());

					uint32_t numSublevels = readUint32(&iterator);
					ImGui::LabelText("Num sublevels", "%d", numSublevels);

					if(numSublevels >= 1) {

						ImGui::Separator();
						ImGui::LabelText("Level name", "%s", readString(&iterator).c_str()); // .lvl
						ImGui::LabelText("Gate name", "%s", readString(&iterator).c_str());
						ImGui::LabelText("Checkpoint?", "%d", readByte(&iterator));
						ImGui::LabelText("Checkpoint leader level name", "%s", readString(&iterator).c_str());
						ImGui::LabelText("Rest level name", "%s", readString(&iterator).c_str());
					
						ImGui::TextUnformatted("A variable length buffer is here and is unknown how to calculate its length, cannot parse further");

					}

				}
				ImGui::End();
			}

			if (ImGui::Begin("Object editor")) {
				if (parseModeIdx == 3 && spnParsed) spnParsed->draw();
				else if (parseModeIdx == 4 && sampParsed) sampParsed->draw();
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

		if (workspaceMesh) {
			if (!mWorkspaceMesh) {
				mWorkspaceMesh = MeshWorkspace();
				mWorkspaceMesh->init();
			}

			mWorkspaceMesh->gui();
		}
		else if(mWorkspaceMesh) {
			mWorkspaceMesh = {};
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