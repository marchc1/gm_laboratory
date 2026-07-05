#include "native/laboratory/icon_texture.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <windows.h>
#include <d3d9.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

#include "native/laboratory/embedded_icons.h"
#include "native/log.h"

namespace gm_laboratory {

	namespace {

		IDirect3DDevice9* g_device = nullptr;
		std::map<std::string, IDirect3DTexture9*> g_cache;

		bool IsAbsolute(const char* p) {
			return (p[0] == '\\' || p[0] == '/') || (p[0] && p[1] == ':');
		}

		const std::string& ModuleDir() {
			static std::string dir = [] {
				HMODULE mod = nullptr;
				GetModuleHandleExW(
					GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
					reinterpret_cast<LPCWSTR>(&g_device), &mod);
				char buf[MAX_PATH] = {};
				GetModuleFileNameA(mod, buf, MAX_PATH);
				std::string p = buf;
				size_t slash = p.find_last_of("\\/");
				return slash == std::string::npos ? std::string() : p.substr(0, slash);
			}();
			return dir;
		}

		std::string ExeDir() {
			char buf[MAX_PATH] = {};
			GetModuleFileNameA(nullptr, buf, MAX_PATH);
			std::string p = buf;
			size_t slash = p.find_last_of("\\/");
			return slash == std::string::npos ? std::string() : p.substr(0, slash);
		}

		std::vector<std::string> Candidates(const char* path) {
			std::vector<std::string> out;
			if (IsAbsolute(path)) {
				out.emplace_back(path);
				return out;
			}
			if (!ModuleDir().empty())
				out.push_back(ModuleDir() + "\\" + path);
			if (!ExeDir().empty())
				out.push_back(ExeDir() + "\\" + path);
			out.emplace_back(path);
			return out;
		}

		IDirect3DTexture9* CreateFromFile(const char* path) {
			int w = 0, h = 0, channels = 0;
			stbi_uc* pixels = nullptr;

			unsigned int embSize = 0;
			if (const unsigned char* emb = GetEmbeddedIcon(path, &embSize)) {
				pixels = stbi_load_from_memory(emb, static_cast<int>(embSize), &w, &h, &channels, 4);
				if (!pixels)
					Log("icons", "embedded decode failed for '%s': %s\n", path, stbi_failure_reason());
			}

			if (!pixels) {
				std::vector<std::string> candidates = Candidates(path);
				for (const std::string& candidate : candidates) {
					pixels = stbi_load(candidate.c_str(), &w, &h, &channels, 4);
					if (pixels)
						break;
				}
				if (!pixels) {
					Log("icons", "no image for '%s' (tried embedded + '%s'): %s\n",
						path, candidates.front().c_str(), stbi_failure_reason());
					return nullptr;
				}
			}

			IDirect3DTexture9* tex = nullptr;
			HRESULT hr = g_device->CreateTexture(static_cast<UINT>(w), static_cast<UINT>(h),
				1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, nullptr);
			if (FAILED(hr) || !tex) {
				Log("icons", "CreateTexture failed for '%s' (0x%08lX)\n", path, static_cast<unsigned long>(hr));
				stbi_image_free(pixels);
				return nullptr;
			}

			D3DLOCKED_RECT rect;
			if (FAILED(tex->LockRect(0, &rect, nullptr, 0))) {
				Log("icons", "LockRect failed for '%s'\n", path);
				tex->Release();
				stbi_image_free(pixels);
				return nullptr;
			}

			for (int y = 0; y < h; ++y) {
				const stbi_uc* src = pixels + static_cast<size_t>(y) * w * 4;
				unsigned char* dst = static_cast<unsigned char*>(rect.pBits) + static_cast<size_t>(y) * rect.Pitch;
				for (int x = 0; x < w; ++x) {
					dst[x * 4 + 0] = src[x * 4 + 2]; // B
					dst[x * 4 + 1] = src[x * 4 + 1]; // G
					dst[x * 4 + 2] = src[x * 4 + 0]; // R
					dst[x * 4 + 3] = src[x * 4 + 3]; // A
				}
			}

			tex->UnlockRect(0);
			stbi_image_free(pixels);
			return tex;
		}

	}

	void IconTexture::SetDevice(void* d3d9Device) {
		IDirect3DDevice9* dev = reinterpret_cast<IDirect3DDevice9*>(d3d9Device);
		if (dev == g_device)
			return;
		Clear();
		g_device = dev;
	}

	ImTextureID IconTexture::Load(const char* path) {
		if (!g_device || !path || !path[0])
			return static_cast<ImTextureID>(0);

		auto it = g_cache.find(path);
		if (it != g_cache.end())
			return static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(it->second));

		IDirect3DTexture9* tex = CreateFromFile(path);
		g_cache[path] = tex;
		return static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(tex));
	}

	void IconTexture::Clear() {
		for (auto& kv : g_cache)
			if (kv.second)
				kv.second->Release();
		g_cache.clear();
		g_device = nullptr;
	}

}
